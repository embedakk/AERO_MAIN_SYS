#include "../inc/wl_application.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"

wl_state_machine wl_sm_current_state = WL_IDLE_STATE;
float total_volume; 
float filled_volume;
float calculated_value;

// Function to calculate volume percentage
float calculate_percentage(float length, float width, float height, float filled_height) 
{
    // total_volume = length * width * filled_height; modified to line 13 
    total_volume = length * width * (height - filled_height);
    filled_volume = length * width * filled_height;
    calculated_value =  (filled_volume / total_volume) * 100;
    if(calculated_value > 100.0)
    {
        calculated_value = 100.0;
    }
    return (100.0f - calculated_value);
}

float WL_GET_PERCENTAGE(void)
{
    //based on the application change the values this parameters are for 1.5 Liter tank
    //signal_data_get_water_level_data these must to be height - signal_data_get_water_level_data
    return calculate_percentage(120.0, 250.0f, 120.0f, signal_data_get_water_level_data);
}

void WL_EXE_APPLICATION(void)
{
    //here add a signal from pipe watering side or disable the WL logic wait for 1 or 2 minutes till the 
    //water from pipes will come back
    if(signal_ctrl_system_start == SENSOR_IC_SIGNAL_TRUE || signal_ctrl_system_continue == SENSOR_IC_SIGNAL_TRUE)
    {
        //check for ec and ph disable signals
        if(signal_ctrl_ph_disable_wl_unit == SENSOR_IC_SIGNAL_TRUE || signal_ctrl_ec_disable_wl_unit == SENSOR_IC_SIGNAL_TRUE)
        {
            //nothing to do on water level unit while detected a disable signal which camed form EC or PH unit for
            //dosing stuffs
            signal_ctrl_circulation_pumps_turn_on                   = SENSOR_IC_SIGNAL_TRUE;
        }else
        {
            switch (wl_sm_current_state)
            {
                case WL_IDLE_STATE:
                {
                    //get the water level value
                    if(signal_data_get_water_level_data >= WL_LOW_THRESHOLD_CM)//tank is empty
                    {
                        //nothing to do on dosing side
                        signal_ctrl_water_level_start_ph_ec_unit                = SENSOR_IC_SIGNAL_FALSE;
                        signal_ctrl_water_level_stop_ph_ec_unit                 = SENSOR_IC_SIGNAL_TRUE;
                        signal_ctrl_ph_activate_get_ph                          = SENSOR_IC_SIGNAL_FALSE;
                        signal_ctrl_ec_activate_get_ec                          = SENSOR_IC_SIGNAL_FALSE;
                        wl_sm_current_state = WL_FILL_THE_TANK;
                    }else if(signal_data_get_water_level_data <= WL_CRITICAL_THRESHOLD_CM) // tank is full and is on critical range
                    {
                        signal_ctrl_ph_activate_get_ph                          = SENSOR_IC_SIGNAL_FALSE;
                        signal_ctrl_ec_activate_get_ec                          = SENSOR_IC_SIGNAL_FALSE;
                        wl_sm_current_state = WL_DISCHARGE_THE_TANK;
                    }
                    else if(signal_data_get_water_level_data >= WL_CRITICAL_THRESHOLD_CM && signal_data_get_water_level_data <= WL_HIGH_THRESHOLD_CM) // tank is full
                    {
                        signal_ctrl_ph_activate_get_ph                          = SENSOR_IC_SIGNAL_TRUE;
                        signal_ctrl_ec_activate_get_ec                          = SENSOR_IC_SIGNAL_TRUE;
                        wl_sm_current_state = WL_TANK_IS_FULL;
                    
                    }else if(signal_data_get_water_level_data < WL_LOW_THRESHOLD_CM && signal_data_get_water_level_data > WL_HIGH_THRESHOLD_CM)
                    {
                        signal_ctrl_ph_activate_get_ph                          = SENSOR_IC_SIGNAL_TRUE;
                        signal_ctrl_ec_activate_get_ec                          = SENSOR_IC_SIGNAL_TRUE;
                        wl_sm_current_state = WL_TANK_IS_READY_FOR_DOSING;
                    }
                }
                    break;
                case WL_TANK_IS_FULL:
                {
                    signal_ctrl_water_level_start_ph_ec_unit                = SENSOR_IC_SIGNAL_TRUE;
                    signal_ctrl_water_level_stop_ph_ec_unit                 = SENSOR_IC_SIGNAL_FALSE;

                    signal_ctrl_peripheral_turn_on_main_tank_water_input    = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_input   = SENSOR_IC_SIGNAL_TRUE;
                
                    // be sure that  water out relay is deactivated
                    signal_ctrl_peripheral_turn_on_main_tank_water_output   = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_output  = SENSOR_IC_SIGNAL_TRUE;

                    wl_sm_current_state = WL_IDLE_STATE;
                }
                    break;
                case WL_TANK_IS_READY_FOR_DOSING:
                {
                    //start dosing stuffs
                    signal_ctrl_water_level_start_ph_ec_unit                = SENSOR_IC_SIGNAL_TRUE;
                    signal_ctrl_water_level_stop_ph_ec_unit                 = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_circulation_pumps_turn_on                   = SENSOR_IC_SIGNAL_TRUE;
                    wl_sm_current_state = WL_FILL_THE_TANK;
                }
                    break;
                case WL_FILL_THE_TANK:
                {
                    // activate the water in relay
                    signal_ctrl_peripheral_turn_on_main_tank_water_input    = SENSOR_IC_SIGNAL_TRUE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_input   = SENSOR_IC_SIGNAL_FALSE;
                
                    // be sure that  water out relay is deactivated
                    signal_ctrl_peripheral_turn_on_main_tank_water_output   = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_output  = SENSOR_IC_SIGNAL_TRUE;
                    signal_ctrl_circulation_pumps_turn_on                   = SENSOR_IC_SIGNAL_TRUE;
                    wl_sm_current_state = WL_IDLE_STATE;
                }
                    break;
                case WL_DISCHARGE_THE_TANK:
                {
                    // activate the water out relay
                    signal_ctrl_peripheral_turn_on_main_tank_water_output   = SENSOR_IC_SIGNAL_TRUE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_output  = SENSOR_IC_SIGNAL_FALSE;
                
                    // be sure that  water in relay is deactivated
                    signal_ctrl_peripheral_turn_on_main_tank_water_input    = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_input   = SENSOR_IC_SIGNAL_TRUE;
                    wl_sm_current_state = WL_IDLE_STATE;
                }
                    break;
                case WL_CONTROL_DUTY_TO_ANOTHER_UNIT:
                {
                    wl_sm_current_state = WL_IDLE_STATE;
                }
                    break;
                
                default:
                {
                    //add diagnostic here
                }
                    break;
            }
        }
    }else if(signal_ctrl_system_stop == SENSOR_IC_SIGNAL_TRUE)
    {
        //turn off all stuffs but keep the values same 
        // activate the water out relay
        signal_ctrl_peripheral_turn_on_main_tank_water_output   = SENSOR_IC_SIGNAL_FALSE;
        signal_ctrl_peripheral_turn_off_main_tank_water_output  = SENSOR_IC_SIGNAL_TRUE;

        // be sure that  water out relay is deactivated
        signal_ctrl_peripheral_turn_on_main_tank_water_input    = SENSOR_IC_SIGNAL_FALSE;
        signal_ctrl_peripheral_turn_off_main_tank_water_input   = SENSOR_IC_SIGNAL_TRUE;

        signal_ctrl_peripheral_turn_on_main_pump                = SENSOR_IC_SIGNAL_FALSE;
        signal_ctrl_peripheral_turn_off_main_pump               = SENSOR_IC_SIGNAL_TRUE;

        signal_ctrl_led_turn_on_led                             = SENSOR_IC_SIGNAL_FALSE;

        signal_ctrl_circulation_pumps_turn_on                   = SENSOR_IC_SIGNAL_FALSE;

        signal_ctrl_peripheral_turn_on_ph_pump                  = SENSOR_IC_SIGNAL_FALSE;
        signal_ctrl_peripheral_turn_on_ec_pump                  = SENSOR_IC_SIGNAL_FALSE;
        
        wl_sm_current_state                                     = WL_IDLE_STATE;

    }else if(signal_ctrl_system_continue == SENSOR_IC_SIGNAL_TRUE)
    {
        signal_ctrl_system_start                                = SENSOR_IC_SIGNAL_TRUE;
        wl_sm_current_state                                     = WL_IDLE_STATE;

    }else if(signal_ctrl_system_delete == SENSOR_IC_SIGNAL_TRUE)
    {
        SENSORS_INIT_SIGNALS();
        wl_sm_current_state                                     = WL_IDLE_STATE;

    }else if(signal_ctrl_system_release_the_tank == SENSOR_IC_SIGNAL_TRUE)
    {
        SENSORS_INIT_SIGNALS();
        // activate the water out relay
        signal_ctrl_peripheral_turn_on_main_tank_water_output   = SENSOR_IC_SIGNAL_TRUE;
        signal_ctrl_peripheral_turn_off_main_tank_water_output  = SENSOR_IC_SIGNAL_FALSE;
        wl_sm_current_state                                     = WL_IDLE_STATE;

    }else if(signal_ctrl_system_fill_the_tank == SENSOR_IC_SIGNAL_TRUE)
    {
        SENSORS_INIT_SIGNALS();
        // activate the water in relay
        signal_ctrl_peripheral_turn_on_main_tank_water_input    = SENSOR_IC_SIGNAL_TRUE;
        signal_ctrl_peripheral_turn_off_main_tank_water_input   = SENSOR_IC_SIGNAL_FALSE;
        wl_sm_current_state                                     = WL_IDLE_STATE;
    }

    return;
}