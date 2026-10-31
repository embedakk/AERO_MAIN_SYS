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

#ifdef __cplusplus
extern "C" {
#endif

#define         SENSOR_IC_SIGNALS_TAG                   "SENSOR_IC_SIGNALS"

#define         SENSOR_IC_SIGNALS_LOGS
#undef          SENSOR_IC_SIGNALS_LOGS

/* Safe default stock concentrations matching XilApp & DWIN units */
#define         DEFAULT_TANK_CAPACITY_L                 100U
#define         DEFAULT_DOSING_INTENSITY_ML_MIN         50U
#define         DEFAULT_PH_CONCENTRATION_DATA           1470UL
#define         DEFAULT_EC_CONCENTRATION_DATA           90000UL
#define         DEFAULT_MIXING_TIME_S                   300U
#define         NVS_MIN_SAVE_INTERVAL_US                2000000LL

typedef enum sensor_ic_signal_status
{
    SENSOR_IC_SIGNAL_TRUE = true,
    SENSOR_IC_SIGNAL_FALSE = false
} sensor_ic_signal_status;

typedef struct {
    uint8_t  system_start_stop_status;      // 0: stop, 1: start, 2: continue, 3: delete
    uint8_t  system_stage_data_set;         // 0..2
    uint8_t  system_stage_data_get;         // 0..2
    uint16_t selected_fruit;                // 0..16 valid, 17 wrong, 18 no selection
    float    ph_cal_low_V;                  // 3.16f
    float    ph_cal_mid_V;                  // 3.01f
    float    ph_cal_high_V;                 // 2.58f
    float    ec_cal_mid_V;                  // 0.0f
    uint16_t tank_capacity;                 // 100
    uint16_t dosing_intensity;              // 50
    uint32_t ph_concentration_data;         // 1470
    uint32_t ec_concentration_data;         // 90000
    uint16_t mixing_time;                   // 300
} signal_data_t;

/*
    data signals
*/
// LED
extern uint8_t signal_data_get_turn_on_time;

// water level
extern float signal_data_get_water_level_data;
extern float signal_data_get_water_cap_percentage;

// water temperature
extern float signal_data_get_water_temperature_data;

// ph
extern float signal_data_get_ph_data;
extern float signal_data_get_target_ph_data;
extern uint32_t signal_data_get_ph_concentration_data;
extern float signal_data_get_ph_water_volume_data;
extern float signal_data_set_ph_difference_data;
extern float signal_data_set_ph_req_volume;
extern float signal_data_set_ph_voltage;

extern float signal_data_set_ph_cal_low_V;
extern float signal_data_set_ph_cal_mid_V;
extern float signal_data_set_ph_cal_high_V;

// ec
extern float signal_data_get_ec_data;
extern float signal_data_get_target_ec_data;
extern uint32_t signal_data_get_ec_concentration_data;
extern float signal_data_get_ec_water_volume_data;
extern float signal_data_set_ec_difference_data;
extern float signal_data_set_ec_req_volume;
extern float signal_data_set_ec_voltage;

extern float signal_data_set_ec_cal_low_V;
extern float signal_data_set_ec_cal_mid_V;
extern float signal_data_set_ec_cal_high_V;

// room temperature & humidity
extern float signal_data_get_room_temperature_data;
extern float signal_data_get_room_humidity_data;

// selected fruit (decides targeted ec, ph, led, room/water temp)
extern uint16_t signal_data_set_selected_fruit;

// null-terminated text buffers (15 visible chars + '\0' / 3 visible chars + '\0')
extern char signal_data_set_text_selected_fruit[16];
extern char signal_data_set_text_led_status[4];
extern char signal_data_set_text_pump_status[4];
extern char signal_data_set_text_sys_status[4];

// set tank capacity, dosing intensity, mixing time from display
extern uint16_t signal_data_get_tank_capacity;
extern uint16_t signal_data_get_dosing_intensity;
extern uint16_t signal_data_get_mixing_time;

// stage of the system set/get data
extern uint8_t signal_data_get_system_stage_data;
extern uint8_t signal_data_set_system_stage_data;

// rtc
extern DS1302_DateTime ref_get_rtc_data;
extern DS1302_DateTime ref_set_rtc_data;
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;

// pump io com signals
extern uint8_t pump_io_get_ec_duty_V_byte_data[4];
extern uint8_t pump_io_get_ph_duty_V_byte_data[4];
extern uint8_t pump_io_get_temperature_byte_data[4];
extern uint8_t pump_io_get_distance_data;

extern float pump_io_get_ec_duty_V_float_data;
extern float pump_io_get_ph_duty_V_float_data;
extern float pump_io_get_temperature_float_data;

/*
    control signals
*/
// system: start, stop, continue, delete, release the tank, fill the tank
extern uint8_t signal_data_get_system_start_stop_status; // 0 - stop, 1 - start, 2 - continue, 3 - delete
extern sensor_ic_signal_status signal_ctrl_system_start;
extern sensor_ic_signal_status signal_ctrl_system_stop;
extern sensor_ic_signal_status signal_ctrl_system_continue;
extern sensor_ic_signal_status signal_ctrl_system_delete;
extern sensor_ic_signal_status signal_ctrl_system_release_the_tank;
extern sensor_ic_signal_status signal_ctrl_system_fill_the_tank;

extern sensor_ic_signal_status signal_ctrl_system_cal_ph_low;
extern sensor_ic_signal_status signal_ctrl_system_cal_ph_mid;
extern sensor_ic_signal_status signal_ctrl_system_cal_ph_high;

extern sensor_ic_signal_status signal_ctrl_system_cal_ec_low;
extern sensor_ic_signal_status signal_ctrl_system_cal_ec_mid;
extern sensor_ic_signal_status signal_ctrl_system_cal_ec_high;

// water level
extern sensor_ic_signal_status signal_ctrl_water_level_activate_get_wl;
extern sensor_ic_signal_status signal_ctrl_water_level_fill_the_tank;
extern sensor_ic_signal_status signal_ctrl_water_level_start_ph_ec_unit;
extern sensor_ic_signal_status signal_ctrl_water_level_stop_ph_ec_unit;
extern sensor_ic_signal_status signal_ctrl_water_level_water_in_out_duty_to_another_unit;

// peripherals
extern sensor_ic_signal_status signal_ctrl_peripheral_turn_on_main_tank_water_input;
extern sensor_ic_signal_status signal_ctrl_peripheral_turn_off_main_tank_water_input;
extern sensor_ic_signal_status signal_ctrl_peripheral_turn_on_main_tank_water_output;
extern sensor_ic_signal_status signal_ctrl_peripheral_turn_off_main_tank_water_output;

extern sensor_ic_signal_status signal_ctrl_peripheral_turn_on_main_pump;
extern sensor_ic_signal_status signal_ctrl_peripheral_turn_off_main_pump;
extern sensor_ic_signal_status signal_ctrl_peripheral_turn_on_ph_pump;
extern sensor_ic_signal_status signal_ctrl_peripheral_turn_on_ec_pump;

// water temperature
extern sensor_ic_signal_status signal_ctrl_water_temperature_activate_get_wtemp;

// ph
extern sensor_ic_signal_status signal_ctrl_ph_activate_get_ph;
extern sensor_ic_signal_status signal_ctrl_ph_disable_wl_unit;
extern sensor_ic_signal_status signal_ctrl_pipe_water_enable_from_ph;

// ec
extern sensor_ic_signal_status signal_ctrl_ec_activate_get_ec;
extern sensor_ic_signal_status signal_ctrl_ec_disable_wl_unit;
extern sensor_ic_signal_status signal_ctrl_pipe_water_enable_from_ec;

// rtc
extern sensor_ic_signal_status signal_ctrl_rtc_activate_set_rtc;
extern sensor_ic_signal_status signal_ctrl_rtc_activate_get_rtc;

// leds
extern sensor_ic_signal_status signal_ctrl_led_turn_on_led;

// circulation pumps
extern sensor_ic_signal_status signal_ctrl_circulation_pumps_turn_on;

// mixer motors
extern sensor_ic_signal_status signal_ctrl_mixer_motors_turn_on;

extern signal_data_t nvs_signal_data;

void init_nvs(void);
void save_signal_data(signal_data_t *data);
bool save_signal_data_if_changed(signal_data_t *data);
bool load_signal_data(signal_data_t *data);

void SIGNALS_init_sync(void);
void SIGNALS_lock(void);
void SIGNALS_unlock(void);
void SIGNALS_set_selected_fruit(uint16_t fruit_id);

void SENSORS_INIT_SIGNALS(void);
void SENSORS_RESET_TO_DEFAULTS(void);

#ifdef __cplusplus
}
#endif

#endif //SENSOR_IC_SIGNALS_H
