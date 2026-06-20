#include "../inc/ec_application.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"
//rtc
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;

DS1302_DateTime reference_time_value;
DS1302_DateTime *reference_time = NULL;

// ec_first_shot_in_out_relays = false;
ec_state_machine ec_app_current_state = EC_CALCULATE_EC_ADJ_VOLUME;

// Convert time to total seconds
int timeToSeconds(DS1302_DateTime t) 
{
    return (t.hour * 3600) + (t.minute * 60) + t.second;
}

// Convert total seconds back to RTC_Time
DS1302_DateTime secondsToTime(int totalSeconds) 
{
    DS1302_DateTime t;
    t.hour = totalSeconds / 3600;
    t.minute = (totalSeconds % 3600) / 60;
    t.second = totalSeconds % 60;
    return t;
}


// Function to calculate time difference with proper minute and second adjustments
DS1302_DateTime calculateTimeDifference(DS1302_DateTime *start, DS1302_DateTime *end) 
{
    DS1302_DateTime diff;
    
    int startSeconds = timeToSeconds(*start);
    int endSeconds = timeToSeconds(*end);

    // Handle midnight rollover
    if (endSeconds < startSeconds) {
        endSeconds += 24 * 3600;
    }

    int duration = endSeconds - startSeconds;
    diff = secondsToTime(duration);

    return diff;
}



// Function to calculate the volume of EC solution to be added
float calculate_solution_volume(float targetEC, float currentEC, float solutionEC, float tankVolume)
{
    // if (solutionEC <= currentEC) 
    // {
    //     //add diagnosis here
    //     // printf("Error: The EC of the solution must be higher than the current EC.\n");
    //     return 0;
    // }
    
    volumeToAdd = ((targetEC - currentEC) * tankVolume) / (solutionEC - currentEC);
    if(volumeToAdd < 0)
    {
        volumeToAdd = volumeToAdd * -1;
    }
    return volumeToAdd;
}

// Function to calculate EC adjustment
float calculate_ec_adjustment(float targetEC, float currentEC, float concentrationFactor, float tankVolume)
{
    if (concentrationFactor <= 0) 
    {
        //add diagnosis here
        // printf("Error: Concentration factor must be greater than zero.\n");
        // return 0;
        concentrationFactor = 1.0f;
    }
    //1500 - 300 positive
    adjustment = (targetEC - currentEC);// / concentrationFactor) * tankVolume;
    return adjustment;
}

int calculate_active_time_of_pump(float volume)
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
bool ec_is_positive_difference = false; //false means negative diff true means positive diff

ec_diff_pos ec_prev_diff_position = EC_POS_IDLE;
ec_diff_pos ec_current_diff_position = EC_POS_IDLE;


void EC_EXE_APPLICATION(void)
{
    if(signal_ctrl_water_level_start_ph_ec_unit == SENSOR_IC_SIGNAL_TRUE && signal_ctrl_water_level_stop_ph_ec_unit == SENSOR_IC_SIGNAL_FALSE)
    {
        signal_data_get_ec_water_volume_data = (float)signal_data_get_tank_capacity;
        switch (ec_app_current_state)
        {
            case EC_CALCULATE_EC_ADJ_VOLUME:
            {
#ifdef EC_APPLICATION_LOGS
                printf("EC_CALCULATE_EC_ADJ_VOLUME state entered\n");
#endif
                //set the ec calculated differece
                signal_data_set_ec_difference_data  = calculate_ec_adjustment(signal_data_get_target_ec_data, signal_data_get_ec_data, signal_data_get_ec_concentration_data, signal_data_get_ec_water_volume_data);
                //set the calculated req ec concentration volume
                signal_data_set_ec_req_volume       = calculate_solution_volume(signal_data_get_target_ec_data, signal_data_get_ec_data, signal_data_get_ec_concentration_data, signal_data_get_ec_water_volume_data);
#ifdef EC_APPLICATION_LOGS
                printf("signal_data_set_ec_difference_data %f signal_data_set_ec_req_volume %f\r\n", 
                    signal_data_set_ec_difference_data,
                    signal_data_set_ec_req_volume);
#endif
                if(signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_NEG && signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_POS)
                {
                    signal_ctrl_pipe_water_enable_from_ec = SENSOR_IC_SIGNAL_TRUE;
#ifdef EC_APPLICATION_LOGS
                    printf("EC within tolerance, enabling water pipe\n");
#endif
                }else
                {
                    signal_ctrl_pipe_water_enable_from_ec = SENSOR_IC_SIGNAL_FALSE;
#ifdef EC_APPLICATION_LOGS
                    printf("EC within tolerance, disabling water pipe\n");
#endif
                }
                //add the next state
                ec_app_current_state = EC_EXE_PUMPS;
            }
                break;
            case EC_EXE_PUMPS:
            {
#ifdef EC_APPLICATION_LOGS
                printf("EC_EXE_PUMPS state entered\n");
#endif
                if(ec_increase_get_current_time_assigned == false)
                {
#ifdef EC_APPLICATION_LOGS
                    printf("ec_increase_get_current_time_assigned => false\r\n");
#endif
                    //if difference is positive means that activate EC pumps
                    if(signal_data_set_ec_difference_data > 0)
                    {
#ifdef EC_APPLICATION_LOGS
                        printf("signal_data_set_ec_difference_data +\r\n");
#endif
                        signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;

                        if(signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_POS)
                        {
#ifdef EC_APPLICATION_LOGS
                            printf("signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_POS\r\n");
#endif
                            reference_time = &reference_time_value;
                            *reference_time = *signal_data_get_rtc_data;  /* copy struct value */                            //calculate required total time here 
                            //volume/motor_cap + 200s
                            required_time_for_ec_dosing = calculate_active_time_of_pump(signal_data_set_ec_req_volume);
#ifdef EC_APPLICATION_LOGS
                            printf("required_time_for_ec_dosing = %d seconds\r\n",required_time_for_ec_dosing);
#endif
                            ec_increase_get_current_time_assigned = true;
                            //if first time activate ready for motor operation
                            signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_TRUE;
                            signal_ctrl_mixer_motors_turn_on       = SENSOR_IC_SIGNAL_TRUE;
                            ec_prev_diff_position = EC_POS_POSITIVE;
#ifdef EC_APPLICATION_LOGS
                            printf("signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_POS ec pump %d mixer %d\r\n", 
                                signal_ctrl_peripheral_turn_on_ec_pump,
                                signal_ctrl_mixer_motors_turn_on);
#endif
                        }else
                        {
                            signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
                            signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_FALSE;
                            ec_prev_diff_position = EC_POS_POSITIVE;
#ifdef EC_APPLICATION_LOGS
                            printf("signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_POS ec pump %d mixer %d\r\n", 
                                signal_ctrl_peripheral_turn_on_ec_pump,
                                signal_ctrl_mixer_motors_turn_on);
#endif
                        }
                    }
                    //if difference is negative means that activate water in and out relays
                    if(signal_data_set_ec_difference_data < 0)
                    {
#ifdef EC_APPLICATION_LOGS
                        printf("signal_data_set_ec_difference_data -\r\n");
#endif
                        if(signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_NEG)
                        {
#ifdef EC_APPLICATION_LOGS
                        printf("signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_NEG\r\n");
#endif
                            reference_time = &reference_time_value;
                            *reference_time = *signal_data_get_rtc_data;  
                            required_time_for_ec_dosing = calculate_active_time_of_pump(signal_data_set_ec_req_volume);
#ifdef EC_APPLICATION_LOGS
                            printf("required_time_for_ec_dosing = %d seconds\r\n",required_time_for_ec_dosing);
#endif
                            ec_increase_get_current_time_assigned = true;
                            signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_TRUE;
                            //turn on water output
                            signal_ctrl_peripheral_turn_on_main_tank_water_output                   = SENSOR_IC_SIGNAL_TRUE;
                            signal_ctrl_peripheral_turn_off_main_tank_water_output                  = SENSOR_IC_SIGNAL_FALSE;
                            if(signal_data_get_water_level_data < 15)
                            {
                                //turn off water input
                                signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_FALSE;
                                signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_TRUE;
#ifdef EC_APPLICATION_LOGS
                                printf("if(signal_data_set_ec_difference_data < 0) turn off water input on signal %d off signal %d\r\n", 
                                    signal_ctrl_peripheral_turn_on_main_tank_water_input,
                                    signal_ctrl_peripheral_turn_off_main_tank_water_input);
#endif
                            }else
                            {
                                //turn on water input
                                signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_TRUE;
                                signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_FALSE;
#ifdef EC_APPLICATION_LOGS
                                printf("if(signal_data_set_ec_difference_data < 0) turn off water input on signal %d off signal %d\r\n", 
                                    signal_ctrl_peripheral_turn_on_main_tank_water_input,
                                    signal_ctrl_peripheral_turn_off_main_tank_water_input);
#endif
                            }
                            signal_ctrl_mixer_motors_turn_on                                        = SENSOR_IC_SIGNAL_TRUE;
                            ec_prev_diff_position = EC_POS_NEGATIVE;
                        }else
                        {
#ifdef EC_APPLICATION_LOGS
                            printf("signal_data_set_ec_difference_data => EC_POS_NEGATIVE\r\n");
#endif
                            ec_prev_diff_position = EC_POS_NEGATIVE;
                            signal_ctrl_ec_disable_wl_unit                                          = SENSOR_IC_SIGNAL_FALSE;
                            signal_ctrl_mixer_motors_turn_on                                        = SENSOR_IC_SIGNAL_FALSE;
                        }
                    }
                }else
                {
#ifdef EC_APPLICATION_LOGS
                    printf("ec_increase_get_current_time_assigned => true\r\n");
#endif
                    //if difference is positive means that activate EC pumps
                    if(signal_data_set_ec_difference_data > 0)
                    {
#ifdef EC_APPLICATION_LOGS
                        printf("signal_data_set_ec_difference_data > 0  \r\n");
#endif
                        ec_current_diff_position = EC_POS_POSITIVE;
                        if(ec_current_diff_position == ec_prev_diff_position)
                        {
                            signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;

                            if(signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_POS)
                            {
#ifdef EC_APPLICATION_LOGS
                                printf("signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_POS  \r\n");
#endif
                                //calculate the time which elapsed and compare with target time
                                //if first time activate ready for motor operation
                                calculated_time_difference = timeToSeconds(calculateTimeDifference(reference_time, signal_data_get_rtc_data));
                                
#ifdef EC_APPLICATION_LOGS
                                printf("time difference %d, required time for ec dosing %d\r\n", 
                                    calculated_time_difference, required_time_for_ec_dosing);
#endif
                                
                                if(calculated_time_difference < required_time_for_ec_dosing)
                                {
                                    signal_ctrl_peripheral_turn_on_ec_pump  = SENSOR_IC_SIGNAL_TRUE;
                                    signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_TRUE;
#ifdef EC_APPLICATION_LOGS
                                    printf("calculated_time_difference < required_time_for_ec_dosing  ec pump %d mixer %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_ec_pump,
                                            signal_ctrl_mixer_motors_turn_on);
#endif
                                }else
                                {
                                    //wait for mixing time now
                                    if(calculated_time_difference < (required_time_for_ec_dosing + signal_data_get_mixing_time))
                                    {
                                        signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
                                        signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_TRUE;
#ifdef EC_APPLICATION_LOGS
                                        printf("wait for mixing time now  ec pump %d mixer %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_ec_pump,
                                            signal_ctrl_mixer_motors_turn_on);
#endif
                                    }else
                                    {
                                        signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_FALSE;
                                        ec_achieved_targeted_time = true;
#ifdef EC_APPLICATION_LOGS
                                        printf("turn off mixer  ec pump %d mixer %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_ec_pump,
                                            signal_ctrl_mixer_motors_turn_on);
#endif
                                    }
                                }
                            }else
                            {
                                ec_achieved_targeted_time = true;
                                signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
                                signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_FALSE;
#ifdef EC_APPLICATION_LOGS
                                printf("achieved target time  ec pump %d mixer %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_ec_pump,
                                            signal_ctrl_mixer_motors_turn_on);
#endif
                            }
                        }else
                        {
                            signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;
                            signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_FALSE;
                            ec_increase_get_current_time_assigned = false;
#ifdef EC_APPLICATION_LOGS
                            printf("ec_current_diff_position == ec_prev_diff_position  ec pump %d mixer %d\r\n", 
                                signal_ctrl_peripheral_turn_on_ec_pump,
                                signal_ctrl_mixer_motors_turn_on);
#endif
                        }
                        
                    }
                    //if difference is negative means that activate water in and out relays
                    if(signal_data_set_ec_difference_data < 0)
                    {

#ifdef EC_APPLICATION_LOGS
                        printf("signal_data_set_ec_difference_data < 0  \r\n");
#endif
                        ec_current_diff_position = EC_POS_NEGATIVE;
                        if(ec_current_diff_position == ec_prev_diff_position)
                        {
                            if(signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_NEG)
                            {
#ifdef EC_APPLICATION_LOGS
                                printf("signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_NEG \r\n");
#endif
                                calculated_time_difference = timeToSeconds(calculateTimeDifference(reference_time, signal_data_get_rtc_data));
#ifdef EC_APPLICATION_LOGS
                                printf("time difference %d, required time for ec dosing %d\r\n", 
                                    calculated_time_difference, required_time_for_ec_dosing);
#endif
                                if(calculated_time_difference < required_time_for_ec_dosing)
                                {
                                    signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_TRUE;
                                    //turn on water output
                                    signal_ctrl_peripheral_turn_on_main_tank_water_output               = SENSOR_IC_SIGNAL_TRUE;
                                    signal_ctrl_peripheral_turn_off_main_tank_water_output              = SENSOR_IC_SIGNAL_FALSE;
                                    signal_ctrl_mixer_motors_turn_on                                    = SENSOR_IC_SIGNAL_TRUE;
#ifdef EC_APPLICATION_LOGS
                                    printf("turn on water output  main tnk on %d main tnk on %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_main_tank_water_output,
                                            signal_ctrl_peripheral_turn_off_main_tank_water_output);
#endif
                                    if(signal_data_get_water_level_data < 15)
                                    {
                                        //turn off water input
                                        signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_FALSE;
                                        signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_TRUE;
#ifdef EC_APPLICATION_LOGS
                                        printf("turn off water output  main tnk in on %d main tnk in off %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_main_tank_water_input,
                                            signal_ctrl_peripheral_turn_off_main_tank_water_input);
#endif
                                    }else
                                    {
                                        //turn on water input
                                        signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_TRUE;
                                        signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_FALSE;
#ifdef EC_APPLICATION_LOGS
                                        printf("turn off water output  main tnk in on %d main tnk in off %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_main_tank_water_input,
                                            signal_ctrl_peripheral_turn_off_main_tank_water_input);
#endif
                                    }
                                }else
                                {
                                    //wait for mixing time now
                                    if(calculated_time_difference < (required_time_for_ec_dosing + signal_data_get_mixing_time))
                                    {
                                        //turn off water input
                                        signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_FALSE;
                                        signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_TRUE;
                                        //turn off water output
                                        signal_ctrl_peripheral_turn_on_main_tank_water_output               = SENSOR_IC_SIGNAL_FALSE;
                                        signal_ctrl_peripheral_turn_off_main_tank_water_output              = SENSOR_IC_SIGNAL_TRUE;

                                        signal_ctrl_mixer_motors_turn_on                                    = SENSOR_IC_SIGNAL_TRUE;
                                        signal_ctrl_ec_disable_wl_unit                                      = SENSOR_IC_SIGNAL_FALSE;
#ifdef EC_APPLICATION_LOGS
                                        printf("wait for mixing time w in off w out off   main tnk  in on %d main tnk in off %d main tnk  o on %d main tnk o off %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_main_tank_water_input,
                                            signal_ctrl_peripheral_turn_off_main_tank_water_input,
                                            signal_ctrl_peripheral_turn_on_main_tank_water_output,
                                            signal_ctrl_peripheral_turn_off_main_tank_water_output);
#endif
                                    }else
                                    {
                                        signal_ctrl_mixer_motors_turn_on                                    = SENSOR_IC_SIGNAL_FALSE;
                                        ec_achieved_targeted_time = true;
#ifdef EC_APPLICATION_LOGS
                                        printf("turn off also the mixer now!! mixer %d\r\n", signal_ctrl_mixer_motors_turn_on);
#endif
                                    }
                                }
                            }else
                            {
                                //turn off water input
                                signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_FALSE;
                                signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_TRUE;
                                //turn off water output
                                signal_ctrl_peripheral_turn_on_main_tank_water_output               = SENSOR_IC_SIGNAL_FALSE;
                                signal_ctrl_peripheral_turn_off_main_tank_water_output              = SENSOR_IC_SIGNAL_FALSE;

                                signal_ctrl_mixer_motors_turn_on                                    = SENSOR_IC_SIGNAL_FALSE;

                                signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
#ifdef EC_APPLICATION_LOGS
                                printf("w in off w out off   main tnk  in on %d main tnk in off %d main tnk  o on %d main tnk o off %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_main_tank_water_input,
                                            signal_ctrl_peripheral_turn_off_main_tank_water_input,
                                            signal_ctrl_peripheral_turn_on_main_tank_water_output,
                                            signal_ctrl_peripheral_turn_off_main_tank_water_output);
#endif
                            }
                        }
                    }
                }
                ec_app_current_state = EC_CALCULATE_ADDED_EC_VOL_DIFF_ON_TANK;

                if(ec_achieved_targeted_time == true)
                {
#ifdef EC_APPLICATION_LOGS
                    printf("ec_achieved_targeted_time == true\r\n");
#endif
                    ec_increase_get_current_time_assigned = false;
                    ec_app_current_state = EC_CALCULATE_EC_ADJ_VOLUME;

                }
                
            }
                break;
            case EC_CALCULATE_ADDED_EC_VOL_DIFF_ON_TANK:
            {
#ifdef EC_APPLICATION_LOGS
                printf("EC_CALCULATE_ADDED_EC_VOL_DIFF_ON_TANK state entered\n");
#endif
                //this part is for dosing diagnsostics
                if(signal_data_set_ec_difference_data > 0)
                {
                    if(check_the_dosing_process_counter < 100)
                    {
                        check_the_dosing_process_counter++;
                    }else
                    {
                        check_the_dosing_process_counter = 0;
                        if(place_to_buffer < 32)
                        {
                            volume_buffer_mean_value  = 0;
                            ec_diff_buffer_mean_value = 0;
                            calculated_volume_buffer[place_to_buffer] = calculate_solution_volume(signal_data_get_target_ec_data, signal_data_get_ec_data, signal_data_get_ec_concentration_data, signal_data_get_ec_water_volume_data); //signal_data_set_ec_req_volume;
                            calculated_ec_diff_buffer[place_to_buffer] = calculate_ec_adjustment(signal_data_get_target_ec_data, signal_data_get_ec_data, signal_data_get_ec_concentration_data, signal_data_get_ec_water_volume_data); //signal_data_set_ec_difference_data;
                            for(for_loop_counter = 0; for_loop_counter < 32; for_loop_counter++)
                            {
                                volume_buffer_sum  += calculated_volume_buffer[for_loop_counter];
                                ec_diff_buffer_sum += calculated_ec_diff_buffer[for_loop_counter];
                            }
                            volume_buffer_mean_value  = volume_buffer_sum / 32;
                            ec_diff_buffer_mean_value = ec_diff_buffer_sum / 32;
                            place_to_buffer++;
                        }else
                        {
                            place_to_buffer = 0;
                        }
                    }
                }

                if(signal_data_set_ec_difference_data < 0)
                {
                    if(check_the_dosing_process_counter < 100)
                    {
                        check_the_dosing_process_counter++;
                    }else
                    {
                        check_the_dosing_process_counter = 0;
                        if(place_to_buffer < 32)
                        {
                            volume_buffer_mean_value  = 0;
                            ec_diff_buffer_mean_value = 0;
                            calculated_volume_buffer[place_to_buffer] = calculate_solution_volume(signal_data_get_target_ec_data, signal_data_get_ec_data, signal_data_get_ec_concentration_data, signal_data_get_ec_water_volume_data); //signal_data_set_ec_req_volume;
                            calculated_ec_diff_buffer[place_to_buffer] = calculate_ec_adjustment(signal_data_get_target_ec_data, signal_data_get_ec_data, signal_data_get_ec_concentration_data, signal_data_get_ec_water_volume_data); //signal_data_set_ec_difference_data;
                            for(for_loop_counter = 0; for_loop_counter < 32; for_loop_counter++)
                            {
                                volume_buffer_sum  += calculated_volume_buffer[for_loop_counter];
                                ec_diff_buffer_sum += calculated_ec_diff_buffer[for_loop_counter];
                            }
                            volume_buffer_mean_value  = volume_buffer_sum / 32;
                            ec_diff_buffer_mean_value = ec_diff_buffer_sum / 32;
                            place_to_buffer++;
                        }else
                        {
                            place_to_buffer = 0;
                        }
                    }
                }
                if(signal_data_set_ec_difference_data > EC_DIFF_TOLERANCE_NEG && signal_data_set_ec_difference_data < EC_DIFF_TOLERANCE_POS)
                {
                    ec_app_current_state = EC_CALCULATE_EC_ADJ_VOLUME;
                }else
                {
                    ec_app_current_state = EC_EXE_PUMPS;
                }
            }
                break;
    
            default:
            {
    
            }
                break;
        }
    }
}