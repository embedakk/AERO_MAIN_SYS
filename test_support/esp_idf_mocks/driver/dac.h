#ifndef DRIVER_DAC_H
#define DRIVER_DAC_H

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DAC_CHANNEL_1 = 0,
    DAC_CHANNEL_2 = 1
} dac_channel_t;

static inline esp_err_t dac_output_enable(dac_channel_t channel) { (void)channel; return ESP_OK; }
static inline esp_err_t dac_output_voltage(dac_channel_t channel, uint8_t dac_value) { (void)channel; (void)dac_value; return ESP_OK; }

#ifdef __cplusplus
}
#endif

#endif // DRIVER_DAC_H

