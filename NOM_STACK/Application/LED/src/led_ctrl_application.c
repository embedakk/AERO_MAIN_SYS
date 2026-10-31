#include "../inc/led_ctrl_application.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"


// typedef struct {
//     uint8_t second;     //!< Second 0..59
//     uint8_t minute;     //!< Minute 0..59
//     uint8_t hour;       //!< Hour 0..23
//     uint8_t dayWeek;    //!< Day of the week (0 as Sunday/ 1 as Monday)
//     uint8_t dayMonth;   //!< Day of the month 1..31
//     uint8_t month;      //!< Month 1..12
//     uint16_t year;      //!< Year 2000..2099
// } DS1302_DateTime;

//rtc
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;




DS1302_DateTime ledStart; 
DS1302_DateTime ledOff;

// Function to calculate LED turn-off time
DS1302_DateTime calculateLEDTurnOffTime(DS1302_DateTime start, int durationHours) 
{
    DS1302_DateTime offTime;
    // Add hours, minutes, and seconds while handling rollover
    offTime.hour = (start.hour + durationHours) % 24;
    offTime.minute = start.minute;
    offTime.second = start.second;

    return offTime;
}

void LED_CTRL_EXE_APPLICATION(void)
{
    if(signal_ctrl_system_start == SENSOR_IC_SIGNAL_TRUE || signal_ctrl_system_continue == SENSOR_IC_SIGNAL_TRUE)
    {
        signal_data_set_text_sys_status[0] = 'O';
        signal_data_set_text_sys_status[1] = 'N';
        signal_data_set_text_sys_status[2] = ' ';
        ledStart.hour = 22;
        ledStart.minute = 1;
        ledStart.second = 1;
        ledOff = calculateLEDTurnOffTime(ledStart, (int)signal_data_get_turn_on_time);
    
        // Check if LED should turn off
        if (signal_data_get_rtc_data->hour == ledOff.hour && signal_data_get_rtc_data->minute == ledOff.minute &&
            signal_data_get_rtc_data->second == ledOff.second) 
        {
            //keep turned off leds
            signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_FALSE;
            signal_data_set_text_led_status[0] = 'O';
            signal_data_set_text_led_status[1] = 'F';
            signal_data_set_text_led_status[2] = 'F';
        }else
        {
            signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_TRUE;
            signal_data_set_text_led_status[0] = 'O';
            signal_data_set_text_led_status[1] = 'N';
            signal_data_set_text_led_status[2] = ' ';
        }
    }else
    {
        signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_FALSE;
        signal_data_set_text_sys_status[0] = 'O';
        signal_data_set_text_sys_status[1] = 'F';
        signal_data_set_text_sys_status[2] = 'F';
    }
    return;
}