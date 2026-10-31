#include "../inc/ph_application.h"
#include "../../NOM_STACK/Application/WL/inc/wl_application.h"
#include "../../NOM_STACK/Services/PUMP_IO_COM/inc/pump_io_com.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"

DS1302_DateTime *ph_current_time = NULL;
int required_time_for_ph_dosing = 0;
int ph_calculated_time_difference = 0;

bool ph_first_shot_in_out_relays = false;
float ph_adjustment = 0.0f;
float ph_volumeToAdd = 0.0f;
float ph_first_shot_wl_data = 0.0f;

uint16_t ph_check_the_dosing_process_counter = 0;
uint8_t ph_place_to_buffer = 0;
uint8_t ph_for_loop_counter = 0;
float ph_volume_buffer_sum = 0.0f, ph_ph_diff_buffer_sum = 0.0f;
float ph_volume_buffer_mean_value = 0.0f, ph_ph_diff_buffer_mean_value = 0.0f;
float ph_calculated_volume_buffer[32] = {0};
float ph_calculated_ph_diff_buffer[32] = {0};

// rtc
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;

DS1302_DateTime ph_reference_time_value;
DS1302_DateTime *ph_reference_time = NULL;

ph_state_machine ph_app_current_state = PH_CALCULATE_PH_ADJ_VOLUME;

bool ph_motor_activation_first_shot = false;
bool ph_increase_get_current_time_assigned = false;
bool ph_achieved_targeted_time = false;
bool ph_is_positive_difference = false;

ph_diff_pos ph_prev_diff_position = PH_POS_IDLE;
ph_diff_pos ph_current_diff_position = PH_POS_IDLE;

void PH_RESET_STATE(void) {
  ph_increase_get_current_time_assigned = false;
  ph_achieved_targeted_time = false;
  ph_prev_diff_position = PH_POS_IDLE;
  ph_current_diff_position = PH_POS_IDLE;
  required_time_for_ph_dosing = 0;
  ph_calculated_time_difference = 0;
  ph_check_the_dosing_process_counter = 0;

  signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_FALSE;
  signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
  signal_ctrl_pipe_water_enable_from_ph = SENSOR_IC_SIGNAL_FALSE;
}

// Convert time to total seconds
int ph_timeToSeconds(DS1302_DateTime t) {
  return (t.hour * 3600) + (t.minute * 60) + t.second;
}

// Convert total seconds back to RTC_Time
DS1302_DateTime ph_secondsToTime(int totalSeconds) {
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
DS1302_DateTime ph_calculateTimeDifference(DS1302_DateTime *start,
                                           DS1302_DateTime *end) {
  DS1302_DateTime diff = {0};
  if (start == NULL || end == NULL) {
    return diff;
  }
  int startSeconds = ph_timeToSeconds(*start);
  int endSeconds = ph_timeToSeconds(*end);

  if (endSeconds < startSeconds) {
    endSeconds += 24 * 3600;
  }

  int duration = endSeconds - startSeconds;
  diff = ph_secondsToTime(duration);

  return diff;
}

// Function to calculate PH adjustment
float calculate_ph_adjustment(float targetPH, float currentPH,
                              float concentrationFactor, float tankVolume) {
  (void)concentrationFactor;
  (void)tankVolume;
  if (!isfinite(targetPH) || !isfinite(currentPH)) {
    ph_adjustment = 0.0f;
    return 0.0f;
  }
  ph_adjustment = (targetPH - currentPH);
  return ph_adjustment;
}

// Function to calculate the required volume of acid/base
double calculate_adjustment_volume(double tank_volume, double current_pH,
                                   double target_pH, double solution_conc) {
  if (!isfinite(tank_volume) || !isfinite(current_pH) || !isfinite(target_pH) ||
      !isfinite(solution_conc) || tank_volume <= 0.0 || solution_conc <= 0.0) {
    return 0.0;
  }
  if (current_pH < 0.0)
    current_pH = 0.0;
  if (current_pH > 14.0)
    current_pH = 14.0;
  if (target_pH < 0.0)
    target_pH = 0.0;
  if (target_pH > 14.0)
    target_pH = 14.0;

  double H_current = pow(10.0, -current_pH);
  double H_target = pow(10.0, -target_pH);
  double delta_H = fabs(H_target - H_current);
  double required_moles = delta_H * tank_volume;
  double acid_concentration_M = solution_conc / 1000.0;
  if (acid_concentration_M <= 0.0) {
    return 0.0;
  }
  double solution_volume = required_moles / acid_concentration_M;
  return solution_volume; // in liters
}

int ph_calculate_active_time_of_pump(float volume) {
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

void CALCULATE_ADDED_PH_VOL_DIFF_ON_TANK(void) {
  if (signal_data_set_ph_difference_data != 0.0f) {
    if (ph_check_the_dosing_process_counter < 100) {
      ph_check_the_dosing_process_counter++;
    } else {
      ph_check_the_dosing_process_counter = 0;
      if (ph_place_to_buffer < 32) {
        ph_volume_buffer_sum = 0.0f;
        ph_ph_diff_buffer_sum = 0.0f;
        ph_volume_buffer_mean_value = 0.0f;
        ph_ph_diff_buffer_mean_value = 0.0f;
        ph_calculated_volume_buffer[ph_place_to_buffer] =
            (float)calculate_adjustment_volume(
                signal_data_get_ph_water_volume_data, signal_data_get_ph_data,
                signal_data_get_target_ph_data,
                (double)signal_data_get_ph_concentration_data);
        ph_calculated_ph_diff_buffer[ph_place_to_buffer] =
            calculate_ph_adjustment(
                signal_data_get_target_ph_data, signal_data_get_ph_data,
                (float)signal_data_get_ph_concentration_data,
                signal_data_get_ph_water_volume_data);
        for (ph_for_loop_counter = 0; ph_for_loop_counter < 32;
             ph_for_loop_counter++) {
          ph_volume_buffer_sum +=
              ph_calculated_volume_buffer[ph_for_loop_counter];
          ph_ph_diff_buffer_sum +=
              ph_calculated_ph_diff_buffer[ph_for_loop_counter];
        }
        ph_volume_buffer_mean_value = ph_volume_buffer_sum / 32.0f;
        ph_ph_diff_buffer_mean_value = ph_ph_diff_buffer_sum / 32.0f;
        ph_place_to_buffer++;
      } else {
        ph_place_to_buffer = 0;
      }
    }
  }
}

void EXE_PUMPS(void) {
  if (ph_increase_get_current_time_assigned == false) {
    signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_FALSE;
    if (signal_data_set_ph_difference_data > 0) {
      if (signal_data_set_ph_difference_data > PH_DIFF_TOLERANCE_POS) {
        ph_reference_time = &ph_reference_time_value;
        *ph_reference_time = *signal_data_get_rtc_data;
        required_time_for_ph_dosing =
            ph_calculate_active_time_of_pump(signal_data_set_ph_req_volume);
        ph_increase_get_current_time_assigned = true;
        ph_achieved_targeted_time = false;
        signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_TRUE;

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
        ph_prev_diff_position = PH_POS_POSITIVE;
      } else {
        ph_prev_diff_position = PH_POS_POSITIVE;
        signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
        if (signal_ctrl_pipe_water_enable_from_ec == SENSOR_IC_SIGNAL_TRUE) {
          signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
        }
      }
    }
    if (signal_data_set_ph_difference_data < 0) {
      signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
      if (signal_data_set_ph_difference_data < PH_DIFF_TOLERANCE_NEG) {
        ph_reference_time = &ph_reference_time_value;
        *ph_reference_time = *signal_data_get_rtc_data;
        required_time_for_ph_dosing =
            ph_calculate_active_time_of_pump(signal_data_set_ph_req_volume);
        ph_increase_get_current_time_assigned = true;
        ph_achieved_targeted_time = false;
        signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_TRUE;
        signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_TRUE;
        ph_prev_diff_position = PH_POS_NEGATIVE;
      } else {
        signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_FALSE;
        signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
        ph_prev_diff_position = PH_POS_NEGATIVE;
      }
    }
  } else {
    if (ph_prev_diff_position == PH_POS_NEGATIVE) {
      signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;

      ph_calculated_time_difference =
          ph_timeToSeconds(ph_calculateTimeDifference(
              ph_reference_time, signal_data_get_rtc_data));
      if (ph_calculated_time_difference < required_time_for_ph_dosing) {
        // Only turn ON the dosing pump if we haven't hit the target yet
        if (signal_data_set_ph_difference_data < PH_DIFF_TOLERANCE_NEG) {
          signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_TRUE;
        } else {
          signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_FALSE;
        }
        signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_TRUE;
      } else {
        if (ph_calculated_time_difference <
            (required_time_for_ph_dosing + signal_data_get_mixing_time)) {
          signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_FALSE;

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
          signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_FALSE;
          if (signal_ctrl_pipe_water_enable_from_ec == SENSOR_IC_SIGNAL_TRUE) {
            signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
          }
          ph_achieved_targeted_time = true;
        }
      }
    } else if (ph_prev_diff_position == PH_POS_POSITIVE) {
      signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_TRUE;
      ph_calculated_time_difference =
          ph_timeToSeconds(ph_calculateTimeDifference(
              ph_reference_time, signal_data_get_rtc_data));

      if (ph_calculated_time_difference < required_time_for_ph_dosing) {
        // The positive PH phase doesn't use the PH pump, it uses input/output
        // water flow to dilute.
        if (signal_data_set_ph_difference_data > PH_DIFF_TOLERANCE_POS) {
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
        } else {
          // Target reached early, stop adding fresh water
          signal_ctrl_peripheral_turn_on_main_tank_water_input =
              SENSOR_IC_SIGNAL_FALSE;
          signal_ctrl_peripheral_turn_off_main_tank_water_input =
              SENSOR_IC_SIGNAL_TRUE;
        }
        signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_TRUE;
      } else {
        if (ph_calculated_time_difference <
            (required_time_for_ph_dosing + signal_data_get_mixing_time)) {
          signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_FALSE;
          // Unconditionally close input valve during mixing phase
          signal_ctrl_peripheral_turn_on_main_tank_water_input =
              SENSOR_IC_SIGNAL_FALSE;
          signal_ctrl_peripheral_turn_off_main_tank_water_input =
              SENSOR_IC_SIGNAL_TRUE;
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
          signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
        } else {
          if (signal_ctrl_pipe_water_enable_from_ec == SENSOR_IC_SIGNAL_TRUE) {
            signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
          }
          ph_achieved_targeted_time = true;
          signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
        }
      }
    }
  }

  // Safety cutoff when tank is empty (>= 105 cm) OR critically overflowing (<=
  // 23 cm)
  if (signal_data_get_water_level_data >= WL_LOW_THRESHOLD_CM ||
      signal_data_get_water_level_data <= WL_CRITICAL_THRESHOLD_CM) {
    signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_circulation_pumps_turn_on = SENSOR_IC_SIGNAL_FALSE;
  }

  if (ph_achieved_targeted_time == true) {
    ph_increase_get_current_time_assigned = false;
  }
}

void PH_EXE_APPLICATION(void) {
  if (!PUMP_IO_COM_is_link_healthy() ||
      signal_ctrl_water_level_start_ph_ec_unit != SENSOR_IC_SIGNAL_TRUE ||
      signal_ctrl_water_level_stop_ph_ec_unit == SENSOR_IC_SIGNAL_TRUE) {
    PH_RESET_STATE();
    return;
  }

  signal_data_get_ph_water_volume_data = (float)signal_data_get_tank_capacity;

  signal_data_set_ph_difference_data = calculate_ph_adjustment(
      signal_data_get_target_ph_data, signal_data_get_ph_data,
      (float)signal_data_get_ph_concentration_data,
      signal_data_get_ph_water_volume_data);
  signal_data_set_ph_req_volume = (float)calculate_adjustment_volume(
      signal_data_get_ph_water_volume_data, signal_data_get_ph_data,
      signal_data_get_target_ph_data,
      (double)signal_data_get_ph_concentration_data);

  if (!ph_increase_get_current_time_assigned &&
      signal_data_set_ph_difference_data > PH_DIFF_TOLERANCE_NEG &&
      signal_data_set_ph_difference_data < PH_DIFF_TOLERANCE_POS) {
    signal_ctrl_pipe_water_enable_from_ph = SENSOR_IC_SIGNAL_TRUE;
    signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;

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
    if (signal_ctrl_pipe_water_enable_from_ec == SENSOR_IC_SIGNAL_TRUE) {
      signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
    }
  } else {
    signal_ctrl_pipe_water_enable_from_ph = SENSOR_IC_SIGNAL_FALSE;
    EXE_PUMPS();
    CALCULATE_ADDED_PH_VOL_DIFF_ON_TANK();
  }
}
