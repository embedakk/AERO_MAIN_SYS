#include "../inc/real_time_clock.h"
#include "esp_timer.h"

#define RTC_TAG "DS3231"

DS1302_DateTime rtc_set_protocol_date = {0, 0, 0, 1, 1, 1, 2025};
DS1302_DateTime rtc_get_protocol_date = {0, 0, 0, 1, 1, 1, 2025};
i2c_dev_t rtc_protocol_rtc;
struct tm rtc_time_data;
bool rtc_init_state = false;

static int64_t s_last_rtc_tick_us = 0;
static bool s_rtc_tick_initialized = false;
static bool s_rtc_hw_healthy = false;

#define CHECK_ARG(ARG) do { if (!(ARG)) return ESP_ERR_INVALID_ARG; } while (0)

static bool is_valid_bcd(uint8_t val)
{
    return ((val >> 4) <= 9U) && ((val & 0x0FU) <= 9U);
}

static uint8_t days_in_month(uint16_t year, uint8_t month)
{
    static const uint8_t kDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1U || month > 12U) {
        return 31U;
    }
    if (month == 2U) {
        bool leap = ((year % 4U == 0U) && (year % 100U != 0U)) || (year % 400U == 0U);
        return leap ? 29U : 28U;
    }
    return kDays[month - 1U];
}

static void advance_datetime_seconds(DS1302_DateTime *dt, uint32_t elapsed_sec)
{
    if (dt == NULL || elapsed_sec == 0U) {
        return;
    }
    if (dt->year < 2000U || dt->year > 2099U) dt->year = 2025U;
    if (dt->month < 1U || dt->month > 12U) dt->month = 1U;
    if (dt->dayMonth < 1U || dt->dayMonth > 31U) dt->dayMonth = 1U;
    if (dt->hour > 23U) dt->hour = 0U;
    if (dt->minute > 59U) dt->minute = 0U;
    if (dt->second > 59U) dt->second = 0U;

    uint32_t total_sec = (uint32_t)dt->second + elapsed_sec;
    dt->second = (uint8_t)(total_sec % 60U);
    uint32_t carry_min = total_sec / 60U;

    if (carry_min > 0U) {
        uint32_t total_min = (uint32_t)dt->minute + carry_min;
        dt->minute = (uint8_t)(total_min % 60U);
        uint32_t carry_hr = total_min / 60U;

        if (carry_hr > 0U) {
            uint32_t total_hr = (uint32_t)dt->hour + carry_hr;
            dt->hour = (uint8_t)(total_hr % 24U);
            uint32_t carry_days = total_hr / 24U;

            while (carry_days > 0U) {
                uint8_t dim = days_in_month(dt->year, dt->month);
                dt->dayMonth++;
                dt->dayWeek = (uint8_t)((dt->dayWeek + 1U) % 7U);
                if (dt->dayMonth > dim) {
                    dt->dayMonth = 1U;
                    dt->month++;
                    if (dt->month > 12U) {
                        dt->month = 1U;
                        if (dt->year < 2099U) {
                            dt->year++;
                        }
                    }
                }
                carry_days--;
            }
        }
    }
}

uint8_t bcd2dec(uint8_t val)
{
    return (uint8_t)((val >> 4) * 10U + (val & 0x0fU));
}

uint8_t dec2bcd(uint8_t val)
{
    return (uint8_t)(((val / 10U) << 4) + (val % 10U));
}

esp_err_t ds3231_init_desc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    CHECK_ARG(dev);

    dev->port = port;
    dev->addr = DS3231_ADDR;
    dev->sda_io_num = sda_gpio;
    dev->scl_io_num = scl_gpio;
    dev->clk_speed = I2C_FREQ_HZ;

    internal_i2c_driver_status = i2c_dev_init(port, sda_gpio, scl_gpio);
    if (internal_i2c_driver_status != ESP_OK)
    {
#ifdef I2C_COM_LOGS
        ESP_LOGE(I2C_COM_TAG, "I2C_NUM_1 i2c_dev_init ==>> ESP FAILURE !!!");
#endif
    }
    else
    {
#ifdef I2C_COM_LOGS
        ESP_LOGI(I2C_COM_TAG, "I2C_NUM_1  i2c_dev_init ==>> ESP OKK ...");
#endif
    }
    return internal_i2c_driver_status;
}

esp_err_t ds3231_set_time(i2c_dev_t *dev, struct tm *time)
{
    CHECK_ARG(dev);
    CHECK_ARG(time);

    if (time->tm_sec < 0 || time->tm_sec > 59 ||
        time->tm_min < 0 || time->tm_min > 59 ||
        time->tm_hour < 0 || time->tm_hour > 23 ||
        time->tm_mday < 1 || time->tm_mday > 31 ||
        time->tm_mon < 0 || time->tm_mon > 11 ||
        time->tm_year < 2000 || time->tm_year > 2099) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[7];
    int wday = (time->tm_wday >= 0 && time->tm_wday <= 6) ? time->tm_wday : 0;

    /* time/date data */
    data[0] = dec2bcd((uint8_t)time->tm_sec);
    data[1] = dec2bcd((uint8_t)time->tm_min);
    data[2] = dec2bcd((uint8_t)time->tm_hour);
    /* The week data must be in the range 1 to 7, starting at 1 on Sunday. */
    data[3] = dec2bcd((uint8_t)(wday + 1));
    data[4] = dec2bcd((uint8_t)time->tm_mday);
    data[5] = dec2bcd((uint8_t)(time->tm_mon + 1));
    data[6] = dec2bcd((uint8_t)(time->tm_year - 2000));

    return i2c_dev_write_reg(dev, DS3231_ADDR_TIME, data, 7);
}

esp_err_t ds3231_get_raw_temp(i2c_dev_t *dev, int16_t *temp)
{
    CHECK_ARG(dev);
    CHECK_ARG(temp);

    uint8_t data[2];

    esp_err_t res = i2c_dev_read_reg(dev, DS3231_ADDR_TEMP, data, sizeof(data));
    if (res == ESP_OK)
        *temp = (int16_t)((int16_t)(int8_t)data[0] << 2 | (data[1] >> 6));

    return res;
}

esp_err_t ds3231_get_temp_integer(i2c_dev_t *dev, int8_t *temp)
{
    CHECK_ARG(temp);

    int16_t t_int;

    esp_err_t res = ds3231_get_raw_temp(dev, &t_int);
    if (res == ESP_OK)
        *temp = (int8_t)(t_int >> 2);

    return res;
}

esp_err_t ds3231_get_temp_float(i2c_dev_t *dev, float *temp)
{
    CHECK_ARG(temp);

    int16_t t_int;

    esp_err_t res = ds3231_get_raw_temp(dev, &t_int);
    if (res == ESP_OK)
        *temp = t_int * 0.25f;

    return res;
}

esp_err_t ds3231_get_time(i2c_dev_t *dev, struct tm *time)
{
    CHECK_ARG(dev);
    CHECK_ARG(time);

    uint8_t data[7];

    /* read time */
    esp_err_t res = i2c_dev_read_reg(dev, DS3231_ADDR_TIME, data, 7);
    if (res != ESP_OK) return res;

    uint8_t raw_sec = data[0] & 0x7FU;
    uint8_t raw_min = data[1] & 0x7FU;
    uint8_t raw_mday = data[4] & 0x3FU;
    uint8_t raw_mon = data[5] & DS3231_MONTH_MASK;
    uint8_t raw_yr = data[6];

    if (!is_valid_bcd(raw_sec) || !is_valid_bcd(raw_min) ||
        !is_valid_bcd(raw_mday) || !is_valid_bcd(raw_mon) || !is_valid_bcd(raw_yr)) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    int hour = 0;
    if (data[2] & DS3231_12HOUR_FLAG) {
        uint8_t raw_12h = data[2] & DS3231_12HOUR_MASK;
        if (!is_valid_bcd(raw_12h)) return ESP_ERR_INVALID_RESPONSE;
        int h12 = (int)bcd2dec(raw_12h);
        if (h12 < 1 || h12 > 12) return ESP_ERR_INVALID_RESPONSE;
        hour = (h12 % 12) + ((data[2] & DS3231_PM_FLAG) ? 12 : 0);
    } else {
        uint8_t raw_24h = data[2] & 0x3FU;
        if (!is_valid_bcd(raw_24h)) return ESP_ERR_INVALID_RESPONSE;
        hour = (int)bcd2dec(raw_24h);
    }

    int sec = (int)bcd2dec(raw_sec);
    int min = (int)bcd2dec(raw_min);
    int wday = (int)bcd2dec(data[3] & 0x07U) - 1;
    int mday = (int)bcd2dec(raw_mday);
    int mon = (int)bcd2dec(raw_mon) - 1;
    int year = (int)bcd2dec(raw_yr) + 2000;

    if (sec < 0 || sec > 59 || min < 0 || min > 59 || hour < 0 || hour > 23 ||
        mday < 1 || mday > 31 || mon < 0 || mon > 11 || year < 2000 || year > 2099) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    time->tm_sec = sec;
    time->tm_min = min;
    time->tm_hour = hour;
    time->tm_wday = (wday >= 0 && wday <= 6) ? wday : 0;
    time->tm_mday = mday;
    time->tm_mon  = mon;
    time->tm_year = year;
    time->tm_isdst = 0;

    return ESP_OK;
}

void RTC_INIT(void)
{
    s_last_rtc_tick_us = esp_timer_get_time();
    s_rtc_tick_initialized = true;

    //SDA 15 CLK 4
    internal_i2c_driver_status = ds3231_init_desc(&rtc_protocol_rtc, I2C_NUM_1, GPIO_NUM_15, GPIO_NUM_4);
    rtc_init_state = (internal_i2c_driver_status == ESP_OK);
    s_rtc_hw_healthy = rtc_init_state;
    if (internal_i2c_driver_status != ESP_OK)
    {
    #ifdef REAL_TIME_CLOCK_LOGS
        ESP_LOGE(REAL_TIME_CLOCK_TAG, "rtc device is halted plese check the driver connections!!!");
    #endif
    }
    else
    {
    #ifdef REAL_TIME_CLOCK_LOGS
        ESP_LOGI(REAL_TIME_CLOCK_TAG, "rtc was initialized successfully...");
    #endif
    }
}

void RTC_SET_DATA(DS1302_DateTime *dateTime)
{
    if (dateTime == NULL) {
        internal_i2c_driver_status = ESP_ERR_INVALID_ARG;
        return;
    }

    uint16_t year = (dateTime->year >= 2000U && dateTime->year <= 2099U) ? dateTime->year : 2025U;
    uint8_t month = (dateTime->month >= 1U && dateTime->month <= 12U) ? dateTime->month : 1U;
    uint8_t max_day = days_in_month(year, month);
    uint8_t day = (dateTime->dayMonth >= 1U && dateTime->dayMonth <= max_day) ? dateTime->dayMonth : max_day;
    uint8_t hour = (dateTime->hour <= 23U) ? dateTime->hour : 0U;
    uint8_t minute = (dateTime->minute <= 59U) ? dateTime->minute : 0U;
    uint8_t second = (dateTime->second <= 59U) ? dateTime->second : 0U;

    rtc_time_data.tm_year = (int)year;
    rtc_time_data.tm_mon = (int)month - 1;
    rtc_time_data.tm_mday = (int)day;
    rtc_time_data.tm_wday = (int)(dateTime->dayWeek % 7U);
    rtc_time_data.tm_hour = (int)hour;
    rtc_time_data.tm_min = (int)minute;
    rtc_time_data.tm_sec = (int)second;

    /* Update monotonic fallback clock reference immediately as well */
    rtc_get_protocol_date.year = year;
    rtc_get_protocol_date.month = month;
    rtc_get_protocol_date.dayMonth = day;
    rtc_get_protocol_date.dayWeek = (uint8_t)(dateTime->dayWeek % 7U);
    rtc_get_protocol_date.hour = hour;
    rtc_get_protocol_date.minute = minute;
    rtc_get_protocol_date.second = second;
    s_last_rtc_tick_us = esp_timer_get_time();
    s_rtc_tick_initialized = true;

    internal_i2c_driver_status = ds3231_set_time(&rtc_protocol_rtc, &rtc_time_data);
    s_rtc_hw_healthy = (internal_i2c_driver_status == ESP_OK);
    if (internal_i2c_driver_status != ESP_OK)
    {
    #ifdef REAL_TIME_CLOCK_LOGS
        ESP_LOGE(REAL_TIME_CLOCK_TAG, "ds3231_set_time!!!");
    #endif
    }
    else
    {
    #ifdef REAL_TIME_CLOCK_LOGS
        ESP_LOGI(REAL_TIME_CLOCK_TAG, "ds3231_set_time OK...");
    #endif
    }
}

DS1302_DateTime *RTC_GET_DATA(void)
{
    int64_t now_us = esp_timer_get_time();
    if (!s_rtc_tick_initialized) {
        s_last_rtc_tick_us = now_us;
        s_rtc_tick_initialized = true;
    }

    struct tm queried_time;
    memset(&queried_time, 0, sizeof(queried_time));
    internal_i2c_driver_status = ds3231_get_time(&rtc_protocol_rtc, &queried_time);

    if (internal_i2c_driver_status == ESP_OK)
    {
        rtc_time_data = queried_time;
        rtc_get_protocol_date.year     = (uint16_t)rtc_time_data.tm_year;
        rtc_get_protocol_date.month    = (uint8_t)(rtc_time_data.tm_mon + 1);
        rtc_get_protocol_date.dayMonth = (uint8_t)rtc_time_data.tm_mday;
        rtc_get_protocol_date.dayWeek  = (uint8_t)rtc_time_data.tm_wday;
        rtc_get_protocol_date.hour     = (uint8_t)rtc_time_data.tm_hour;
        rtc_get_protocol_date.minute   = (uint8_t)rtc_time_data.tm_min;
        rtc_get_protocol_date.second   = (uint8_t)rtc_time_data.tm_sec;
        s_last_rtc_tick_us = now_us;
        s_rtc_hw_healthy = true;
    }
    else
    {
        s_rtc_hw_healthy = false;
        /* Monotonic fallback progression via esp_timer_get_time() so dosing timers never freeze ON */
        int64_t delta_us = now_us - s_last_rtc_tick_us;
        if (delta_us >= 1000000LL) {
            uint32_t elapsed_sec = (uint32_t)(delta_us / 1000000LL);
            advance_datetime_seconds(&rtc_get_protocol_date, elapsed_sec);
            s_last_rtc_tick_us += (int64_t)elapsed_sec * 1000000LL;
        }
    #ifdef REAL_TIME_CLOCK_LOGS
        ESP_LOGE(REAL_TIME_CLOCK_TAG, "ds3231_get_time fallback active!");
    #endif
    }

    return &rtc_get_protocol_date;
}

bool RTC_is_hardware_healthy(void)
{
    return s_rtc_hw_healthy;
}
