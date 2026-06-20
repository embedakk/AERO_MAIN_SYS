#include "../inc/pipe_water_ctrl_app.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"
#include "../../NOM_STACK/Application/WL/inc/wl_application.h"


int lastMin = -1;
bool pumpState = 0; // 0 = OFF, 1 = ON
int startMinute;
int elapsedMinutes;

// rtc
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;

void PIPE_WATER_CTRL_EXE_APPLICATION(void) {
  if (signal_ctrl_water_level_start_ph_ec_unit == SENSOR_IC_SIGNAL_TRUE && signal_ctrl_pipe_water_enable_from_ph == SENSOR_IC_SIGNAL_TRUE && signal_ctrl_pipe_water_enable_from_ec == SENSOR_IC_SIGNAL_TRUE) 
  {
    elapsedMinutes = (signal_data_get_rtc_data->minute - lastMin + 60) % 60;
    if (pumpState == 0 && elapsedMinutes >= PIPE_WATER_PUMP_OFF_TIME) {
      // Turn ON the pump
      pumpState = 1;
      lastMin = signal_data_get_rtc_data->minute;
    } else if (pumpState == 1 && elapsedMinutes >= PIPE_WATER_PUMP_ON_TIME) {
      // Turn OFF the pump
      pumpState = 0;
      lastMin = signal_data_get_rtc_data->minute;
    }

    // SAFETY CHECK 1: Prevent dry running! 
    // If water level drops to empty (>= WL_LOW_THRESHOLD_CM), force the pump OFF.
    if (signal_data_get_water_level_data >= WL_LOW_THRESHOLD_CM) {
        pumpState = 0;
    }

    if (pumpState == true) {
      signal_ctrl_peripheral_turn_on_main_pump = SENSOR_IC_SIGNAL_TRUE;
      signal_ctrl_peripheral_turn_off_main_pump = SENSOR_IC_SIGNAL_FALSE;
      signal_data_set_text_pump_status[0] = 'O';
      signal_data_set_text_pump_status[1] = 'N';
      signal_data_set_text_pump_status[2] = ' ';

      // SAFETY CHECK 2: Prevent dilution/overflow!
      // If the main pump is actively watering plants, forcefully close the input valve
      // so we don't refill with fresh water and ruin the pH/EC balance of the current cycle.
      signal_ctrl_peripheral_turn_on_main_tank_water_input = SENSOR_IC_SIGNAL_FALSE;
      signal_ctrl_peripheral_turn_off_main_tank_water_input = SENSOR_IC_SIGNAL_TRUE;
    } else {
      signal_ctrl_peripheral_turn_on_main_pump = SENSOR_IC_SIGNAL_FALSE;
      signal_ctrl_peripheral_turn_off_main_pump = SENSOR_IC_SIGNAL_TRUE;
      signal_data_set_text_pump_status[0] = 'O';
      signal_data_set_text_pump_status[1] = 'F';
      signal_data_set_text_pump_status[2] = 'F';

      // SAFETY CHECK 3: Drain-back delay!
      // Keep the input valve closed for a short time after the pump turns off to allow
      // water in the pipes to drain back into the tank before deciding to refill.
      int minutesSincePumpTurnedOff = (signal_data_get_rtc_data->minute - lastMin + 60) % 60;
      if (minutesSincePumpTurnedOff < PIPE_WATER_RETURN_DELAY_TIME) {
          signal_ctrl_peripheral_turn_on_main_tank_water_input = SENSOR_IC_SIGNAL_FALSE;
          signal_ctrl_peripheral_turn_off_main_tank_water_input = SENSOR_IC_SIGNAL_TRUE;
      }
    }
  } else {
    signal_ctrl_peripheral_turn_on_main_pump = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_off_main_pump = SENSOR_IC_SIGNAL_TRUE;
    signal_data_set_text_pump_status[0] = 'O';
    signal_data_set_text_pump_status[1] = 'F';
    signal_data_set_text_pump_status[2] = 'F';
  }
  return;
}
