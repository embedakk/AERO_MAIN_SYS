#include "../inc/ph_application.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"

//rtc
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;

DS1302_DateTime ph_reference_time_value;
DS1302_DateTime *ph_reference_time = NULL;

ph_state_machine ph_app_current_state = PH_CALCULATE_PH_ADJ_VOLUME;

// Convert time to total seconds
int ph_timeToSeconds(DS1302_DateTime t) 
{
    return (t.hour * 3600) + (t.minute * 60) + t.second;
}

// Convert total seconds back to RTC_Time
DS1302_DateTime ph_secondsToTime(int totalSeconds) 
{
    DS1302_DateTime t;
    t.hour = totalSeconds / 3600;
    t.minute = (totalSeconds % 3600) / 60;
    t.second = totalSeconds % 60;
    return t;
}

// Function to calculate time difference with proper minute and second adjustments
DS1302_DateTime ph_calculateTimeDifference(DS1302_DateTime *start, DS1302_DateTime *end) 
{
    DS1302_DateTime diff;
    
    int startSeconds = ph_timeToSeconds(*start);
    int endSeconds = ph_timeToSeconds(*end);

    // Handle midnight rollover
    if (endSeconds < startSeconds) {
        endSeconds += 24 * 3600;
    }

    int duration = endSeconds - startSeconds;
    diff = ph_secondsToTime(duration);

    return diff;
}

// Function to calculate PH adjustment
float calculate_ph_adjustment(float targetPH, float currentPH, float concentrationFactor, float tankVolume)
{
    if (concentrationFactor <= 0.0f) {
        // fallback: just return pH delta if no concentration info
        adjustment = (targetPH - currentPH);
    } else {
        // amount (in arbitrary units) scaled by concentration and tank volume
        adjustment = (targetPH - currentPH); //((targetPH - currentPH) / (concentrationFactor / 1000.0f)) * tankVolume;
    }
    return adjustment;
}

// Solution Type	                        Common Chemical	                    Typical Molarity	    Meaning
// pH Down	                                Nitric acid (HNO₃)	                    1 M	                1 mole H⁺ per liter
// pH Down	                                Phosphoric acid (H₃PO₄, 85%)	        ~14.7 M	            Very strong, usually diluted
// pH Down	                                Sulfuric acid (H₂SO₄, 5%)	            ~0.9 M	            Safe common concentration
// pH Up	                                Potassium hydroxide (KOH, 10%)	        ~1.8 M	            1.8 mol OH⁻ per liter
// For example, if you take 100 mL of 85% H₃PO₄ (~14.7 M) and dilute to 1 L total volume:
//concetration = (14.7 M * 0.1 L) / 1 L = 1.47 M
//solution_conc = 1470.0f; //in mM

// Function to calculate the volume of PH solution to be added
// Function to calculate the required volume of acid/base
double calculate_adjustment_volume(double tank_volume, double current_pH, double target_pH, double solution_conc) 
{
    //
    // Convert pH to H+ ion concentration
    double H_current = pow(10, -current_pH);
    double H_target = pow(10, -target_pH);

    // Change in H+ ion concentration
    double delta_H = fabs(H_target - H_current);

    // Required moles of H+ or OH- to adjust the pH
    double required_moles = delta_H * tank_volume;

    double acid_concentration_M = solution_conc / 1000.0;

    // Required volume of acid/base solution
    // double solution_volume = required_moles / (solution_conc / 1000.0); // Convert mM to mol/L
    double solution_volume = required_moles / acid_concentration_M;

    return solution_volume;  // in liters
}

//dosing intensity
int ph_calculate_active_time_of_pump(float volume)
{
    if(signal_data_get_dosing_intensity <= 0)
    {
        signal_data_get_dosing_intensity = 1;
    }
    // return (int)(((volume * 0.6)/ (signal_data_get_dosing_intensity)) * 60);
    // dosing intensity ml/min
    return (int)(((volume * 700.0f) / (signal_data_get_dosing_intensity)) * 60);
}

bool ph_motor_activation_first_shot = false;
bool ph_increase_get_current_time_assigned = false;
bool ph_achieved_targeted_time = false;
bool ph_is_positive_difference = false; //false means negative diff true means positive diff

ph_diff_pos ph_prev_diff_position = PH_POS_IDLE;
ph_diff_pos ph_current_diff_position = PH_POS_IDLE;


void PH_EXE_APPLICATION(void)
{
    if(signal_ctrl_water_level_start_ph_ec_unit == SENSOR_IC_SIGNAL_TRUE && signal_ctrl_water_level_stop_ph_ec_unit == SENSOR_IC_SIGNAL_FALSE)
    {
        signal_data_get_ph_water_volume_data = signal_data_get_tank_capacity;
        switch (ph_app_current_state)
        {
            case PH_CALCULATE_PH_ADJ_VOLUME:
            {
#ifdef PH_APPLICATION_LOGS
                printf("PH_CALCULATE_PH_ADJ_VOLUME state entered\n");
#endif
                //set the ph calculated differece
                signal_data_set_ph_difference_data  = calculate_ph_adjustment(signal_data_get_target_ph_data, signal_data_get_ph_data, signal_data_get_ph_concentration_data, signal_data_get_ph_water_volume_data);
                //set the calculated req ec concentration volume
                signal_data_set_ph_req_volume       = calculate_adjustment_volume(signal_data_get_ph_water_volume_data, signal_data_get_ph_data, signal_data_get_target_ph_data, signal_data_get_ph_concentration_data);
                if(signal_data_set_ph_difference_data > PH_DIFF_TOLERANCE_NEG && signal_data_set_ph_difference_data < PH_DIFF_TOLERANCE_POS)
                {
                    signal_ctrl_pipe_water_enable_from_ph = SENSOR_IC_SIGNAL_TRUE;
#ifdef PH_APPLICATION_LOGS
                    printf("PH within tolerance, enabling water pipe\n");
#endif

                }else
                {
                    signal_ctrl_pipe_water_enable_from_ph = SENSOR_IC_SIGNAL_FALSE;
#ifdef PH_APPLICATION_LOGS
                    printf("PH within tolerance, disabling water pipe\n");
#endif

                }
                //add the next state
                ph_app_current_state = PH_EXE_PUMPS;
            }
                break;
            case PH_EXE_PUMPS:
            {
#ifdef PH_APPLICATION_LOGS
                printf("PH_EXE_PUMPS state entered\n");
#endif
                if(ph_increase_get_current_time_assigned == false)
                {
#ifdef PH_APPLICATION_LOGS
                    printf("ph_increase_get_current_time_assigned => false\r\n");
#endif
                    //if difference is positive means that activate water in and out relays
                    if(signal_data_set_ph_difference_data > 0)
                    {
#ifdef PH_APPLICATION_LOGS
                        printf("signal_data_set_ph_difference_data +\r\n");
#endif
                        if(signal_data_set_ph_difference_data > PH_DIFF_TOLERANCE_POS)
                        {
#ifdef PH_APPLICATION_LOGS
                            printf("signal_data_set_ph_difference_data > PH_DIFF_TOLERANCE_POS\r\n");
#endif
                            // reference_time = signal_data_get_rtc_data;
                            /* when assigning start time (replace these lines)
                               reference_time = signal_data_get_rtc_data;
                            */
                            /* use a copy instead: */
                            ph_reference_time = &ph_reference_time_value;
                            *ph_reference_time = *signal_data_get_rtc_data;  /* copy struct value */
                            required_time_for_ph_dosing = ph_calculate_active_time_of_pump(signal_data_set_ph_req_volume * 1000.0f);
#ifdef PH_APPLICATION_LOGS
                            printf("required_time_for_ph_dosing = %d seconds\r\n",required_time_for_ph_dosing);
#endif
                            ph_increase_get_current_time_assigned = true;
                            signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_TRUE;
                            //turn on water output
                            signal_ctrl_peripheral_turn_on_main_tank_water_output                   = SENSOR_IC_SIGNAL_TRUE;
                            signal_ctrl_peripheral_turn_off_main_tank_water_output                  = SENSOR_IC_SIGNAL_FALSE;
                            if(signal_data_get_water_level_data < 15)
                            {
#ifdef PH_APPLICATION_LOGS
                                printf("if(signal_data_set_ph_difference_data > 0) turn off water input on signal %d off signal %d\r\n", 
                                    signal_ctrl_peripheral_turn_on_main_tank_water_input,
                                    signal_ctrl_peripheral_turn_off_main_tank_water_input);
#endif
                                //turn off water input
                                signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_FALSE;
                                signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_TRUE;
                            }else
                            {
#ifdef PH_APPLICATION_LOGS
                                printf("if(signal_data_set_ph_difference_data > 0) turn on water inputon signal %d off signal %d\r\n", 
                                    signal_ctrl_peripheral_turn_on_main_tank_water_input,
                                    signal_ctrl_peripheral_turn_off_main_tank_water_input);
#endif
                                //turn on water input
                                signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_TRUE;
                                signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_FALSE;
                            }
                            signal_ctrl_mixer_motors_turn_on                                        = SENSOR_IC_SIGNAL_TRUE;
                            ph_prev_diff_position                                                   = PH_POS_POSITIVE;
                        }else
                        {
#ifdef PH_APPLICATION_LOGS
                            printf("signal_data_set_ph_difference_data =< PH_DIFF_TOLERANCE_POS\r\n");
#endif
                            ph_prev_diff_position                                                   = PH_POS_POSITIVE;
                            signal_ctrl_ph_disable_wl_unit                                          = SENSOR_IC_SIGNAL_FALSE;
                            signal_ctrl_mixer_motors_turn_on                                        = SENSOR_IC_SIGNAL_FALSE;
                        }
                    }
                    //if difference is negative means that activate ph pump
                    if(signal_data_set_ph_difference_data < 0)
                    {
#ifdef PH_APPLICATION_LOGS
                        printf("signal_data_set_ph_difference_data -\r\n");
#endif
                        signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
                        if(signal_data_set_ph_difference_data < PH_DIFF_TOLERANCE_NEG)
                        {   
                            // reference_time = signal_data_get_rtc_data;
                            /* when assigning start time (replace these lines)
                               reference_time = signal_data_get_rtc_data;
                            */
                            /* use a copy instead: */
                            ph_reference_time = &ph_reference_time_value;
                            *ph_reference_time = *signal_data_get_rtc_data;  /* copy struct value */

                            required_time_for_ph_dosing = ph_calculate_active_time_of_pump(signal_data_set_ph_req_volume * 1000.0f);
#ifdef PH_APPLICATION_LOGS
                            printf("required_time_for_ph_dosing = %d seconds\r\n",required_time_for_ph_dosing);
#endif
                            ph_increase_get_current_time_assigned   = true;
                            signal_ctrl_peripheral_turn_on_ph_pump  = SENSOR_IC_SIGNAL_TRUE;
                            signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_TRUE;
                            ph_prev_diff_position                   = PH_POS_NEGATIVE;
#ifdef PH_APPLICATION_LOGS
                            printf("signal_data_set_ph_difference_data < PH_DIFF_TOLERANCE_NEG ph pump %d mixer %d\r\n", 
                                signal_ctrl_peripheral_turn_on_ph_pump,
                                signal_ctrl_mixer_motors_turn_on);
#endif
                        }else
                        {
                            
                            signal_ctrl_peripheral_turn_on_ph_pump  = SENSOR_IC_SIGNAL_FALSE;
                            signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_FALSE;
                            ph_prev_diff_position                   = PH_POS_NEGATIVE;
#ifdef PH_APPLICATION_LOGS
                            printf("signal_data_set_ph_difference_data >= PH_DIFF_TOLERANCE_NEG ph pump %d mixer %d\r\n", 
                                signal_ctrl_peripheral_turn_on_ph_pump,
                                signal_ctrl_mixer_motors_turn_on);
#endif
                        }
                    }
                }else
                {
#ifdef PH_APPLICATION_LOGS
                    printf("ph_increase_get_current_time_assigned => true\r\n");
#endif
                    if(signal_data_set_ph_difference_data < 0)
                    {
#ifdef PH_APPLICATION_LOGS
                        printf("signal_data_set_ph_difference_data < 0  \r\n");
#endif
                        ph_current_diff_position = PH_POS_NEGATIVE;
                        if (ph_current_diff_position == ph_prev_diff_position)
                        {
                            signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
                            if(signal_data_set_ph_difference_data < PH_DIFF_TOLERANCE_NEG)
                            {
#ifdef PH_APPLICATION_LOGS
                                printf("signal_data_set_ph_difference_data < PH_DIFF_TOLERANCE_NEG  \r\n");
#endif
                                calculated_time_difference = ph_timeToSeconds(ph_calculateTimeDifference(ph_reference_time, signal_data_get_rtc_data));
#ifdef PH_APPLICATION_LOGS
                                printf("time difference %d, required time for ph dosing %d\r\n", 
                                    calculated_time_difference, required_time_for_ph_dosing);
#endif
                                if(calculated_time_difference < required_time_for_ph_dosing)
                                {
                                    
                                    signal_ctrl_peripheral_turn_on_ec_pump  = SENSOR_IC_SIGNAL_TRUE;
                                    signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_TRUE;
#ifdef PH_APPLICATION_LOGS
                                    printf("calculated_time_difference < required_time_for_ph_dosing  ph pump %d mixer %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_ph_pump,
                                            signal_ctrl_mixer_motors_turn_on);
#endif
                                }else
                                {
                                    //wait for mixing time now
                                    if(calculated_time_difference < (required_time_for_ph_dosing + signal_data_get_mixing_time))
                                    {
                                        
                                        signal_ctrl_peripheral_turn_on_ph_pump  = SENSOR_IC_SIGNAL_FALSE;
                                        signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_TRUE;
#ifdef PH_APPLICATION_LOGS
                                        printf("wait for mixing time now  ph pump %d mixer %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_ph_pump,
                                            signal_ctrl_mixer_motors_turn_on);
#endif
                                    }else
                                    {
                                        
                                        signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_FALSE;
                                        ph_achieved_targeted_time               = true;
#ifdef PH_APPLICATION_LOGS
                                        printf("turn off mixer  ph pump %d mixer %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_ph_pump,
                                            signal_ctrl_mixer_motors_turn_on);
#endif
                                    }
                                }
                            }else
                            {
                                ph_achieved_targeted_time = true;
                                
                                signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_FALSE;
                                signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_FALSE;
#ifdef PH_APPLICATION_LOGS
                                printf("achieved target time  ph pump %d mixer %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_ph_pump,
                                            signal_ctrl_mixer_motors_turn_on);
#endif
                            }
                        }else
                        {
                            signal_ctrl_peripheral_turn_on_ph_pump  = SENSOR_IC_SIGNAL_FALSE;
                            signal_ctrl_mixer_motors_turn_on        = SENSOR_IC_SIGNAL_FALSE;
                            ph_increase_get_current_time_assigned   = false;
#ifdef PH_APPLICATION_LOGS
                            printf("signal_data_set_ph_difference_data < 0  ph pump %d mixer %d\r\n", 
                                signal_ctrl_peripheral_turn_on_ph_pump,
                                signal_ctrl_mixer_motors_turn_on);
#endif
                        }
                    }

                    if(signal_data_set_ph_difference_data > 0)
                    {
#ifdef PH_APPLICATION_LOGS
                        printf("signal_data_set_ph_difference_data > 0  \r\n");
#endif
                        ph_current_diff_position = PH_POS_POSITIVE;
                        if (ph_current_diff_position == ph_prev_diff_position)
                        {
                            if(signal_data_set_ph_difference_data > PH_DIFF_TOLERANCE_POS)
                            {
#ifdef PH_APPLICATION_LOGS
                                printf("signal_data_set_ph_difference_data > PH_DIFF_TOLERANCE_POS  \r\n");
#endif
                                calculated_time_difference = ph_timeToSeconds(ph_calculateTimeDifference(ph_reference_time, signal_data_get_rtc_data));
#ifdef PH_APPLICATION_LOGS
                                printf("time difference %d, required time for ph dosing %d\r\n", 
                                    calculated_time_difference, required_time_for_ph_dosing);
#endif
                                if(calculated_time_difference < required_time_for_ph_dosing)
                                {
                                    signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_TRUE;
                                    //turn on water output
                                    signal_ctrl_peripheral_turn_on_main_tank_water_output               = SENSOR_IC_SIGNAL_TRUE;
                                    signal_ctrl_peripheral_turn_off_main_tank_water_output              = SENSOR_IC_SIGNAL_FALSE;
                                    signal_ctrl_mixer_motors_turn_on                                    = SENSOR_IC_SIGNAL_TRUE;
#ifdef PH_APPLICATION_LOGS
                                    printf("turn on water output  main tnk on %d main tnk on %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_main_tank_water_output,
                                            signal_ctrl_peripheral_turn_off_main_tank_water_output);
#endif
                                    if(signal_data_get_water_level_data < 15)
                                    {
                                        //turn off water input
                                        signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_FALSE;
                                        signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_TRUE;
#ifdef PH_APPLICATION_LOGS
                                        printf("turn off water output  main tnk on %d main tnk on %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_main_tank_water_output,
                                            signal_ctrl_peripheral_turn_off_main_tank_water_output);
#endif
                                    }else
                                    {
                                        //turn on water input
                                        signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_TRUE;
                                        signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_FALSE;
#ifdef PH_APPLICATION_LOGS
                                        printf("turn on water output  main tnk on %d main tnk on %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_main_tank_water_output,
                                            signal_ctrl_peripheral_turn_off_main_tank_water_output);
#endif

                                    }
                                }else
                                {
                                    //wait for mixing time now
                                    if(calculated_time_difference < (required_time_for_ph_dosing + signal_data_get_mixing_time))
                                    {
                                        //turn off water input
                                        signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_FALSE;
                                        signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_TRUE;
                                        //turn off water output
                                        signal_ctrl_peripheral_turn_on_main_tank_water_output               = SENSOR_IC_SIGNAL_FALSE;
                                        signal_ctrl_peripheral_turn_off_main_tank_water_output              = SENSOR_IC_SIGNAL_TRUE;
                                    
                                        signal_ctrl_mixer_motors_turn_on                                    = SENSOR_IC_SIGNAL_TRUE;
                                        signal_ctrl_ph_disable_wl_unit                                      = SENSOR_IC_SIGNAL_FALSE;
#ifdef PH_APPLICATION_LOGS
                                        printf("wait for mixing time w in off w out off   main tnk  in on %d main tnk in off %d main tnk  o on %d main tnk o off %d\r\n", 
                                            signal_ctrl_peripheral_turn_on_main_tank_water_input,
                                            signal_ctrl_peripheral_turn_off_main_tank_water_input,
                                            signal_ctrl_peripheral_turn_on_main_tank_water_output,
                                            signal_ctrl_peripheral_turn_off_main_tank_water_output);
#endif
                                    }else
                                    {
                                        signal_ctrl_mixer_motors_turn_on                                    = SENSOR_IC_SIGNAL_FALSE;
                                        ph_achieved_targeted_time = true;
#ifdef PH_APPLICATION_LOGS
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

                                signal_ctrl_ph_disable_wl_unit                                      = SENSOR_IC_SIGNAL_FALSE;
#ifdef PH_APPLICATION_LOGS
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
                ph_app_current_state = PH_CALCULATE_ADDED_PH_VOL_DIFF_ON_TANK;
                if(ph_achieved_targeted_time == true)
                {
#ifdef PH_APPLICATION_LOGS
                    printf("ph_achieved_targeted_time == true\r\n");
#endif
                    ph_increase_get_current_time_assigned = false;
                    ph_app_current_state = PH_CALCULATE_PH_ADJ_VOLUME;
                }
            }
                break;
            case PH_CALCULATE_ADDED_PH_VOL_DIFF_ON_TANK:
            {
#ifdef PH_APPLICATION_LOGS
                printf("PH_CALCULATE_ADDED_PH_VOL_DIFF_ON_TANK state entered\n");
#endif
                //this part is for dosing diagnsostics
                if(signal_data_set_ph_difference_data > 0)
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
                            ph_diff_buffer_mean_value = 0;
                            calculated_volume_buffer[place_to_buffer] = calculate_adjustment_volume(signal_data_get_ph_water_volume_data, signal_data_get_ph_data, signal_data_get_target_ph_data, signal_data_get_ph_concentration_data);
                            //(targetPH - currentPH)
                            calculated_ph_diff_buffer[place_to_buffer] = calculate_ph_adjustment(signal_data_get_target_ph_data, signal_data_get_ph_data, signal_data_get_ph_concentration_data, signal_data_get_ph_water_volume_data); // signal_data_set_ph_difference_data;
                            for(for_loop_counter = 0; for_loop_counter < 32; for_loop_counter++)
                            {
                                volume_buffer_sum  += calculated_volume_buffer[for_loop_counter];
                                ph_diff_buffer_sum += calculated_ph_diff_buffer[for_loop_counter];
                            }
                            volume_buffer_mean_value  = volume_buffer_sum / 32;
                            ph_diff_buffer_mean_value = ph_diff_buffer_sum / 32;
                            place_to_buffer++;
                        }else
                        {
                            place_to_buffer = 0;
                        }
                    }
                }

                if(signal_data_set_ph_difference_data < 0)
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
                            ph_diff_buffer_mean_value = 0;
                            calculated_volume_buffer[place_to_buffer] = calculate_adjustment_volume(signal_data_get_ph_water_volume_data, signal_data_get_ph_data, signal_data_get_target_ph_data, signal_data_get_ph_concentration_data);
                            calculated_ph_diff_buffer[place_to_buffer] = calculate_ph_adjustment(signal_data_get_target_ph_data, signal_data_get_ph_data, signal_data_get_ph_concentration_data, signal_data_get_ph_water_volume_data);
                            for(for_loop_counter = 0; for_loop_counter < 32; for_loop_counter++)
                            {
                                volume_buffer_sum  += calculated_volume_buffer[for_loop_counter];
                                ph_diff_buffer_sum += calculated_ph_diff_buffer[for_loop_counter];
                            }
                            volume_buffer_mean_value  = volume_buffer_sum / 32;
                            ph_diff_buffer_mean_value = ph_diff_buffer_sum / 32;
                            place_to_buffer++;
                        }else
                        {
                            place_to_buffer = 0;
                        }
                    }
                }
                if(signal_data_set_ph_difference_data > PH_DIFF_TOLERANCE_NEG && signal_data_set_ph_difference_data < PH_DIFF_TOLERANCE_POS)
                {
                    ph_app_current_state = PH_CALCULATE_PH_ADJ_VOLUME;
                }else
                {
                    ph_app_current_state = PH_EXE_PUMPS;
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