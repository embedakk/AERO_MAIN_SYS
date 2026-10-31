#include "../inc/wl_application.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"
#include "../../NOM_STACK/Services/PUMP_IO_COM/inc/pump_io_com.h"

wl_state_machine wl_sm_current_state = WL_IDLE_STATE;
float total_volume = 0.0f; 
float filled_volume = 0.0f;
float calculated_value = 0.0f;

static void wl_force_all_actuators_safe_off(void)
{
    signal_ctrl_peripheral_turn_on_main_tank_water_output   = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_off_main_tank_water_output  = SENSOR_IC_SIGNAL_TRUE;

    signal_ctrl_peripheral_turn_on_main_tank_water_input    = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_off_main_tank_water_input   = SENSOR_IC_SIGNAL_TRUE;

    signal_ctrl_peripheral_turn_on_main_pump                = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_off_main_pump               = SENSOR_IC_SIGNAL_TRUE;

    signal_ctrl_led_turn_on_led                             = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_circulation_pumps_turn_on                   = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_mixer_motors_turn_on                        = SENSOR_IC_SIGNAL_FALSE;

    signal_ctrl_peripheral_turn_on_ph_pump                  = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_on_ec_pump                  = SENSOR_IC_SIGNAL_FALSE;

    signal_ctrl_water_level_start_ph_ec_unit                = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_water_level_stop_ph_ec_unit                 = SENSOR_IC_SIGNAL_TRUE;
    signal_ctrl_ph_activate_get_ph                          = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_ec_activate_get_ec                          = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_ph_disable_wl_unit                          = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_ec_disable_wl_unit                          = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_pipe_water_enable_from_ph                   = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_pipe_water_enable_from_ec                   = SENSOR_IC_SIGNAL_FALSE;
}

void WL_RESET_STATE(void)
{
    wl_force_all_actuators_safe_off();
    wl_sm_current_state = WL_IDLE_STATE;
}

// Function to calculate volume percentage (filled_height is ultrasonic distance from tank top in cm)
float calculate_percentage(float length, float width, float height, float filled_height) 
{
    if (!isfinite(length) || !isfinite(width) || !isfinite(height) || !isfinite(filled_height) ||
        length <= 0.0f || width <= 0.0f || height <= 0.0f)
    {
        calculated_value = 100.0f;
        return 0.0f;
    }

    if (filled_height <= 0.0f)
    {
        calculated_value = 0.0f;
        return 100.0f;
    }

    if (filled_height >= height)
    {
        calculated_value = 100.0f;
        return 0.0f;
    }

    total_volume = length * width * (height - filled_height);
    filled_volume = length * width * filled_height;
    if (total_volume <= 0.0f)
    {
        calculated_value = 100.0f;
        return 0.0f;
    }

    calculated_value = (filled_volume / total_volume) * 100.0f;
    if (calculated_value > 100.0f)
    {
        calculated_value = 100.0f;
    }
    else if (calculated_value < 0.0f)
    {
        calculated_value = 0.0f;
    }
    return (100.0f - calculated_value);
}

float WL_GET_PERCENTAGE(void)
{
    return calculate_percentage(120.0f, 250.0f, 120.0f, signal_data_get_water_level_data);
}

void WL_EXE_APPLICATION(void)
{
    // 1. One-shot DELETE command: reset signals/NVS to safe defaults and shut down all actuators
    if (signal_ctrl_system_delete == SENSOR_IC_SIGNAL_TRUE)
    {
        SENSORS_RESET_TO_DEFAULTS();
        wl_force_all_actuators_safe_off();
        wl_sm_current_state = WL_IDLE_STATE;
        return;
    }

    // 2. Explicit STOP command: force every actuator and dosing signal OFF
    if (signal_ctrl_system_stop == SENSOR_IC_SIGNAL_TRUE)
    {
        wl_force_all_actuators_safe_off();
        wl_sm_current_state = WL_IDLE_STATE;
        return;
    }

    // 3. STM32 IO board link health gate:
    //    - Keep all actuators Safe-Off while link/sensor data is missing or stale.
    //    - Cancel manual fill/drain if link drops so valves never reopen unattended.
    //    - Preserve signal_ctrl_system_start / continue so automatic mode auto-resumes
    //      once valid STM32 sensor frames arrive.
    if (!PUMP_IO_COM_is_link_healthy())
    {
        if (signal_ctrl_system_fill_the_tank == SENSOR_IC_SIGNAL_TRUE ||
            signal_ctrl_system_release_the_tank == SENSOR_IC_SIGNAL_TRUE)
        {
            signal_ctrl_system_fill_the_tank = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_release_the_tank = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_stop = SENSOR_IC_SIGNAL_TRUE;
            signal_data_get_system_start_stop_status = 0;
        }
        wl_force_all_actuators_safe_off();
        wl_sm_current_state = WL_IDLE_STATE;
        return;
    }

    // 4. Manual RELEASE_TANK: drain until empty threshold (>= WL_LOW_THRESHOLD_CM), then auto-stop
    if (signal_ctrl_system_release_the_tank == SENSOR_IC_SIGNAL_TRUE)
    {
        wl_force_all_actuators_safe_off();
        wl_sm_current_state = WL_IDLE_STATE;

        if (signal_data_get_water_level_data >= WL_LOW_THRESHOLD_CM)
        {
            signal_ctrl_system_release_the_tank = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_stop = SENSOR_IC_SIGNAL_TRUE;
            signal_data_get_system_start_stop_status = 0;
        }
        else
        {
            signal_ctrl_peripheral_turn_on_main_tank_water_output  = SENSOR_IC_SIGNAL_TRUE;
            signal_ctrl_peripheral_turn_off_main_tank_water_output = SENSOR_IC_SIGNAL_FALSE;
        }
        return;
    }

    // 5. Manual FILL_TANK: fill until high threshold (<= WL_HIGH_THRESHOLD_CM), then auto-stop
    if (signal_ctrl_system_fill_the_tank == SENSOR_IC_SIGNAL_TRUE)
    {
        wl_force_all_actuators_safe_off();
        wl_sm_current_state = WL_IDLE_STATE;

        if (signal_data_get_water_level_data <= WL_HIGH_THRESHOLD_CM)
        {
            signal_ctrl_system_fill_the_tank = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_stop = SENSOR_IC_SIGNAL_TRUE;
            signal_data_get_system_start_stop_status = 0;
        }
        else
        {
            signal_ctrl_peripheral_turn_on_main_tank_water_input  = SENSOR_IC_SIGNAL_TRUE;
            signal_ctrl_peripheral_turn_off_main_tank_water_input = SENSOR_IC_SIGNAL_FALSE;
        }
        return;
    }

    // 6. Automatic START / CONTINUE mode (requires a valid crop 0..16)
    if (signal_ctrl_system_start == SENSOR_IC_SIGNAL_TRUE || signal_ctrl_system_continue == SENSOR_IC_SIGNAL_TRUE)
    {
        if (signal_data_set_selected_fruit >= 17U)
        {
            wl_force_all_actuators_safe_off();
            wl_sm_current_state = WL_IDLE_STATE;
            return;
        }

        if (signal_ctrl_system_continue == SENSOR_IC_SIGNAL_TRUE)
        {
            signal_ctrl_system_start = SENSOR_IC_SIGNAL_TRUE;
        }

        // Check for EC and pH disable signals (active dilution control)
        if (signal_ctrl_ph_disable_wl_unit == SENSOR_IC_SIGNAL_TRUE || signal_ctrl_ec_disable_wl_unit == SENSOR_IC_SIGNAL_TRUE)
        {
            signal_ctrl_circulation_pumps_turn_on = SENSOR_IC_SIGNAL_TRUE;
        }
        else
        {
            switch (wl_sm_current_state)
            {
                case WL_IDLE_STATE:
                {
                    if (signal_data_get_water_level_data >= WL_LOW_THRESHOLD_CM) // tank is empty
                    {
                        signal_ctrl_water_level_start_ph_ec_unit = SENSOR_IC_SIGNAL_FALSE;
                        signal_ctrl_water_level_stop_ph_ec_unit  = SENSOR_IC_SIGNAL_TRUE;
                        signal_ctrl_ph_activate_get_ph           = SENSOR_IC_SIGNAL_FALSE;
                        signal_ctrl_ec_activate_get_ec           = SENSOR_IC_SIGNAL_FALSE;
                        wl_sm_current_state = WL_FILL_THE_TANK;
                    }
                    else if (signal_data_get_water_level_data <= WL_CRITICAL_THRESHOLD_CM) // critical overflow range
                    {
                        signal_ctrl_water_level_start_ph_ec_unit = SENSOR_IC_SIGNAL_FALSE;
                        signal_ctrl_water_level_stop_ph_ec_unit  = SENSOR_IC_SIGNAL_TRUE;
                        signal_ctrl_ph_activate_get_ph           = SENSOR_IC_SIGNAL_FALSE;
                        signal_ctrl_ec_activate_get_ec           = SENSOR_IC_SIGNAL_FALSE;
                        wl_sm_current_state = WL_DISCHARGE_THE_TANK;
                    }
                    else if (signal_data_get_water_level_data > WL_CRITICAL_THRESHOLD_CM && signal_data_get_water_level_data <= WL_HIGH_THRESHOLD_CM) // tank is full
                    {
                        signal_ctrl_ph_activate_get_ph = SENSOR_IC_SIGNAL_TRUE;
                        signal_ctrl_ec_activate_get_ec = SENSOR_IC_SIGNAL_TRUE;
                        wl_sm_current_state = WL_TANK_IS_FULL;
                    }
                    else // WL_HIGH_THRESHOLD_CM < level < WL_LOW_THRESHOLD_CM
                    {
                        signal_ctrl_ph_activate_get_ph = SENSOR_IC_SIGNAL_TRUE;
                        signal_ctrl_ec_activate_get_ec = SENSOR_IC_SIGNAL_TRUE;
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
                
                    signal_ctrl_peripheral_turn_on_main_tank_water_output   = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_output  = SENSOR_IC_SIGNAL_TRUE;

                    signal_ctrl_circulation_pumps_turn_on                   = SENSOR_IC_SIGNAL_TRUE;
                    wl_sm_current_state = WL_IDLE_STATE;
                }
                break;

                case WL_TANK_IS_READY_FOR_DOSING:
                {
                    signal_ctrl_water_level_start_ph_ec_unit                = SENSOR_IC_SIGNAL_TRUE;
                    signal_ctrl_water_level_stop_ph_ec_unit                 = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_circulation_pumps_turn_on                   = SENSOR_IC_SIGNAL_TRUE;
                    wl_sm_current_state = WL_FILL_THE_TANK;
                }
                break;

                case WL_FILL_THE_TANK:
                {
                    signal_ctrl_peripheral_turn_on_main_tank_water_input    = SENSOR_IC_SIGNAL_TRUE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_input   = SENSOR_IC_SIGNAL_FALSE;
                
                    signal_ctrl_peripheral_turn_on_main_tank_water_output   = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_output  = SENSOR_IC_SIGNAL_TRUE;
                    signal_ctrl_circulation_pumps_turn_on                   = SENSOR_IC_SIGNAL_TRUE;
                    wl_sm_current_state = WL_IDLE_STATE;
                }
                break;

                case WL_DISCHARGE_THE_TANK:
                {
                    signal_ctrl_peripheral_turn_on_main_tank_water_output   = SENSOR_IC_SIGNAL_TRUE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_output  = SENSOR_IC_SIGNAL_FALSE;
                
                    signal_ctrl_peripheral_turn_on_main_tank_water_input    = SENSOR_IC_SIGNAL_FALSE;
                    signal_ctrl_peripheral_turn_off_main_tank_water_input   = SENSOR_IC_SIGNAL_TRUE;
                    wl_sm_current_state = WL_IDLE_STATE;
                }
                break;

                case WL_CONTROL_DUTY_TO_ANOTHER_UNIT:
                default:
                {
                    wl_sm_current_state = WL_IDLE_STATE;
                }
                break;
            }
        }
        return;
    }

    // Idle default when no command flag is active
    wl_force_all_actuators_safe_off();
    wl_sm_current_state = WL_IDLE_STATE;
}
