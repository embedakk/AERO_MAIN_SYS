#ifndef I2C_COM_H
#define I2C_COM_H

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

#include "driver/gpio.h"
#include "driver/i2c.h"

#define         I2C_COM_TAG         "I2C_COM"

#define         I2C_COM_LOGS
#undef          I2C_COM_LOGS

#define I2C_COM_PH_SLAVE_ADDR	            0x22//0xAA // 170
#define I2C_COM_EC_SLAVE_ADDR	            0x24    //0xAB // 171
#define I2C_COM_ESP_SLAVE_TIMEOUT           10 / portTICK_PERIOD_MS  // Timeout

#define I2C_COM_I2C_SLAVE_NUM               I2C_NUM_0   // I2C Port Number
#define I2C_COM_I2C_SLAVE_SDA               21          // SDA Pin
#define I2C_COM_I2C_SLAVE_SCL               22          // SCL Pin
#define I2C_COM_I2C_SLAVE_ADDR              0x08        // I2C Slave Address
#define I2C_COM_I2C_BUF_LEN                 128         // Buffer Size

#define I2C_FREQ_HZ 400000
#define I2CDEV_TIMEOUT 1000


esp_err_t internal_i2c_driver_status;
uint8_t i2c_com_sensor_data_input[10];
uint16_t i2c_com_ph_data, i2c_com_ec_data; //voltage raw data's
int i2c_com_sensor_data_input_len;

typedef struct {
	i2c_port_t port;	// I2C port number
	uint8_t addr;		// I2C address
	gpio_num_t sda_io_num;	// GPIO number for I2C sda signal
	gpio_num_t scl_io_num;	// GPIO number for I2C scl signal
	uint32_t clk_speed;		// I2C clock frequency for master mode
} i2c_dev_t;

esp_err_t i2c_dev_init(i2c_port_t port, int sda, int scl);
esp_err_t i2c_dev_read(const i2c_dev_t *dev, const void *out_data, size_t out_size, void *in_data, size_t in_size);
esp_err_t i2c_dev_write(const i2c_dev_t *dev, const void *out_reg, size_t out_reg_size, const void *out_data, size_t out_size);
inline esp_err_t i2c_dev_read_reg(const i2c_dev_t *dev, uint8_t reg, void *in_data, size_t in_size)
{
	return i2c_dev_read(dev, &reg, 1, in_data, in_size);
}

inline esp_err_t i2c_dev_write_reg(const i2c_dev_t *dev, uint8_t reg, const void *out_data, size_t out_size)
{
	return i2c_dev_write(dev, &reg, 1, out_data, out_size);
}


void I2C_COM_INIT(void);
uint8_t *I2C_COM_GET_SENSOR_DATA(void);

#endif //I2C_COM_H