#ifndef EC_SENSOR_H
#define EC_SENSOR_H

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

#include "../../NOM_STACK/Services/I2C_COM/inc/i2c_com.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"


#define         PH_SENSOR_TAG         "PH_SENSOR"

#define         PH_SENSOR_LOGS
#undef          PH_SENSOR_LOGS

#define EC_SENSOR_ADC_CHANNEL           ADC1_CHANNEL_6 // GPIO34
#define EC_SENSOR_ADC_MAX_READING       65535
#define EC_SENSOR_ADC_REF_VOLTAGE       5  // ESP32 ADC reference voltage
#define EC_SENSOR_PH_NEUTRAL_VOLTAGE    2.5 // Expected voltage at pH 7.0
#define EC_SENSOR_PH_SLOPE              -0.18

#define EC_SENSOR_BUFFER_LEN            5
#define EC_SENSOR_LR_SAMPLE_NR          3   
#define EC_SENSOR_LOW_SOLUTION          86
#define EC_SENSOR_MID_SOLUTION          1344
#define EC_SENSOR_HIGH_SOLUTION         12200

// int ph_sensor_n, ph_sensor_cnt;
// float ph_sensor_x[PH_SENSOR_BUFFER_LEN], ph_sensor_y[PH_SENSOR_BUFFER_LEN], 
//     ph_sensor_sumX, ph_sensor_sumX2, ph_sensor_sumY, ph_sensor_sumXY, 
//     ph_sensor_a, ph_sensor_b;

float ec_sensor_voltage_value;
float ec_sensor_ec_value;

void EC_SENSOR_INIT(void);
float EC_SENSOR_READ(void);
void EC_SENSOR_CALCULATE_LIN_REGRESSUION(void);
#endif //EC_SENSOR_H