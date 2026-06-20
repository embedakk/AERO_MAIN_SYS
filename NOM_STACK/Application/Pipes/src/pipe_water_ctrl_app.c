#include "../inc/pipe_water_ctrl_app.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"

int lastMin = -1;
bool pumpState = 0; // 0 = OFF, 1 = ON
int startMinute;
int elapsedMinutes;

//rtc
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;

void PIPE_WATER_CTRL_EXE_APPLICATION(void)
{
    if(signal_ctrl_pipe_water_enable_from_ph == SENSOR_IC_SIGNAL_TRUE && signal_ctrl_pipe_water_enable_from_ec == SENSOR_IC_SIGNAL_TRUE)
    {
        elapsedMinutes = (signal_data_get_rtc_data->minute - lastMin + 60) % 60;
        if (pumpState == 0 && elapsedMinutes >= PIPE_WATER_PUMP_OFF_TIME) {
            // Turn ON the pump
            pumpState = 1;
            lastMin = signal_data_get_rtc_data->minute;
        } 
        else if (pumpState == 1 && elapsedMinutes >= PIPE_WATER_PUMP_ON_TIME) {
            // Turn OFF the pump
            pumpState = 0;
            lastMin = signal_data_get_rtc_data->minute;
        }

        if(pumpState == true)
        {
            signal_ctrl_peripheral_turn_on_main_pump    = SENSOR_IC_SIGNAL_TRUE;
            signal_ctrl_peripheral_turn_off_main_pump   = SENSOR_IC_SIGNAL_FALSE;
            signal_data_set_text_pump_status[0]         = 'O';
            signal_data_set_text_pump_status[1]         = 'N';
            signal_data_set_text_pump_status[2]         = ' ';
        }else
        {
            signal_ctrl_peripheral_turn_on_main_pump    = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_peripheral_turn_off_main_pump   = SENSOR_IC_SIGNAL_TRUE;       
            signal_data_set_text_pump_status[0]         = 'O';
            signal_data_set_text_pump_status[1]         = 'F';
            signal_data_set_text_pump_status[2]         = 'F';
        }
    }
    return;
}



