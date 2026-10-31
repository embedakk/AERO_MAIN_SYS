#include "../inc/pipe_water_ctrl_app.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"
#include "../../NOM_STACK/Application/WL/inc/wl_application.h"
#include "../../NOM_STACK/Services/PUMP_IO_COM/inc/pump_io_com.h"

int lastMin = -1;
bool pumpState = false; // false = OFF, true = ON
int startMinute = 0;
int elapsedMinutes = 0;

// rtc
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;

void PIPE_WATER_RESET_STATE(void)
{
    pumpState = false;
    lastMin = -1;
    elapsedMinutes = 0;

    signal_ctrl_peripheral_turn_on_main_pump = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_off_main_pump = SENSOR_IC_SIGNAL_TRUE;
    memcpy(signal_data_set_text_pump_status, "OFF", 4);
}

void PIPE_WATER_CTRL_EXE_APPLICATION(void)
{
    if (!PUMP_IO_COM_is_link_healthy() ||
        signal_ctrl_water_level_start_ph_ec_unit != SENSOR_IC_SIGNAL_TRUE ||
        signal_ctrl_water_level_stop_ph_ec_unit == SENSOR_IC_SIGNAL_TRUE ||
        signal_ctrl_pipe_water_enable_from_ph != SENSOR_IC_SIGNAL_TRUE ||
        signal_ctrl_pipe_water_enable_from_ec != SENSOR_IC_SIGNAL_TRUE ||
        signal_data_get_rtc_data == NULL)
    {
        PIPE_WATER_RESET_STATE();
        return;
    }

    if (lastMin < 0) {
        elapsedMinutes = PIPE_WATER_PUMP_OFF_TIME;
    } else {
        elapsedMinutes = ((int)signal_data_get_rtc_data->minute - lastMin + 60) % 60;
    }

    if (!pumpState && elapsedMinutes >= PIPE_WATER_PUMP_OFF_TIME) {
        // Turn ON the pump
        pumpState = true;
        lastMin = (int)signal_data_get_rtc_data->minute;
    } else if (pumpState && elapsedMinutes >= PIPE_WATER_PUMP_ON_TIME) {
        // Turn OFF the pump
        pumpState = false;
        lastMin = (int)signal_data_get_rtc_data->minute;
    }

    // SAFETY CHECK 1: Prevent dry running or overflow circulation!
    // If water level drops to empty (>= WL_LOW_THRESHOLD_CM), force the pump OFF.
    if (signal_data_get_water_level_data >= WL_LOW_THRESHOLD_CM) {
        pumpState = false;
    }

    if (pumpState) {
        signal_ctrl_peripheral_turn_on_main_pump = SENSOR_IC_SIGNAL_TRUE;
        signal_ctrl_peripheral_turn_off_main_pump = SENSOR_IC_SIGNAL_FALSE;
        memcpy(signal_data_set_text_pump_status, "ON ", 4);

        // SAFETY CHECK 2: Prevent dilution/overflow!
        // If the main pump is actively watering plants, forcefully close the input valve
        // so we don't refill with fresh water and ruin the pH/EC balance of the current cycle.
        signal_ctrl_peripheral_turn_on_main_tank_water_input = SENSOR_IC_SIGNAL_FALSE;
        signal_ctrl_peripheral_turn_off_main_tank_water_input = SENSOR_IC_SIGNAL_TRUE;
    } else {
        signal_ctrl_peripheral_turn_on_main_pump = SENSOR_IC_SIGNAL_FALSE;
        signal_ctrl_peripheral_turn_off_main_pump = SENSOR_IC_SIGNAL_TRUE;
        memcpy(signal_data_set_text_pump_status, "OFF", 4);

        // SAFETY CHECK 3: Drain-back delay!
        // Keep the input valve closed for a short time after the pump turns off to allow
        // water in the pipes to drain back into the tank before deciding to refill.
        if (lastMin >= 0) {
            int minutesSincePumpTurnedOff = ((int)signal_data_get_rtc_data->minute - lastMin + 60) % 60;
            if (minutesSincePumpTurnedOff < PIPE_WATER_RETURN_DELAY_TIME) {
                signal_ctrl_peripheral_turn_on_main_tank_water_input = SENSOR_IC_SIGNAL_FALSE;
                signal_ctrl_peripheral_turn_off_main_tank_water_input = SENSOR_IC_SIGNAL_TRUE;
            }
        }
    }
}
