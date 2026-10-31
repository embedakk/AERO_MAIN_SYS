#include "../inc/ec_application.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"
#include "../../NOM_STACK/Application/WL/inc/wl_application.h"


DS1302_DateTime *ec_current_time;
int required_time_for_ec_dosing;
int ec_calculated_time_difference;

bool ec_first_shot_in_out_relays;
float ec_adjustment;
float ec_volumeToAdd;
float ec_first_shot_wl_data;

uint16_t ec_check_the_dosing_process_counter;
uint8_t ec_place_to_buffer;
uint8_t ec_for_loop_counter;
float ec_volume_buffer_sum, ec_diff_buffer_sum;
float ec_volume_buffer_mean_value, ec_diff_buffer_mean_value;
float ec_calculated_volume_buffer[32];
float ec_calculated_ec_diff_buffer[32];

//rtc
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;

DS1302_DateTime ec_reference_time_value;
DS1302_DateTime *ec_reference_time = NULL;

ec_state_machine ec_app_current_state = EC_CALCULATE_EC_ADJ_VOLUME;

// Convert time to total seconds
int ec_timeToSeconds(DS1302_DateTime t) 
{
    return (t.hour * 3600) + (t.minute * 60) + t.second;
}

// Convert total seconds back to RTC_Time
DS1302_DateTime ec_secondsToTime(int totalSeconds) 
{
    DS1302_DateTime t;
    t.hour = totalSeconds / 3600;
    t.minute = (totalSeconds % 3600) / 60;
    t.second = totalSeconds % 60;
    return t;
}

// Function to calculate time difference with proper minute and second adjustments
DS1302_DateTime ec_calculateTimeDifference(DS1302_DateTime *start, DS1302_DateTime *end) 
{
    DS1302_DateTime diff;
    
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
float calculate_solution_volume(float targetEC, float currentEC, float solutionEC, float tankVolume)
{
    ec_volumeToAdd = ((targetEC - currentEC) * tankVolume) / (solutionEC - currentEC);
    if(ec_volumeToAdd < 0)
    {
        ec_volumeToAdd = ec_volumeToAdd * -1;
    }
    return ec_volumeToAdd;
}

// Function to calculate EC adjustment
float calculate_ec_adjustment(float targetEC, float currentEC, float concentrationFactor, float tankVolume)
{
    if (concentrationFactor <= 0) 
    {
        concentrationFactor = 1.0f;
    }
    ec_adjustment = (targetEC - currentEC);
    return ec_adjustment;
}

int ec_calculate_active_time_of_pump(float volume)
{
    if(signal_data_get_dosing_intensity <= 0)
    {
        signal_data_get_dosing_intensity = 1;
    }
    return (int)(((volume * 700)/ (signal_data_get_dosing_intensity)) * 60);
}

bool ec_motor_activation_first_shot = false;
bool ec_increase_get_current_time_assigned = false;
bool ec_achieved_targeted_time = false;
bool ec_is_positive_difference = false; 

ec_diff_pos ec_prev_diff_position = EC_POS_IDLE;
ec_diff_pos ec_current_diff_position = EC_POS_IDLE;

void calculate_added_vol_diff_ec(void)
{
    if(signal_data_set_ec_difference_data > 0)
    {
        if(ec_check_the_dosing_process_counter < 100)
        {
            ec_check_the_dosing_process_counter++;
        }else
        {
            ec_check_the_dosing_process_counter = 0;
            if(ec_place_to_buffer < 32)
            {
                ec_volume_buffer_mean_value  = 0;
                ec_diff_buffer_mean_value = 0;
                ec_calculated_volume_buffer[ec_place_to_buffer] = calculate_solution_volume(signal_data_get_target_ec_data, signal_data_get_ec_data, signal_data_get_ec_concentration_data, signal_data_get_ec_water_volume_data); 
                ec_calculated_ec_diff_buffer[ec_place_to_buffer] = calculate_ec_adjustment(signal_data_get_target_ec_data, signal_data_get_ec_data, signal_data_get_ec_concentration_data, signal_data_get_ec_water_volume_data); 
                for(ec_for_loop_counter = 0; ec_for_loop_counter < 32; ec_for_loop_counter++)
                {
                    ec_volume_buffer_sum  += ec_calculated_volume_buffer[ec_for_loop_counter];
                    ec_diff_buffer_sum += ec_calculated_ec_diff_buffer[ec_for_loop_counter];
                }
                ec_volume_buffer_mean_value  = ec_volume_buffer_sum / 32;
                ec_diff_buffer_mean_value = ec_diff_buffer_sum / 32;
                ec_place_to_buffer++;
            }else
            {
                ec_place_to_buffer = 0;
            }
        }
    }
    if(signal_data_set_ec_difference_data < 0)
    {
        if(ec_check_the_dosing_process_counter < 100)
        {
            ec_check_the_dosing_process_counter++;
        }else
        {
            ec_check_the_dosing_process_counter = 0;
            if(ec_place_to_buffer < 32)
            {
                ec_volume_buffer_mean_value  = 0;
                ec_diff_buffer_mean_value = 0;
                ec_calculated_volume_buffer[ec_place_to_buffer] = calculate_solution_volume(signal_data_get_target_ec_data, signal_data_get_ec_data, signal_data_get_ec_concentration_data, signal_data_get_ec_water_volume_data); 
                ec_calculated_ec_diff_buffer[ec_place_to_buffer] = calculate_ec_adjustment(signal_data_get_target_ec_data, signal_data_get_ec_data, signal_data_get_ec_concentration_data, signal_data_get_ec_water_volume_data); 
                for(ec_for_loop_counter = 0; ec_for_loop_counter < 32; ec_for_loop_counter++)
                {
                    ec_volume_buffer_sum  += ec_calculated_volume_buffer[ec_for_loop_counter];
                    ec_diff_buffer_sum += ec_calculated_ec_diff_buffer[ec_for_loop_counter];
                }
                ec_volume_buffer_mean_value  = ec_volume_buffer_sum / 32;
                ec_diff_buffer_mean_value = ec_diff_buffer_sum / 32;
                ec_place_to_buffer++;
            }else
            {
                ec_place_to_buffer = 0;
            }
        }
    }
}

void exe_ec_pumps(void)
{
    if(ec_increase_get_current_time_assigned == false)
    {
        // printf("exe_ec_pumps => ec_increase_get_current_time_assigned == false\n");

        if(signal_data_set_ec_difference_data > 0)
        {
            // printf("exe_ec_pumps => signal_data_set_ec_difference_data > 0\n");
            signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
            if(signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_POS)
            {
                // printf("exe_ec_pumps => signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_POS\n");
                ec_reference_time = &ec_reference_time_value;
                *ec_reference_time = *signal_data_get_rtc_data;
                required_time_for_ec_dosing = ec_calculate_active_time_of_pump(signal_data_set_ec_req_volume);
                ec_increase_get_current_time_assigned = true;
                ec_achieved_targeted_time = false;
                signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_TRUE;
                signal_ctrl_mixer_motors_turn_on       = SENSOR_IC_SIGNAL_TRUE;
                ec_prev_diff_position = EC_POS_POSITIVE;
            }else
            {
                // printf("exe_ec_pumps => signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_POS elseee\n");
                signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
                if (signal_ctrl_pipe_water_enable_from_ph == SENSOR_IC_SIGNAL_TRUE) {
                    signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_FALSE;
                }
                ec_prev_diff_position = EC_POS_POSITIVE;
            }
        }
        if(signal_data_set_ec_difference_data < 0)
        {
            // printf("exe_ec_pumps => signal_data_set_ec_difference_data < 0\n");
            signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
            if(signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_NEG)
            {
                // printf("exe_ec_pumps => signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_NEG\n");
                ec_reference_time = &ec_reference_time_value;
                *ec_reference_time = *signal_data_get_rtc_data;  
                required_time_for_ec_dosing = ec_calculate_active_time_of_pump(signal_data_set_ec_req_volume);
                ec_increase_get_current_time_assigned = true;
                ec_achieved_targeted_time = false;
                signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_TRUE;
                if(signal_data_get_water_level_data > WL_LOW_THRESHOLD_CM)
                {
                    // printf("exe_ec_pumps => signal_data_get_water_level_data > WL_LOW_THRESHOLD_CM out off\n");
                    signal_ctrl_peripheral_turn_on_main_tank_water_output                   = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_output                  = SENSOR_IC_SIGNAL_TRUE;
                }else
                {
                    // printf("exe_ec_pumps => signal_data_get_water_level_data > WL_LOW_THRESHOLD_CM  elsee out on\n");
                    signal_ctrl_peripheral_turn_on_main_tank_water_output                   = SENSOR_IC_SIGNAL_TRUE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_output                  = SENSOR_IC_SIGNAL_FALSE;
                }
                if(signal_data_get_water_level_data <= WL_HIGH_THRESHOLD_CM)
                {
                    // printf("exe_ec_pumps => signal_data_get_water_level_data <= WL_HIGH_THRESHOLD_CM in off\n");
                    signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_TRUE;
                    
                }else
                {
                    // printf("exe_ec_pumps => signal_data_get_water_level_data > WL_HIGH_THRESHOLD_CM elsee in on\n");
                    signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_TRUE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_FALSE;
                }
                signal_ctrl_mixer_motors_turn_on                                        = SENSOR_IC_SIGNAL_TRUE;
                ec_prev_diff_position = EC_POS_NEGATIVE;
            }else
            {
                // printf("exe_ec_pumps => signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_NEG elsee\n");
                ec_prev_diff_position = EC_POS_NEGATIVE;
                signal_ctrl_ec_disable_wl_unit                                          = SENSOR_IC_SIGNAL_FALSE;
                if (signal_ctrl_pipe_water_enable_from_ph == SENSOR_IC_SIGNAL_TRUE) {
                    signal_ctrl_mixer_motors_turn_on                                        = SENSOR_IC_SIGNAL_FALSE;
                }
            }
        }
    }else
    {
        if(ec_prev_diff_position == EC_POS_NEGATIVE)
        {
            ec_calculated_time_difference = ec_timeToSeconds(ec_calculateTimeDifference(ec_reference_time, signal_data_get_rtc_data));
            if(ec_calculated_time_difference < required_time_for_ec_dosing)
            {
                signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_TRUE;
                signal_ctrl_peripheral_turn_on_main_tank_water_output               = SENSOR_IC_SIGNAL_TRUE;
                signal_ctrl_peripheral_turn_off_main_tank_water_output              = SENSOR_IC_SIGNAL_FALSE;
                
                // The negative EC phase doesn't use the EC pump, it uses input/output water flow to dilute.
                // But we still apply the timer logic. If target not reached yet, turn IN on (dilute)
                if(signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_NEG) {
                    if(signal_data_get_water_level_data <= WL_HIGH_THRESHOLD_CM)
                    {
                        signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_FALSE;
                        signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_TRUE;
                    }else
                    {
                        signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_TRUE;
                        signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_FALSE;
                    }
                } else {
                    // Target reached early, stop adding fresh water
                    signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_TRUE;
                }
                signal_ctrl_mixer_motors_turn_on                                    = SENSOR_IC_SIGNAL_TRUE;
            }else
            {
                if(ec_calculated_time_difference < (required_time_for_ec_dosing + signal_data_get_mixing_time))
                {
                    // Unconditionally close input valve during mixing phase
                    signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_TRUE;
                    
                    // Close output valve during mixing phase, unless overflowing
                    if(signal_data_get_water_level_data <= WL_CRITICAL_THRESHOLD_CM)
                    {
                        signal_ctrl_peripheral_turn_on_main_tank_water_output               = SENSOR_IC_SIGNAL_TRUE;
                        signal_ctrl_peripheral_turn_off_main_tank_water_output              = SENSOR_IC_SIGNAL_FALSE;
                    }else
                    {
                        signal_ctrl_peripheral_turn_on_main_tank_water_output               = SENSOR_IC_SIGNAL_FALSE;
                        signal_ctrl_peripheral_turn_off_main_tank_water_output              = SENSOR_IC_SIGNAL_TRUE;
                    }
                    signal_ctrl_mixer_motors_turn_on                                    = SENSOR_IC_SIGNAL_TRUE;
                    signal_ctrl_ec_disable_wl_unit                                      = SENSOR_IC_SIGNAL_FALSE;
                }else
                {
                    signal_ctrl_mixer_motors_turn_on                                    = SENSOR_IC_SIGNAL_FALSE;
                    ec_achieved_targeted_time = true;
                }
            }
        }
        else if(ec_prev_diff_position == EC_POS_POSITIVE)
        {
            signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
            ec_calculated_time_difference = ec_timeToSeconds(ec_calculateTimeDifference(ec_reference_time, signal_data_get_rtc_data));
            if(ec_calculated_time_difference < required_time_for_ec_dosing)
            {
                // Only turn ON the dosing pump if we haven't hit the target yet
                if(signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_POS) {
                    signal_ctrl_peripheral_turn_on_ec_pump  = SENSOR_IC_SIGNAL_TRUE;
                } else {
                    signal_ctrl_peripheral_turn_on_ec_pump  = SENSOR_IC_SIGNAL_FALSE;
                }
                signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_TRUE;
            }else
            {
                if(ec_calculated_time_difference < (required_time_for_ec_dosing + signal_data_get_mixing_time))
                {
                    signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
                    
                    // Unconditionally close input valve during mixing phase
                    signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_TRUE;
                    
                    // Close output valve during mixing phase, unless overflowing
                    if(signal_data_get_water_level_data <= WL_CRITICAL_THRESHOLD_CM)
                    {
                        signal_ctrl_peripheral_turn_on_main_tank_water_output               = SENSOR_IC_SIGNAL_TRUE;
                        signal_ctrl_peripheral_turn_off_main_tank_water_output              = SENSOR_IC_SIGNAL_FALSE;
                    }else
                    {
                        signal_ctrl_peripheral_turn_on_main_tank_water_output               = SENSOR_IC_SIGNAL_FALSE;
                        signal_ctrl_peripheral_turn_off_main_tank_water_output              = SENSOR_IC_SIGNAL_TRUE;
                    }
                    
                    signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_TRUE;
                }else
                {
                    if (signal_ctrl_pipe_water_enable_from_ph == SENSOR_IC_SIGNAL_TRUE) {
                        signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_FALSE;
                    }
                    ec_achieved_targeted_time = true;
                }
            }
        }
    }

    if(signal_data_get_water_level_data > WL_LOW_THRESHOLD_CM && signal_data_get_water_level_data < WL_CRITICAL_THRESHOLD_CM)
    {
        // printf("signal_data_get_water_level_data > WL_LOW_THRESHOLD_CM && signal_data_get_water_level_data < WL_CRITICAL_THRESHOLD_CM\n");
        signal_ctrl_peripheral_turn_on_ec_pump  = SENSOR_IC_SIGNAL_FALSE;
        signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_FALSE;
        signal_ctrl_circulation_pumps_turn_on   = SENSOR_IC_SIGNAL_FALSE;
    }
    // ec_app_current_state = EC_CALCULATE_ADDED_EC_VOL_DIFF_ON_TANK;
    if(ec_achieved_targeted_time == true)
    {
        ec_increase_get_current_time_assigned = false;
    }
    // printf("**************************************************************************************************************************************\r\n");
}


void EC_EXE_APPLICATION(void)
{
    if(signal_ctrl_water_level_start_ph_ec_unit == SENSOR_IC_SIGNAL_TRUE && signal_ctrl_water_level_stop_ph_ec_unit == SENSOR_IC_SIGNAL_FALSE)
    {
        signal_data_get_ec_water_volume_data = (float)signal_data_get_tank_capacity;

        signal_data_set_ec_difference_data  = calculate_ec_adjustment(signal_data_get_target_ec_data, signal_data_get_ec_data, signal_data_get_ec_concentration_data, signal_data_get_ec_water_volume_data);
        signal_data_set_ec_req_volume       = calculate_solution_volume(signal_data_get_target_ec_data, signal_data_get_ec_data, signal_data_get_ec_concentration_data, signal_data_get_ec_water_volume_data);
        if(!ec_increase_get_current_time_assigned && signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_NEG && signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_POS)
        {
            signal_ctrl_pipe_water_enable_from_ec                   = SENSOR_IC_SIGNAL_TRUE;
            signal_ctrl_peripheral_turn_on_ec_pump                  = SENSOR_IC_SIGNAL_FALSE;
            
            // In case we want to apply the same full check here:
            if(signal_data_get_water_level_data <= WL_HIGH_THRESHOLD_CM)
            {
                // Note: The original code didn't force the input valve OFF here directly 
                // in EC like it did in PH, but let's be explicit and consistent if it was implicit.
                // Wait, original ec_application.c didn't turn it off here at line 374, so we don't need to add it unless requested.
                // But PH turned it off. So let's align it.
                signal_ctrl_peripheral_turn_on_main_tank_water_input    = SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_peripheral_turn_off_main_tank_water_input   = SENSOR_IC_SIGNAL_TRUE;
            }else
            {
                signal_ctrl_peripheral_turn_on_main_tank_water_input    = SENSOR_IC_SIGNAL_TRUE;
                signal_ctrl_peripheral_turn_off_main_tank_water_input   = SENSOR_IC_SIGNAL_FALSE;
            }

            if(signal_ctrl_pipe_water_enable_from_ph == SENSOR_IC_SIGNAL_TRUE)
            {
                signal_ctrl_mixer_motors_turn_on                        = SENSOR_IC_SIGNAL_FALSE;
            }
        }else
        {
            signal_ctrl_pipe_water_enable_from_ec = SENSOR_IC_SIGNAL_FALSE;
            exe_ec_pumps();
            calculate_added_vol_diff_ec();
        }
    }
}