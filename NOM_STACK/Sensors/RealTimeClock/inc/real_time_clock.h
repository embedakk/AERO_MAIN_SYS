#ifndef REAL_TIME_CLOCK_H
#define REAL_TIME_CLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/i2c.h"

#include "../../NOM_STACK/Services/I2C_COM/inc/i2c_com.h"

#ifdef __cplusplus
extern "C" {
#endif

#define         REAL_TIME_CLOCK_TAG         "REAL_TIME_CLOCK"

#define         REAL_TIME_CLOCK_LOGS
#undef          REAL_TIME_CLOCK_LOGS

//new RTC DS3231

typedef struct {
    uint8_t second;     //!< Second 0..59
    uint8_t minute;     //!< Minute 0..59
    uint8_t hour;       //!< Hour 0..23
    uint8_t dayWeek;    //!< Day of the week (0 as Sunday/ 1 as Monday)
    uint8_t dayMonth;   //!< Day of the month 1..31
    uint8_t month;      //!< Month 1..12
    uint16_t year;      //!< Year 2000..2099
} DS1302_DateTime;

extern DS1302_DateTime rtc_set_protocol_date;
extern DS1302_DateTime rtc_get_protocol_date;

#define DS3231_ADDR 0x68 //!< I2C address

#define DS3231_STAT_OSCILLATOR 0x80
#define DS3231_STAT_32KHZ      0x08
#define DS3231_STAT_BUSY       0x04
#define DS3231_STAT_ALARM_2    0x02
#define DS3231_STAT_ALARM_1    0x01

#define DS3231_CTRL_OSCILLATOR    0x80
#define DS3231_CTRL_SQUAREWAVE_BB 0x40
#define DS3231_CTRL_TEMPCONV      0x20
#define DS3231_CTRL_ALARM_INTS    0x04
#define DS3231_CTRL_ALARM2_INT    0x02
#define DS3231_CTRL_ALARM1_INT    0x01

#define DS3231_ALARM_WDAY   0x40
#define DS3231_ALARM_NOTSET 0x80

#define DS3231_ADDR_TIME    0x00
#define DS3231_ADDR_ALARM1  0x07
#define DS3231_ADDR_ALARM2  0x0b
#define DS3231_ADDR_CONTROL 0x0e
#define DS3231_ADDR_STATUS  0x0f
#define DS3231_ADDR_AGING   0x10
#define DS3231_ADDR_TEMP    0x11

#define DS3231_12HOUR_FLAG  0x40
#define DS3231_12HOUR_MASK  0x1f
#define DS3231_PM_FLAG      0x20
#define DS3231_MONTH_MASK   0x1f

extern i2c_dev_t rtc_protocol_rtc;
extern struct tm rtc_time_data;

uint8_t bcd2dec(uint8_t val);
uint8_t dec2bcd(uint8_t val);
esp_err_t ds3231_init_desc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio);
esp_err_t ds3231_set_time(i2c_dev_t *dev, struct tm *time);
esp_err_t ds3231_get_raw_temp(i2c_dev_t *dev, int16_t *temp);
esp_err_t ds3231_get_temp_integer(i2c_dev_t *dev, int8_t *temp);
esp_err_t ds3231_get_temp_float(i2c_dev_t *dev, float *temp);
esp_err_t ds3231_get_time(i2c_dev_t *dev, struct tm *time);

void RTC_INIT(void);
void RTC_SET_DATA(DS1302_DateTime *dateTime);
DS1302_DateTime *RTC_GET_DATA(void);
bool RTC_is_hardware_healthy(void);

#ifdef __cplusplus
}
#endif

#endif //REAL_TIME_CLOCK_H
