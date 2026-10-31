#include "../inc/ec_application.h"
#include "../../NOM_STACK/Application/WL/inc/wl_application.h"
#include "../../NOM_STACK/Services/PUMP_IO_COM/inc/pump_io_com.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"

DS1302_DateTime *ec_current_time = NULL;
int required_time_for_ec_dosing = 0;
int ec_calculated_time_difference = 0;

bool ec_first_shot_in_out_relays = false;
float ec_adjustment = 0.0f;
float ec_volumeToAdd = 0.0f;
float ec_first_shot_wl_data = 0.0f;

uint16_t ec_check_the_dosing_process_counter = 0;
uint8_t ec_place_to_buffer = 0;
uint8_t ec_for_loop_counter = 0;
float ec_volume_buffer_sum = 0.0f, ec_diff_buffer_sum = 0.0f;
float ec_volume_buffer_mean_value = 0.0f, ec_diff_buffer_mean_value = 0.0f;
float ec_calculated_volume_buffer[32] = {0};
float ec_calculated_ec_diff_buffer[32] = {0};

// rtc
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;

DS1302_DateTime ec_reference_time_value;
DS1302_DateTime *ec_reference_time = NULL;

ec_state_machine ec_app_current_state = EC_CALCULATE_EC_ADJ_VOLUME;

bool ec_motor_activation_first_shot = false;
bool ec_increase_get_current_time_assigned = false;
bool ec_achieved_targeted_time = false;
bool ec_is_positive_difference = false;

ec_diff_pos ec_prev_diff_position = EC_POS_IDLE;
ec_diff_pos ec_current_diff_position = EC_POS_IDLE;

void EC_RESET_STATE(void) {
  ec_increase_get_current_time_assigned = false;
  ec_achieved_targeted_time = false;
  ec_prev_diff_position = EC_POS_IDLE;
  ec_current_diff_position = EC_POS_IDLE;
  required_time_for_ec_dosing = 0;
  ec_calculated_time_difference = 0;
  ec_check_the_dosing_process_counter = 0;

  signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
  signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
  signal_ctrl_pipe_water_enable_from_ec = SENSOR_IC_SIGNAL_FALSE;
}

// Convert time to total seconds
int ec_timeToSeconds(DS1302_DateTime t) {
  return (t.hour * 3600) + (t.minute * 60) + t.second;
}

// Convert total seconds back to RTC_Time
DS1302_DateTime ec_secondsToTime(int totalSeconds) {
  DS1302_DateTime t = {0};
  if (totalSeconds < 0) {
    totalSeconds = 0;
  }
  t.hour = (uint8_t)((totalSeconds / 3600) % 24);
  t.minute = (uint8_t)((totalSeconds % 3600) / 60);
  t.second = (uint8_t)(totalSeconds % 60);
  return t;
}

// Function to calculate time difference with proper minute and second
// adjustments
DS1302_DateTime ec_calculateTimeDifference(DS1302_DateTime *start,
                                           DS1302_DateTime *end) {
  DS1302_DateTime diff = {0};
  if (start == NULL || end == NULL) {
    return diff;
  }

  int startSeconds = ec_timeToSeconds(*start);
  int endSeconds = ec_timeToSeconds(*end);

  // Handle midnight rollover
  if (endSeconds < startSeconds) {
    endSeconds += 24 * 3600;
  }

  int duration = endSeconds - startSeconds;
  diff = ec_secondsToTime(duration);

  return diff;
}

// Function to calculate the volume of EC solution to be added
float calculate_solution_volume(float targetEC, float currentEC,
                                float solutionEC, float tankVolume) {
  if (!isfinite(targetEC) || !isfinite(currentEC) || !isfinite(solutionEC) ||
      !isfinite(tankVolume) || tankVolume <= 0.0f) {
    ec_volumeToAdd = 0.0f;
    return 0.0f;
  }

  float denom = solutionEC - currentEC;
  if (targetEC > currentEC && denom <= 1.0f) {
    ec_volumeToAdd = 0.0f;
    return 0.0f;
  }
  if (fabsf(denom) < 1.0f) {
    ec_volumeToAdd = 0.0f;
    return 0.0f;
  }

  ec_volumeToAdd = ((targetEC - currentEC) * tankVolume) / denom;
  if (ec_volumeToAdd < 0.0f) {
    ec_volumeToAdd = -ec_volumeToAdd;
  }
  return ec_volumeToAdd;
}

// Function to calculate EC adjustment
float calculate_ec_adjustment(float targetEC, float currentEC,
                              float concentrationFactor, float tankVolume) {
  (void)tankVolume;
  if (!isfinite(targetEC) || !isfinite(currentEC)) {
    ec_adjustment = 0.0f;
    return 0.0f;
  }
  if (concentrationFactor <= 0.0f) {
    concentrationFactor = 1.0f;
  }
  ec_adjustment = (targetEC - currentEC);
  return ec_adjustment;
}

int ec_calculate_active_time_of_pump(float volume) {
  if (!isfinite(volume) || volume <= 0.0f) {
    return 0;
  }
  if (signal_data_get_dosing_intensity <= 0) {
    signal_data_get_dosing_intensity = 1;
  }
  float seconds =
      ((volume * 700.0f) / (float)signal_data_get_dosing_intensity) * 60.0f;
  if (seconds < 1.0f) {
    seconds = 1.0f; // Minimum 1-second micro-dose pulse when out of tolerance
  }
  if (seconds > 86399.0f) {
    seconds = 86399.0f;
  }
  return (int)seconds;
}

void calculate_added_vol_diff_ec(void) {
  if (signal_data_set_ec_difference_data != 0.0f) {
    if (ec_check_the_dosing_process_counter < 100) {
      ec_check_the_dosing_process_counter++;
    } else {
      ec_check_the_dosing_process_counter = 0;
      if (ec_place_to_buffer < 32) {
        ec_volume_buffer_sum = 0.0f;
        ec_diff_buffer_sum = 0.0f;
        ec_volume_buffer_mean_value = 0.0f;
        ec_diff_buffer_mean_value = 0.0f;
        ec_calculated_volume_buffer[ec_place_to_buffer] =
            calculate_solution_volume(
                signal_data_get_target_ec_data, signal_data_get_ec_data,
                (float)signal_data_get_ec_concentration_data,
                signal_data_get_ec_water_volume_data);
        ec_calculated_ec_diff_buffer[ec_place_to_buffer] =
            calculate_ec_adjustment(
                signal_data_get_target_ec_data, signal_data_get_ec_data,
                (float)signal_data_get_ec_concentration_data,
                signal_data_get_ec_water_volume_data);
        for (ec_for_loop_counter = 0; ec_for_loop_counter < 32;
             ec_for_loop_counter++) {
          ec_volume_buffer_sum +=
              ec_calculated_volume_buffer[ec_for_loop_counter];
          ec_diff_buffer_sum +=
              ec_calculated_ec_diff_buffer[ec_for_loop_counter];
        }
        ec_volume_buffer_mean_value = ec_volume_buffer_sum / 32.0f;
        ec_diff_buffer_mean_value = ec_diff_buffer_sum / 32.0f;
        ec_place_to_buffer++;
      } else {
        ec_place_to_buffer = 0;
      }
    }
  }
}

void exe_ec_pumps(void) {
  if (ec_increase_get_current_time_assigned == false) {
    if (signal_data_set_ec_difference_data > 0) {
      signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
      if (signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_POS) {
        ec_reference_time = &ec_reference_time_value;
        *ec_reference_time = *signal_data_get_rtc_data;
        required_time_for_ec_dosing =
            ec_calculate_active_time_of_pump(signal_data_set_ec_req_volume);
        ec_increase_get_current_time_assigned = true;
        ec_achieved_targeted_time = false;
        signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_TRUE;
        signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_TRUE;
        ec_prev_diff_position = EC_POS_POSITIVE;
      } else {
        signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
        if (signal_ctrl_pipe_water_enable_from_ph == SENSOR_IC_SIGNAL_TRUE) {
          signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
        }
        ec_prev_diff_position = EC_POS_POSITIVE;
      }
    }
    if (signal_data_set_ec_difference_data < 0) {
      signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
      if (signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_NEG) {
        ec_reference_time = &ec_reference_time_value;
        *ec_reference_time = *signal_data_get_rtc_data;
        required_time_for_ec_dosing =
            ec_calculate_active_time_of_pump(signal_data_set_ec_req_volume);
        ec_increase_get_current_time_assigned = true;
        ec_achieved_targeted_time = false;
        signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_TRUE;
        if (signal_data_get_water_level_data > WL_LOW_THRESHOLD_CM) {
          signal_ctrl_peripheral_turn_on_main_tank_water_output =
              SENSOR_IC_SIGNAL_FALSE;
          signal_ctrl_peripheral_turn_off_main_tank_water_output =
              SENSOR_IC_SIGNAL_TRUE;
        } else {
          signal_ctrl_peripheral_turn_on_main_tank_water_output =
              SENSOR_IC_SIGNAL_TRUE;
          signal_ctrl_peripheral_turn_off_main_tank_water_output =
              SENSOR_IC_SIGNAL_FALSE;
        }
        if (signal_data_get_water_level_data <= WL_HIGH_THRESHOLD_CM) {
          signal_ctrl_peripheral_turn_on_main_tank_water_input =
              SENSOR_IC_SIGNAL_FALSE;
          signal_ctrl_peripheral_turn_off_main_tank_water_input =
              SENSOR_IC_SIGNAL_TRUE;
        } else {
          signal_ctrl_peripheral_turn_on_main_tank_water_input =
              SENSOR_IC_SIGNAL_TRUE;
          signal_ctrl_peripheral_turn_off_main_tank_water_input =
              SENSOR_IC_SIGNAL_FALSE;
        }
        signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_TRUE;
        ec_prev_diff_position = EC_POS_NEGATIVE;
      } else {
        ec_prev_diff_position = EC_POS_NEGATIVE;
        signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
        if (signal_ctrl_pipe_water_enable_from_ph == SENSOR_IC_SIGNAL_TRUE) {
          signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
        }
      }
    }
  } else {
    if (ec_prev_diff_position == EC_POS_NEGATIVE) {
      ec_calculated_time_difference =
          ec_timeToSeconds(ec_calculateTimeDifference(
              ec_reference_time, signal_data_get_rtc_data));
      if (ec_calculated_time_difference < required_time_for_ec_dosing) {
        signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_TRUE;
        if (signal_data_get_water_level_data > WL_LOW_THRESHOLD_CM) {
          signal_ctrl_peripheral_turn_on_main_tank_water_output =
              SENSOR_IC_SIGNAL_FALSE;
          signal_ctrl_peripheral_turn_off_main_tank_water_output =
              SENSOR_IC_SIGNAL_TRUE;
        } else {
          signal_ctrl_peripheral_turn_on_main_tank_water_output =
              SENSOR_IC_SIGNAL_TRUE;
          signal_ctrl_peripheral_turn_off_main_tank_water_output =
              SENSOR_IC_SIGNAL_FALSE;
        }

        // The negative EC phase doesn't use the EC pump, it uses input/output
        // water flow to dilute. But we still apply the timer logic. If target
        // not reached yet, turn IN on (dilute)
        if (signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_NEG) {
          if (signal_data_get_water_level_data <= WL_HIGH_THRESHOLD_CM) {
            signal_ctrl_peripheral_turn_on_main_tank_water_input =
                SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_peripheral_turn_off_main_tank_water_input =
                SENSOR_IC_SIGNAL_TRUE;
          } else {
            signal_ctrl_peripheral_turn_on_main_tank_water_input =
                SENSOR_IC_SIGNAL_TRUE;
            signal_ctrl_peripheral_turn_off_main_tank_water_input =
                SENSOR_IC_SIGNAL_FALSE;
          }
        } else {
          // Target reached early, stop adding fresh water
          signal_ctrl_peripheral_turn_on_main_tank_water_input =
              SENSOR_IC_SIGNAL_FALSE;
          signal_ctrl_peripheral_turn_off_main_tank_water_input =
              SENSOR_IC_SIGNAL_TRUE;
        }
        signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_TRUE;
      } else {
        if (ec_calculated_time_difference <
            (required_time_for_ec_dosing + signal_data_get_mixing_time)) {
          // Unconditionally close input valve during mixing phase
          signal_ctrl_peripheral_turn_on_main_tank_water_input =
              SENSOR_IC_SIGNAL_FALSE;
          signal_ctrl_peripheral_turn_off_main_tank_water_input =
              SENSOR_IC_SIGNAL_TRUE;

          // Close output valve during mixing phase, unless overflowing
          if (signal_data_get_water_level_data <= WL_CRITICAL_THRESHOLD_CM) {
            signal_ctrl_peripheral_turn_on_main_tank_water_output =
                SENSOR_IC_SIGNAL_TRUE;
            signal_ctrl_peripheral_turn_off_main_tank_water_output =
                SENSOR_IC_SIGNAL_FALSE;
          } else {
            signal_ctrl_peripheral_turn_on_main_tank_water_output =
                SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_peripheral_turn_off_main_tank_water_output =
                SENSOR_IC_SIGNAL_TRUE;
          }
          signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_TRUE;
          signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
        } else {
          signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
          signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
          ec_achieved_targeted_time = true;
        }
      }
    } else if (ec_prev_diff_position == EC_POS_POSITIVE) {
      signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
      ec_calculated_time_difference =
          ec_timeToSeconds(ec_calculateTimeDifference(
              ec_reference_time, signal_data_get_rtc_data));
      if (ec_calculated_time_difference < required_time_for_ec_dosing) {
        // Only turn ON the dosing pump if we haven't hit the target yet
        if (signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_POS) {
          signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_TRUE;
        } else {
          signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
        }
        signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_TRUE;
      } else {
        if (ec_calculated_time_difference <
            (required_time_for_ec_dosing + signal_data_get_mixing_time)) {
          signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;

          // Unconditionally close input valve during mixing phase
          signal_ctrl_peripheral_turn_on_main_tank_water_input =
              SENSOR_IC_SIGNAL_FALSE;
          signal_ctrl_peripheral_turn_off_main_tank_water_input =
              SENSOR_IC_SIGNAL_TRUE;

          // Close output valve during mixing phase, unless overflowing
          if (signal_data_get_water_level_data <= WL_CRITICAL_THRESHOLD_CM) {
            signal_ctrl_peripheral_turn_on_main_tank_water_output =
                SENSOR_IC_SIGNAL_TRUE;
            signal_ctrl_peripheral_turn_off_main_tank_water_output =
                SENSOR_IC_SIGNAL_FALSE;
          } else {
            signal_ctrl_peripheral_turn_on_main_tank_water_output =
                SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_peripheral_turn_off_main_tank_water_output =
                SENSOR_IC_SIGNAL_TRUE;
          }

          signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_TRUE;
        } else {
          signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
          if (signal_ctrl_pipe_water_enable_from_ph == SENSOR_IC_SIGNAL_TRUE) {
            signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
          }
          ec_achieved_targeted_time = true;
        }
      }
    }
  }

  // Safety cutoff when tank is empty (>= 105 cm) OR critically overflowing (<=
  // 23 cm)
  if (signal_data_get_water_level_data >= WL_LOW_THRESHOLD_CM ||
      signal_data_get_water_level_data <= WL_CRITICAL_THRESHOLD_CM) {
    signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_circulation_pumps_turn_on = SENSOR_IC_SIGNAL_FALSE;
  }

  if (ec_achieved_targeted_time == true) {
    ec_increase_get_current_time_assigned = false;
  }
}

void EC_EXE_APPLICATION(void) {
  if (!PUMP_IO_COM_is_link_healthy() ||
      signal_ctrl_water_level_start_ph_ec_unit != SENSOR_IC_SIGNAL_TRUE ||
      signal_ctrl_water_level_stop_ph_ec_unit == SENSOR_IC_SIGNAL_TRUE) {
    EC_RESET_STATE();
    return;
  }

  signal_data_get_ec_water_volume_data = (float)signal_data_get_tank_capacity;

  signal_data_set_ec_difference_data = calculate_ec_adjustment(
      signal_data_get_target_ec_data, signal_data_get_ec_data,
      (float)signal_data_get_ec_concentration_data,
      signal_data_get_ec_water_volume_data);
  signal_data_set_ec_req_volume = calculate_solution_volume(
      signal_data_get_target_ec_data, signal_data_get_ec_data,
      (float)signal_data_get_ec_concentration_data,
      signal_data_get_ec_water_volume_data);

  if (!ec_increase_get_current_time_assigned &&
      signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_NEG &&
      signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_POS) {
    signal_ctrl_pipe_water_enable_from_ec = SENSOR_IC_SIGNAL_TRUE;
    signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;

    if (signal_data_get_water_level_data <= WL_HIGH_THRESHOLD_CM) {
      signal_ctrl_peripheral_turn_on_main_tank_water_input =
          SENSOR_IC_SIGNAL_FALSE;
      signal_ctrl_peripheral_turn_off_main_tank_water_input =
          SENSOR_IC_SIGNAL_TRUE;
    } else {
      signal_ctrl_peripheral_turn_on_main_tank_water_input =
          SENSOR_IC_SIGNAL_TRUE;
      signal_ctrl_peripheral_turn_off_main_tank_water_input =
          SENSOR_IC_SIGNAL_FALSE;
    }

    if (signal_ctrl_pipe_water_enable_from_ph == SENSOR_IC_SIGNAL_TRUE) {
      signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
    }
  } else {
    signal_ctrl_pipe_water_enable_from_ec = SENSOR_IC_SIGNAL_FALSE;
    exe_ec_pumps();
    calculate_added_vol_diff_ec();
  }
}
