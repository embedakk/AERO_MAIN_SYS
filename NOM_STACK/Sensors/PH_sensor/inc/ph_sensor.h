#ifndef PH_SENSOR_H
#define PH_SENSOR_H

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

#define PH_SENSOR_ADC_CHANNEL           ADC1_CHANNEL_6 // GPIO34
#define PH_SENSOR_ADC_MAX_READING       65535
#define PH_SENSOR_ADC_REF_VOLTAGE       5  // ESP32 ADC reference voltage
#define PH_SENSOR_PH_NEUTRAL_VOLTAGE    2.5 // Expected voltage at pH 7.0
#define PH_SENSOR_PH_SLOPE              -0.18

#define PH_SENSOR_BUFFER_LEN            5
#define PH_SENSOR_LR_SAMPLE_NR          3   
#define PH_SENSOR_LOW_SOLUTION          4.01
#define PH_SENSOR_MID_SOLUTION          6.86
#define PH_SENSOR_HIGH_SOLUTION         9.18

int ph_sensor_n, ph_sensor_cnt;
float ph_sensor_x[PH_SENSOR_BUFFER_LEN], ph_sensor_y[PH_SENSOR_BUFFER_LEN], 
    ph_sensor_sumX, ph_sensor_sumX2, ph_sensor_sumY, ph_sensor_sumXY, 
    ph_sensor_a, ph_sensor_b;

float ph_sensor_voltage_value;
float ph_sensor_ph_value;

float readPHVoltage();
void sortArray(float *arr, int n);
float medianFilter(float v);
float kalmanUpdate(float measurement);
float voltageToPH(float v);

void PH_SENSOR_INIT(void);
float PH_SENSOR_READ(void);
void PH_SENSOR_CALCULATE_LIN_REGRESSUION(void);
#endif //PH_SENSOR_H