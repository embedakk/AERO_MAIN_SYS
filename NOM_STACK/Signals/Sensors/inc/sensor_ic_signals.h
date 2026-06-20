#ifndef SENSOR_IC_SIGNALS_H
#define SENSOR_IC_SIGNALS_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"

#include "../../NOM_STACK/Sensors/RealTimeClock/inc/real_time_clock.h"


#define         SENSOR_IC_SIGNALS_TAG         "SENSOR_IC_SIGNALS"

#define         SENSOR_IC_SIGNALS_LOGS
#undef          SENSOR_IC_SIGNALS_LOGS

typedef enum sensor_ic_signal_status
{
    SENSOR_IC_SIGNAL_TRUE = true,
    SENSOR_IC_SIGNAL_FALSE = false
}sensor_ic_signal_status;


typedef struct {
    uint8_t  system_start_stop_status;      // 0
    uint8_t  system_stage_data_set;         // 0
    uint8_t  system_stage_data_get;         // 0
    uint16_t selected_fruit;                // 18
    float    ph_cal_low_V;                  // 3.16f
    float    ph_cal_mid_V;                  // 3.01f
    float    ph_cal_high_V;                 // 2.58f
    float    ec_cal_mid_V;                  // 0.0f
    uint16_t tank_capacity;                 // 100
    uint16_t dosing_intensity;              // 50
    uint32_t ph_concentration_data;         // 10
    uint32_t ec_concentration_data;         // 10
    uint16_t mixing_time;                   // 300
} signal_data_t;

/*
    data signals
*/
//LED
uint8_t signal_data_get_turn_on_time;

//water level
float signal_data_get_water_level_data;

float signal_data_get_water_cap_percentage;

//water temperature
float signal_data_get_water_temperature_data;

//ph
float signal_data_get_ph_data;
float signal_data_get_target_ph_data;
// float signal_data_get_ph_concentration_data;
uint32_t signal_data_get_ph_concentration_data;
float signal_data_get_ph_water_volume_data;
float signal_data_set_ph_difference_data;
float signal_data_set_ph_req_volume;
float signal_data_set_ph_voltage;

float signal_data_set_ph_cal_low_V;
float signal_data_set_ph_cal_mid_V;
float signal_data_set_ph_cal_high_V;

//ec
float signal_data_get_ec_data;
float signal_data_get_target_ec_data;
// float signal_data_get_ec_concentration_data;
uint32_t signal_data_get_ec_concentration_data;
float signal_data_get_ec_water_volume_data;
float signal_data_set_ec_difference_data;
float signal_data_set_ec_req_volume;
float signal_data_set_ec_voltage;

float signal_data_set_ec_cal_low_V;
float signal_data_set_ec_cal_mid_V;
float signal_data_set_ec_cal_high_V;

//room temperature
float signal_data_get_room_temperature_data;

//room humidity
float signal_data_get_room_humidity_data;

//selected fruit
uint16_t signal_data_set_selected_fruit; // this will decide for targeted ec, ph , led, and temperature of the room and water 
//signal_data_get_turn_on_time
//signal_data_get_target_ph_data
//signal_data_get_target_ec_data

char     signal_data_set_text_selected_fruit[15];

//current led status on the pipes
char     signal_data_set_text_led_status[3];

//current pump status 
char     signal_data_set_text_pump_status[3];

//current sys status 
char     signal_data_set_text_sys_status[3];

//set the tank capacity from display
uint16_t signal_data_get_tank_capacity;

//set the dosing intensity from display
uint16_t signal_data_get_dosing_intensity;

//set the mixing time from display
uint16_t signal_data_get_mixing_time;

//stage of the system set get data
uint8_t signal_data_get_system_stage_data;
uint8_t signal_data_set_system_stage_data;

//rtc
DS1302_DateTime ref_get_rtc_data;
DS1302_DateTime ref_set_rtc_data;
// //rtc
// DS1302_DateTime *signal_data_get_rtc_data;
// DS1302_DateTime *signal_data_set_rtc_data;
// if ((dateTime->second > 59) ||
// (dateTime->minute > 59) ||
// (dateTime->hour > 23) ||
// (dateTime->dayMonth < 1) || (dateTime->dayMonth > 31) ||
// (dateTime->month < 1) || (dateTime->month > 12) ||
// (dateTime->dayWeek < 1) || (dateTime->dayWeek > 7) ||
// (dateTime->year > 2099))

// pump io com signals
uint8_t pump_io_get_ec_duty_V_byte_data[4];
uint8_t pump_io_get_ph_duty_V_byte_data[4];
uint8_t pump_io_get_temperature_byte_data[4];
uint8_t pump_io_get_distance_data;

float pump_io_get_ec_duty_V_float_data;
float pump_io_get_ph_duty_V_float_data;
float pump_io_get_temperature_float_data;

/*
    control signals
*/
//system 
//create signals for start, stop, continue, delete, release the tank, fill the tank
uint8_t  signal_data_get_system_start_stop_status; // 0 - stop, 1 - start, 2 - continue, 3 - delete
sensor_ic_signal_status signal_ctrl_system_start;
sensor_ic_signal_status signal_ctrl_system_stop;
sensor_ic_signal_status signal_ctrl_system_continue;
sensor_ic_signal_status signal_ctrl_system_delete;
sensor_ic_signal_status signal_ctrl_system_release_the_tank;
sensor_ic_signal_status signal_ctrl_system_fill_the_tank;

sensor_ic_signal_status signal_ctrl_system_cal_ph_low;
sensor_ic_signal_status signal_ctrl_system_cal_ph_mid;
sensor_ic_signal_status signal_ctrl_system_cal_ph_high;

sensor_ic_signal_status signal_ctrl_system_cal_ec_low;
sensor_ic_signal_status signal_ctrl_system_cal_ec_mid;
sensor_ic_signal_status signal_ctrl_system_cal_ec_high;


//water level
sensor_ic_signal_status signal_ctrl_water_level_activate_get_wl;
sensor_ic_signal_status signal_ctrl_water_level_fill_the_tank;
sensor_ic_signal_status signal_ctrl_water_level_start_ph_ec_unit;
sensor_ic_signal_status signal_ctrl_water_level_stop_ph_ec_unit;
sensor_ic_signal_status signal_ctrl_water_level_water_in_out_duty_to_another_unit;

//peripherals
sensor_ic_signal_status signal_ctrl_peripheral_turn_on_main_tank_water_input;
sensor_ic_signal_status signal_ctrl_peripheral_turn_off_main_tank_water_input;
sensor_ic_signal_status signal_ctrl_peripheral_turn_on_main_tank_water_output;
sensor_ic_signal_status signal_ctrl_peripheral_turn_off_main_tank_water_output;

sensor_ic_signal_status signal_ctrl_peripheral_turn_on_main_pump;
sensor_ic_signal_status signal_ctrl_peripheral_turn_off_main_pump;
sensor_ic_signal_status signal_ctrl_peripheral_turn_on_ph_pump;
sensor_ic_signal_status signal_ctrl_peripheral_turn_on_ec_pump;

//water temperature
sensor_ic_signal_status signal_ctrl_water_temperature_activate_get_wtemp;

//ph
sensor_ic_signal_status signal_ctrl_ph_activate_get_ph;
sensor_ic_signal_status signal_ctrl_ph_disable_wl_unit;
sensor_ic_signal_status signal_ctrl_pipe_water_enable_from_ph;

//ec
sensor_ic_signal_status signal_ctrl_ec_activate_get_ec;
sensor_ic_signal_status signal_ctrl_ec_disable_wl_unit;
sensor_ic_signal_status signal_ctrl_pipe_water_enable_from_ec;

//rtc
sensor_ic_signal_status signal_ctrl_rtc_activate_set_rtc;
sensor_ic_signal_status signal_ctrl_rtc_activate_get_rtc;

//leds
sensor_ic_signal_status signal_ctrl_led_turn_on_led;

//circulation pumps
sensor_ic_signal_status signal_ctrl_circulation_pumps_turn_on;

//mixer motors
sensor_ic_signal_status signal_ctrl_mixer_motors_turn_on;

signal_data_t nvs_signal_data;

void init_nvs();
void save_signal_data(signal_data_t *data);
bool load_signal_data(signal_data_t *data);

void SENSORS_INIT_SIGNALS(void);

#endif //SENSOR_IC_SIGNALS_H