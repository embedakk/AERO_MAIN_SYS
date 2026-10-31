#ifndef MOCK_ESP_STATE_H
#define MOCK_ESP_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*mock_gpio_read_cb_t)(int gpio_num, int64_t now_us);

void mock_esp_reset_all(void);

// Time controls
void mock_esp_set_time_us(int64_t time_us);
void mock_esp_advance_time_us(int64_t delta_us);
int64_t mock_esp_get_time_us(void);

// FreeRTOS task / mutex counters
uint32_t mock_esp_get_task_create_count(void);
uint32_t mock_esp_get_mutex_lock_depth(void);

// NVS mock inspection & fault injection
void mock_nvs_reset(void);
uint32_t mock_nvs_get_commit_count(void);
void mock_nvs_set_raw_blob(const void *data, size_t len);
bool mock_nvs_has_blob(void);
void mock_nvs_set_fail_open(bool fail);

// UART TX capture (ports 0..2)
#define MOCK_UART_TX_CAP 8192U
void mock_uart_clear_tx(int port);
size_t mock_uart_get_tx_len(int port);
const uint8_t *mock_uart_get_tx_data(int port);

// I2C mock registers & command-link leak tracking
void mock_i2c_set_next_result(esp_err_t err);
void mock_i2c_set_regs(uint8_t start_reg, const uint8_t *vals, size_t len);
void mock_i2c_get_regs(uint8_t start_reg, uint8_t *out_vals, size_t len);
uint32_t mock_i2c_get_cmd_alloc_count(void);
uint32_t mock_i2c_get_cmd_free_count(void);

// GPIO mock controls
void mock_gpio_set_input_level(int gpio_num, int level);
int mock_gpio_get_output_level(int gpio_num);
void mock_gpio_set_read_callback(mock_gpio_read_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif // MOCK_ESP_STATE_H

