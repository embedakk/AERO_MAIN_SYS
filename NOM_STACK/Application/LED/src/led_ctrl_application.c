#include "../inc/led_ctrl_application.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"
#include "../../NOM_STACK/Services/PUMP_IO_COM/inc/pump_io_com.h"

//rtc
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;

DS1302_DateTime ledStart; 
DS1302_DateTime ledOff;

// Function to calculate LED turn-off time
DS1302_DateTime calculateLEDTurnOffTime(DS1302_DateTime start, int durationHours) 
{
    DS1302_DateTime offTime = {0};
    if (durationHours < 0) {
        durationHours = 0;
    }
    offTime.hour   = (uint8_t)(((int)start.hour + durationHours) % 24);
    offTime.minute = start.minute;
    offTime.second = start.second;

    return offTime;
}

void LED_CTRL_EXE_APPLICATION(void)
{
    bool sys_requested = (signal_ctrl_system_start == SENSOR_IC_SIGNAL_TRUE ||
                          signal_ctrl_system_continue == SENSOR_IC_SIGNAL_TRUE) &&
                         (signal_data_set_selected_fruit < 17U);

    if (!sys_requested)
    {
        signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_FALSE;
        memcpy(signal_data_set_text_led_status, "OFF", 4);
        memcpy(signal_data_set_text_sys_status, "OFF", 4);
        return;
    }

    memcpy(signal_data_set_text_sys_status, "ON ", 4);

    if (!PUMP_IO_COM_is_link_healthy() || signal_data_get_rtc_data == NULL)
    {
        signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_FALSE;
        memcpy(signal_data_set_text_led_status, "OFF", 4);
        return;
    }

    ledStart.hour = 22;
    ledStart.minute = 1;
    ledStart.second = 1;

    int durationHours = (int)signal_data_get_turn_on_time;
    if (durationHours <= 0) {
        signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_FALSE;
        memcpy(signal_data_set_text_led_status, "OFF", 4);
        return;
    }
    if (durationHours >= 24) {
        signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_TRUE;
        memcpy(signal_data_set_text_led_status, "ON ", 4);
        return;
    }

    ledOff = calculateLEDTurnOffTime(ledStart, durationHours);

    int startSec = ((int)ledStart.hour * 3600) + ((int)ledStart.minute * 60) + (int)ledStart.second;
    int nowSec   = ((int)signal_data_get_rtc_data->hour * 3600) +
                   ((int)signal_data_get_rtc_data->minute * 60) +
                   (int)signal_data_get_rtc_data->second;
    int elapsedSec = (nowSec - startSec + 86400) % 86400;

    if (elapsedSec < (durationHours * 3600))
    {
        signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_TRUE;
        memcpy(signal_data_set_text_led_status, "ON ", 4);
    }
    else
    {
        signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_FALSE;
        memcpy(signal_data_set_text_led_status, "OFF", 4);
    }
}
