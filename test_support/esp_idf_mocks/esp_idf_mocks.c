#include "mock_esp_state.h"
#include "esp_timer.h"
#include "esp32/rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"

#include <stdlib.h>
#include <string.h>

static int64_t s_mock_time_us = 1000000LL;
static uint32_t s_task_create_count = 0;
static uint32_t s_mutex_lock_depth = 0;
static int s_dummy_mutex_token = 1;
static int s_dummy_queue_token = 1;

// NVS state
static uint8_t s_nvs_blob[512];
static size_t s_nvs_blob_len = 0;
static bool s_nvs_has_blob = false;
static bool s_nvs_fail_open = false;
static uint32_t s_nvs_commit_count = 0;

// UART TX capture
static uint8_t s_uart_tx_buf[3][MOCK_UART_TX_CAP];
static size_t s_uart_tx_len[3] = {0};

// I2C state
typedef struct {
    bool has_reg_addr;
    uint8_t reg_addr;
    uint8_t write_buf[64];
    size_t write_len;
    uint8_t *read_dst;
    size_t read_len;
} mock_i2c_cmd_t;

static uint8_t s_i2c_regs[256];
static esp_err_t s_i2c_next_result = ESP_OK;
static uint32_t s_i2c_cmd_alloc_count = 0;
static uint32_t s_i2c_cmd_free_count = 0;

// GPIO state
static int s_gpio_in_level[64];
static int s_gpio_out_level[64];
static mock_gpio_read_cb_t s_gpio_read_cb = NULL;

void mock_nvs_reset(void)
{
    memset(s_nvs_blob, 0, sizeof(s_nvs_blob));
    s_nvs_blob_len = 0;
    s_nvs_has_blob = false;
    s_nvs_fail_open = false;
    s_nvs_commit_count = 0;
}

void mock_esp_reset_all(void)
{
    s_mock_time_us = 1000000LL;
    s_task_create_count = 0;
    s_mutex_lock_depth = 0;
    mock_nvs_reset();

    for (int i = 0; i < 3; i++) {
        s_uart_tx_len[i] = 0;
        memset(s_uart_tx_buf[i], 0, MOCK_UART_TX_CAP);
    }

    memset(s_i2c_regs, 0, sizeof(s_i2c_regs));
    s_i2c_next_result = ESP_OK;
    s_i2c_cmd_alloc_count = 0;
    s_i2c_cmd_free_count = 0;

    for (int i = 0; i < 64; i++) {
        s_gpio_in_level[i] = 1; // Idle pull-up high (e.g., OneWire idle / GPIO default)
        s_gpio_out_level[i] = 0;
    }
    s_gpio_in_level[19] = 0; // Ultrasonic echo default low
    s_gpio_read_cb = NULL;
}

void mock_esp_set_time_us(int64_t time_us) { s_mock_time_us = time_us; }
void mock_esp_advance_time_us(int64_t delta_us) { s_mock_time_us += delta_us; }
int64_t mock_esp_get_time_us(void) { return s_mock_time_us; }
void vPortYield(void) { s_mock_time_us += 1000LL; }

int64_t esp_timer_get_time(void) { return s_mock_time_us; }
void ets_delay_us(uint32_t us) { s_mock_time_us += (int64_t)us; }

uint32_t mock_esp_get_task_create_count(void) { return s_task_create_count; }
uint32_t mock_esp_get_mutex_lock_depth(void) { return s_mutex_lock_depth; }

BaseType_t xTaskCreate(TaskFunction_t pxTaskCode, const char * const pcName, const uint32_t usStackDepth,
                       void * const pvParameters, UBaseType_t uxPriority, TaskHandle_t * const pxCreatedTask)
{
    (void)pxTaskCode; (void)pcName; (void)usStackDepth; (void)pvParameters; (void)uxPriority;
    s_task_create_count++;
    if (pxCreatedTask) *pxCreatedTask = (TaskHandle_t)(uintptr_t)s_task_create_count;
    return pdPASS;
}

BaseType_t xTaskCreatePinnedToCore(TaskFunction_t pxTaskCode, const char * const pcName, const uint32_t usStackDepth,
                                   void * const pvParameters, UBaseType_t uxPriority, TaskHandle_t * const pvCreatedTask,
                                   const BaseType_t xCoreID)
{
    (void)xCoreID;
    return xTaskCreate(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pvCreatedTask);
}

void vTaskDelay(const TickType_t xTicksToDelay) { s_mock_time_us += (int64_t)xTicksToDelay * 1000LL; }
void vTaskDelayUntil(TickType_t * const pxPreviousWakeTime, const TickType_t xTimeIncrement)
{
    if (pxPreviousWakeTime) *pxPreviousWakeTime += xTimeIncrement;
    s_mock_time_us += (int64_t)xTimeIncrement * 1000LL;
}
void vTaskDelete(TaskHandle_t xTaskToDelete) { (void)xTaskToDelete; }
TickType_t xTaskGetTickCount(void) { return (TickType_t)(s_mock_time_us / 1000LL); }

SemaphoreHandle_t xSemaphoreCreateMutex(void) { return &s_dummy_mutex_token; }
SemaphoreHandle_t xSemaphoreCreateRecursiveMutex(void) { return &s_dummy_mutex_token; }
BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xBlockTime)
{
    (void)xSemaphore; (void)xBlockTime;
    s_mutex_lock_depth++;
    return pdTRUE;
}
BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore)
{
    (void)xSemaphore;
    if (s_mutex_lock_depth > 0) s_mutex_lock_depth--;
    return pdTRUE;
}
BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t xMutex, TickType_t xBlockTime)
{
    return xSemaphoreTake(xMutex, xBlockTime);
}
BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t xMutex)
{
    return xSemaphoreGive(xMutex);
}

QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize)
{
    (void)uxQueueLength; (void)uxItemSize;
    return &s_dummy_queue_token;
}
BaseType_t xQueueReceive(QueueHandle_t xQueue, void * const pvBuffer, TickType_t xTicksToWait)
{
    (void)xQueue; (void)pvBuffer; (void)xTicksToWait;
    return pdFALSE;
}
BaseType_t xQueueSend(QueueHandle_t xQueue, const void * const pvItemToQueue, TickType_t xTicksToWait)
{
    (void)xQueue; (void)pvItemToQueue; (void)xTicksToWait;
    return pdTRUE;
}
BaseType_t xQueueReset(QueueHandle_t xQueue)
{
    (void)xQueue;
    return pdTRUE;
}

// NVS
uint32_t mock_nvs_get_commit_count(void) { return s_nvs_commit_count; }
bool mock_nvs_has_blob(void) { return s_nvs_has_blob; }
void mock_nvs_set_fail_open(bool fail) { s_nvs_fail_open = fail; }
void mock_nvs_set_raw_blob(const void *data, size_t len)
{
    if (data != NULL && len <= sizeof(s_nvs_blob)) {
        memcpy(s_nvs_blob, data, len);
        s_nvs_blob_len = len;
        s_nvs_has_blob = true;
    }
}

esp_err_t nvs_flash_init(void) { return ESP_OK; }
esp_err_t nvs_flash_erase(void) { mock_nvs_reset(); return ESP_OK; }

esp_err_t nvs_open(const char *name, nvs_open_mode_t open_mode, nvs_handle_t *out_handle)
{
    (void)name;
    if (s_nvs_fail_open) return ESP_FAIL;
    if (open_mode == NVS_READONLY && !s_nvs_has_blob) return ESP_ERR_NVS_NOT_FOUND;
    if (out_handle) *out_handle = 1U;
    return ESP_OK;
}
void nvs_close(nvs_handle_t handle) { (void)handle; }

esp_err_t nvs_set_blob(nvs_handle_t handle, const char *key, const void *value, size_t length)
{
    (void)handle; (void)key;
    if (!value || length > sizeof(s_nvs_blob)) return ESP_ERR_INVALID_SIZE;
    memcpy(s_nvs_blob, value, length);
    s_nvs_blob_len = length;
    s_nvs_has_blob = true;
    return ESP_OK;
}

esp_err_t nvs_get_blob(nvs_handle_t handle, const char *key, void *out_value, size_t *length)
{
    (void)handle; (void)key;
    if (!s_nvs_has_blob) return ESP_ERR_NVS_NOT_FOUND;
    if (!out_value || !length) return ESP_ERR_INVALID_ARG;
    if (*length < s_nvs_blob_len) {
        *length = s_nvs_blob_len;
        return ESP_ERR_INVALID_SIZE;
    }
    memcpy(out_value, s_nvs_blob, s_nvs_blob_len);
    *length = s_nvs_blob_len;
    return ESP_OK;
}

esp_err_t nvs_commit(nvs_handle_t handle)
{
    (void)handle;
    s_nvs_commit_count++;
    return ESP_OK;
}

// GPIO
void mock_gpio_set_input_level(int gpio_num, int level)
{
    if (gpio_num >= 0 && gpio_num < 64) s_gpio_in_level[gpio_num] = level;
}
int mock_gpio_get_output_level(int gpio_num)
{
    return (gpio_num >= 0 && gpio_num < 64) ? s_gpio_out_level[gpio_num] : 0;
}
void mock_gpio_set_read_callback(mock_gpio_read_cb_t cb) { s_gpio_read_cb = cb; }

esp_err_t gpio_config(const gpio_config_t *pGPIOConfig) { (void)pGPIOConfig; return ESP_OK; }
esp_err_t gpio_reset_pin(gpio_num_t gpio_num) { (void)gpio_num; return ESP_OK; }
void gpio_pad_select_gpio(int gpio_num) { (void)gpio_num; }
esp_err_t gpio_set_direction(gpio_num_t gpio_num, gpio_mode_t mode) { (void)gpio_num; (void)mode; return ESP_OK; }
esp_err_t gpio_set_level(gpio_num_t gpio_num, uint32_t level)
{
    if (gpio_num >= 0 && gpio_num < 64) s_gpio_out_level[gpio_num] = (int)level;
    return ESP_OK;
}
int gpio_get_level(gpio_num_t gpio_num)
{
    if (s_gpio_read_cb) return s_gpio_read_cb(gpio_num, s_mock_time_us);
    return (gpio_num >= 0 && gpio_num < 64) ? s_gpio_in_level[gpio_num] : 0;
}

// UART
void mock_uart_clear_tx(int port)
{
    if (port >= 0 && port < 3) {
        s_uart_tx_len[port] = 0;
    }
}
size_t mock_uart_get_tx_len(int port)
{
    return (port >= 0 && port < 3) ? s_uart_tx_len[port] : 0;
}
const uint8_t *mock_uart_get_tx_data(int port)
{
    return (port >= 0 && port < 3) ? s_uart_tx_buf[port] : NULL;
}

esp_err_t uart_param_config(uart_port_t uart_num, const uart_config_t *uart_config) { (void)uart_num; (void)uart_config; return ESP_OK; }
esp_err_t uart_set_pin(uart_port_t uart_num, int tx, int rx, int rts, int cts) { (void)uart_num; (void)tx; (void)rx; (void)rts; (void)cts; return ESP_OK; }
esp_err_t uart_driver_install(uart_port_t uart_num, int rx_buf, int tx_buf, int q_size, QueueHandle_t *uart_queue, int flags)
{
    (void)uart_num; (void)rx_buf; (void)tx_buf; (void)q_size; (void)flags;
    if (uart_queue) *uart_queue = &s_dummy_queue_token;
    return ESP_OK;
}
esp_err_t uart_enable_rx_intr(uart_port_t uart_num) { (void)uart_num; return ESP_OK; }

int uart_write_bytes(uart_port_t uart_num, const void *src, size_t size)
{
    if (uart_num >= 0 && uart_num < 3 && src != NULL && size > 0) {
        size_t avail = MOCK_UART_TX_CAP - s_uart_tx_len[uart_num];
        size_t to_copy = (size <= avail) ? size : avail;
        memcpy(&s_uart_tx_buf[uart_num][s_uart_tx_len[uart_num]], src, to_copy);
        s_uart_tx_len[uart_num] += to_copy;
    }
    return (int)size;
}
int uart_read_bytes(uart_port_t uart_num, void *buf, uint32_t length, TickType_t ticks_to_wait)
{
    (void)uart_num; (void)buf; (void)length; (void)ticks_to_wait;
    return 0;
}
esp_err_t uart_get_buffered_data_len(uart_port_t uart_num, size_t *size)
{
    (void)uart_num;
    if (size) *size = 0;
    return ESP_OK;
}
esp_err_t uart_flush_input(uart_port_t uart_num) { (void)uart_num; return ESP_OK; }

// I2C
void mock_i2c_set_next_result(esp_err_t err) { s_i2c_next_result = err; }
void mock_i2c_set_regs(uint8_t start_reg, const uint8_t *vals, size_t len)
{
    for (size_t i = 0; i < len && (start_reg + i) < 256U; i++) {
        s_i2c_regs[start_reg + i] = vals[i];
    }
}
void mock_i2c_get_regs(uint8_t start_reg, uint8_t *out_vals, size_t len)
{
    for (size_t i = 0; i < len && (start_reg + i) < 256U; i++) {
        out_vals[i] = s_i2c_regs[start_reg + i];
    }
}
uint32_t mock_i2c_get_cmd_alloc_count(void) { return s_i2c_cmd_alloc_count; }
uint32_t mock_i2c_get_cmd_free_count(void) { return s_i2c_cmd_free_count; }

esp_err_t i2c_param_config(i2c_port_t i2c_num, const i2c_config_t *i2c_conf) { (void)i2c_num; (void)i2c_conf; return s_i2c_next_result; }
esp_err_t i2c_driver_install(i2c_port_t i2c_num, i2c_mode_t mode, size_t rx_len, size_t tx_len, int flags)
{
    (void)i2c_num; (void)mode; (void)rx_len; (void)tx_len; (void)flags;
    return s_i2c_next_result;
}
esp_err_t i2c_driver_delete(i2c_port_t i2c_num) { (void)i2c_num; return ESP_OK; }

i2c_cmd_handle_t i2c_cmd_link_create(void)
{
    s_i2c_cmd_alloc_count++;
    mock_i2c_cmd_t *cmd = (mock_i2c_cmd_t *)calloc(1, sizeof(mock_i2c_cmd_t));
    return (i2c_cmd_handle_t)cmd;
}
void i2c_cmd_link_delete(i2c_cmd_handle_t cmd_handle)
{
    if (cmd_handle) {
        s_i2c_cmd_free_count++;
        free(cmd_handle);
    }
}
esp_err_t i2c_master_start(i2c_cmd_handle_t cmd_handle) { (void)cmd_handle; return ESP_OK; }
esp_err_t i2c_master_stop(i2c_cmd_handle_t cmd_handle) { (void)cmd_handle; return ESP_OK; }
esp_err_t i2c_master_write_byte(i2c_cmd_handle_t cmd_handle, uint8_t data, bool ack_en)
{
    (void)cmd_handle; (void)data; (void)ack_en;
    return ESP_OK;
}
esp_err_t i2c_master_write(i2c_cmd_handle_t cmd_handle, const uint8_t *data, size_t data_len, bool ack_en)
{
    (void)ack_en;
    mock_i2c_cmd_t *cmd = (mock_i2c_cmd_t *)cmd_handle;
    if (!cmd || !data || data_len == 0) return ESP_ERR_INVALID_ARG;
    if (!cmd->has_reg_addr) {
        cmd->has_reg_addr = true;
        cmd->reg_addr = data[0];
        if (data_len > 1) {
            size_t rem = data_len - 1;
            if (rem > sizeof(cmd->write_buf)) rem = sizeof(cmd->write_buf);
            memcpy(cmd->write_buf, data + 1, rem);
            cmd->write_len = rem;
        }
    } else {
        size_t avail = sizeof(cmd->write_buf) - cmd->write_len;
        size_t to_copy = (data_len <= avail) ? data_len : avail;
        memcpy(cmd->write_buf + cmd->write_len, data, to_copy);
        cmd->write_len += to_copy;
    }
    return ESP_OK;
}
esp_err_t i2c_master_read_byte(i2c_cmd_handle_t cmd_handle, uint8_t *data, i2c_ack_type_t ack)
{
    return i2c_master_read(cmd_handle, data, 1, ack);
}
esp_err_t i2c_master_read(i2c_cmd_handle_t cmd_handle, uint8_t *data, size_t data_len, i2c_ack_type_t ack)
{
    (void)ack;
    mock_i2c_cmd_t *cmd = (mock_i2c_cmd_t *)cmd_handle;
    if (!cmd || !data) return ESP_ERR_INVALID_ARG;
    cmd->read_dst = data;
    cmd->read_len = data_len;
    return ESP_OK;
}
esp_err_t i2c_master_cmd_begin(i2c_port_t i2c_num, i2c_cmd_handle_t cmd_handle, TickType_t ticks_to_wait)
{
    (void)i2c_num; (void)ticks_to_wait;
    mock_i2c_cmd_t *cmd = (mock_i2c_cmd_t *)cmd_handle;
    if (!cmd) return ESP_ERR_INVALID_ARG;
    if (s_i2c_next_result != ESP_OK) {
        return s_i2c_next_result;
    }
    uint8_t reg = cmd->has_reg_addr ? cmd->reg_addr : 0U;
    if (cmd->write_len > 0) {
        mock_i2c_set_regs(reg, cmd->write_buf, cmd->write_len);
    }
    if (cmd->read_dst && cmd->read_len > 0) {
        mock_i2c_get_regs(reg, cmd->read_dst, cmd->read_len);
    }
    return ESP_OK;
}

int i2c_slave_read_buffer(i2c_port_t i2c_num, uint8_t *data, size_t max_size, TickType_t ticks_to_wait)
{
    (void)i2c_num; (void)data; (void)max_size; (void)ticks_to_wait;
    return 0;
}
int i2c_slave_write_buffer(i2c_port_t i2c_num, const uint8_t *data, int size, TickType_t ticks_to_wait)
{
    (void)i2c_num; (void)data; (void)ticks_to_wait;
    return size;
}

