#ifndef WL_SENSOR_H
#define WL_SENSOR_H

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
#include <sys/time.h>
#include "esp_system.h"
#include "esp_log.h"

#include <driver/gpio.h>
#include <driver/dac.h>

#define         WL_SENSOR_TAG         "WL_SENSOR"

#define         WL_SENSOR_LOGS
#undef          WL_SENSOR_LOGS

#define ESP_ERR_ULTRASONIC_PING         0x200
#define ESP_ERR_ULTRASONIC_PING_TIMEOUT 0x201
#define ESP_ERR_ULTRASONIC_ECHO_TIMEOUT 0x202

#define WL_SENSOR_ECHO_PIN              19
#define WL_SENSOR_TRIGGER_PIN           18

#define TRIGGER_LOW_DELAY 4
#define TRIGGER_HIGH_DELAY 10
#define PING_TIMEOUT 6000
#define ROUNDTRIP 58
#define MAX_DISTANCE_CM 500


#define timeout_expired(start, len) ((uint32_t)(get_time_us() - (start)) >= (len))

#define RETURN_CRTCAL(MUX, RES) do { portEXIT_CRITICAL(&MUX); return RES; } while(0)


/**
 * Device descriptor
 */
typedef struct
{
    gpio_num_t trigger_pin;
    gpio_num_t echo_pin;
} ultrasonic_sensor_t;


/**
 * Init ranging module
 * \param dev Pointer to the device descriptor
 */
void WL_SENSOR_ultrasonic_init(const ultrasonic_sensor_t *dev);

/**
 * Measure distance
 * \param dev Pointer to the device descriptor
 * \param max_distance Maximal distance to measure, centimeters
 * \return Distance in centimeters or ULTRASONIC_ERROR_xxx if error occured
 */
esp_err_t WL_SENSOR_ultrasonic_measure_cm(const ultrasonic_sensor_t *dev, uint32_t max_distance, uint32_t *distance);

void WL_SENSOR_INIT(void);

float WL_SENSOR_GET_DISTANCE_CM(void);

#endif //WL_SENSOR_H