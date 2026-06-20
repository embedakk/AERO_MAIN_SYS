#ifndef NORMAL_OP_H
#define NORMAL_OP_H

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

#define         NORMAL_OP_TAG         "NORMAL_OP"

#define         NORMAL_OP_LOGS
#undef          NORMAL_OP_LOGS

uint8_t *normal_op_i2c_com_ph_ec_sensor_data;

void NORMAL_OP_GET_I2C_COM_PH_EC_SENSOR_DATA(void);
void NORMAL_OP_GET_PH_EC_SENSORS(float *ph_data, float *ec_data); //get the data's from ph and ec libraries or logics
DS1302_DateTime *NORMAL_OP_GET_RTC(void); // will be structure for get data
void NORMAL_OP_SET_RTC(DS1302_DateTime *rtc_data); // will be structure for get data
float NORMAL_OP_GET_WATER_TEMPERATURE(void); // get the data from temperature library
float NORMAL_OP_GET_WATER_LEVEL(void); // get the data from water level library
float NORMAL_OP_GET_WATER_LEVEL_PERCENTAGE(void);
void NORMAL_OP_SET_PH_EC_PUMPS(bool ph_pump_state, bool ec_pump_state);
void NORMAL_OP_SET_MAIN_PUMP(bool main_pump_state);
void NORMAL_OP_SET_CIRCULATION_PUMPS(bool circulation_pumps_state);
void NORMAL_OP_SET_MIXER_MOTORS(bool mixer_motors_state);
void NORMAL_OP_SET_TANK_WATER_INPUT(bool tank_water_input_state);
void NORMAL_OP_SET_TANK_WATER_OUTPUT(bool tank_water_output_state);
float NORMAL_OP_GET_ROOM_TEMPERATURE(void);
float NORMAL_OP_GET_ROOM_HUMIDITY(void);
void NORMAL_OP_SET_ROOM_AIR_CONDITIONER(float room_air_conditioner_data); // room_air_conditioner_data change to struct for all values
void NORMAL_OP_SET_ROOM_LEDS(bool room_leds_state);
void NORMAL_OP_SET_PIPE_VENTILATION(bool pipe_ventilation_state);

void NORMAL_OP_EC_PID(void);
void NORMAL_OP_PH_PID(void);
void NORMAL_OP_PH_CALIBRATION(void);
void NORMAL_OP_EC_CALIBRATION(void);

void NORMAL_OP_PH_CALIBRATION_SET_LOW(void);
void NORMAL_OP_EC_CALIBRATION_SET_LOW(void);
void NORMAL_OP_PH_CALIBRATION_SET_MID(void);
void NORMAL_OP_EC_CALIBRATION_SET_MID(void);
void NORMAL_OP_PH_CALIBRATION_SET_HIGH(void);
void NORMAL_OP_EC_CALIBRATION_SET_HIGH(void);

float NORMAL_OP_PH_CALIBRATION_GET_VOLTAGE(void);
float NORMAL_OP_EC_CALIBRATION_GET_VOLTAGE(void);

void NORMAL_OP_PH_CALIBRATION_CALCULATE_LIN_REGRESSION_VALUES(void);
void NORMAL_OP_EC_CALIBRATION_CALCULATE_LIN_REGRESSION_VALUES(void);

// void NORMAL_OP_PERIPHERALS(void);

#endif // NORMAL_OP_H