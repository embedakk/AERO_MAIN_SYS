#include "../inc/real_time_clock.h"


#define RTC_TAG "DS1302"
#define DS1302_LOGS
#undef  DS1302_LOGS

// /*!
//  * \brief Initialize DS1302.
//  * \param clkPin
//  *      Clock pin
//  * \param ioPin
//  *      I/O pin.
//  * \param cePin
//  *      Chip select pin. (In previous versions RST pin which is the same)
//  * \return
//  *      true:  RTC running
//  *      false: RTC halted or not detected
//  */
// bool DS1302_begin(DS1302_Dev *dev, uint8_t clkPin, uint8_t ioPin, uint8_t cePin)
// {
//     //gpio_pad_select_gpio(clkPin);
//     //gpio_pad_select_gpio(ioPin);
//     //gpio_pad_select_gpio(cePin);
//     gpio_reset_pin(clkPin);
//     gpio_reset_pin(ioPin);
//     gpio_reset_pin(cePin);

//     dev->clkPin = clkPin;
//     dev->ioPin = ioPin;
//     dev->cePin = cePin;

//     // Initialize pins
//     gpio_set_level(dev->clkPin, 0);
//     gpio_set_level(dev->ioPin, 0);
//     gpio_set_level(dev->cePin, 0);

//     gpio_set_direction(dev->clkPin, GPIO_MODE_OUTPUT);
//     gpio_set_direction(dev->ioPin, GPIO_MODE_OUTPUT);
//     gpio_set_direction(dev->cePin, GPIO_MODE_OUTPUT);

//     // Enable RTC clock
//     DS1302_halt(dev, false);

//     // Check Clocl Halt Flag
//     if (DS1302_isHalted(dev)) {
//     	return false;
//     } else {
//         return true;
//     }
// }

// /*!
//  * \brief Set write protect flag
//  * \param enable
//  *      true:  Enable RTC write protect
//  *      false: Disable RTC write protect
//  */
// void DS1302_writeProtect(DS1302_Dev *dev, bool enable)
// {
//     DS1302_writeClockRegister(dev, DS1302_REG_WP, (uint8_t) (enable << 7));
// }

// /*!
//  * \brief Get write protect state
//  * \return
//  *      true:  RTC registers are read only
//  *      false: RTC registers are writable
//  */
// bool DS1302_isWriteProtected(DS1302_Dev *dev)
// {
//     if (DS1302_readClockRegister(dev, DS1302_REG_WP) & (1 << DS1302_BIT_WP)) {
//         return true;
//     } else {
//         return false;
//     }
// }

// /*!
//  * \brief Set RTC clock halted or running
//  * \param halt
//  *      true:  Enable RTC clock
//  *      false: Halt RTC clock
//  */
// void DS1302_halt(DS1302_Dev *dev, bool halt)
// {
//     uint8_t regOld;
//     uint8_t regNew;
    
//     regOld = DS1302_readClockRegister(dev, DS1302_REG_SECONDS);
// #ifdef DS1302_LOGS
//     ESP_LOGD(RTC_TAG, "DS1302_halt regOld=%02x", regOld);
// #endif
//     if (halt) {
//         regNew = (uint8_t)(regOld | (1 << DS1302_BIT_CH));
//     } else {
//         regNew = (uint8_t)(regOld & ~(1 << DS1302_BIT_CH));
//     }
    
//     if (regOld != regNew) {
//         DS1302_writeClockRegister(dev, DS1302_REG_SECONDS, regNew);
//     }
// }

// /*!
//  * \brief Get RTC halt status
//  * \return
//  *      true:  RTC clock is halted
//  *      false: RTC clock is running
//  */
// bool DS1302_isHalted(DS1302_Dev *dev)
// {
//     uint8_t val = DS1302_readClockRegister(dev, DS1302_REG_SECONDS);
// #ifdef DS1302_LOGS
//     ESP_LOGD(RTC_TAG,"DS1302_REG_SECONDS=%02x",val);
// #endif
//     if (DS1302_readClockRegister(dev, DS1302_REG_SECONDS) & (1 << DS1302_BIT_CH)) {
//         return true;
//     } else {
//         return false;
//     }
// }

// /*!
//  * \brief Set RTC date and time
//  * \param dateTime
//  *      Date time structure
//  */
// void DS1302_setDateTime(DS1302_Dev *dev, DS1302_DateTime *dateTime)
// {
//     uint8_t ch;

//     // Read CH bit
//     ch = (uint8_t)(DS1302_readClockRegister(dev, 0) & (1 << DS1302_BIT_CH));

//     // Write clock registers(always 24H)
//     DS1302_transferBegin(dev);
//     DS1302_writeAddrCmd(dev, DS1302_CMD_WRITE_CLOCK_BURST);
//     DS1302_writeByte(dev, ch | decToBcd((uint8_t)(dateTime->second & 0x7F)));
//     DS1302_writeByte(dev, decToBcd(dateTime->minute));
//     DS1302_writeByte(dev, decToBcd((uint8_t)(dateTime->hour & 0x3F)));
//     DS1302_writeByte(dev, decToBcd((uint8_t)(dateTime->dayMonth & 0x3F)));
//     DS1302_writeByte(dev, decToBcd((uint8_t)(dateTime->month & 0x1F)));
//     DS1302_writeByte(dev, decToBcd((uint8_t)(dateTime->dayWeek & 0x07)));
//     DS1302_writeByte(dev, decToBcd((uint8_t)((dateTime->year - 2000) & 0xFF)));
//     DS1302_writeByte(dev, 0); // Including write protect = 0
//     DS1302_transferEnd(dev);
// }

// /*!
//  * \brief Get RTC date and time
//  * \param dateTime
//  *      Date and time structure
//  */
// bool DS1302_getDateTime(DS1302_Dev *dev, DS1302_DateTime *dateTime)
// {
//     uint8_t buf[7];

//     // Read clock date and time registers
//     DS1302_transferBegin(dev);
//     DS1302_writeAddrCmd(dev, DS1302_CMD_READ_CLOCK_BURST);
//     DS1302_readBuffer(dev, &buf, sizeof(buf));
// #ifdef DS1302_LOGS
//     for(int i=0;i<7;i++) ESP_LOGD(RTC_TAG, "buf[%d]=0x%x",i,buf[i]);
// #endif
//     DS1302_transferEnd(dev);

//     // Convert BCD buffer to Decimal
//     dateTime->second = bcdToDec(buf[0] & 0x7f); // Without CH bit from seconds register
//     dateTime->minute = bcdToDec(buf[1]);
//     dateTime->hour = bcdToDec(buf[2]);
//     dateTime->dayMonth = bcdToDec(buf[3]);
//     dateTime->month = bcdToDec(buf[4]);
//     dateTime->dayWeek = bcdToDec(buf[5]);
//     dateTime->year = 2000 + bcdToDec(buf[6]);
// #ifdef DS1302_LOGS
//     ESP_LOGE(RTC_TAG, "DS1302 Get Date Time... POSSIBLE ISSUE WITH ASSIGNMENT!!!");
//     ESP_LOGW(RTC_TAG, "dateTime->second=%d",dateTime->second);
//     ESP_LOGW(RTC_TAG, "dateTime->minute=%d",dateTime->minute);
//     ESP_LOGW(RTC_TAG, "dateTime->hour=%d",dateTime->hour);
//     ESP_LOGW(RTC_TAG, "dateTime->dayMonth=%d",dateTime->dayMonth);
//     ESP_LOGW(RTC_TAG, "dateTime->month=%d",dateTime->month);
//     ESP_LOGW(RTC_TAG, "dateTime->dayWeek=%d",dateTime->dayWeek);
//     ESP_LOGW(RTC_TAG, "dateTime->year=%d",dateTime->year);
// #endif
//     // Check buffer for valid data
//     if ((dateTime->second > 59) ||
//         (dateTime->minute > 59) ||
//         (dateTime->hour > 23) ||
//         (dateTime->dayMonth < 1) || (dateTime->dayMonth > 31) ||
//         (dateTime->month < 1) || (dateTime->month > 12) ||
//         (dateTime->dayWeek < 1) || (dateTime->dayWeek > 7) ||
//         (dateTime->year > 2099))
//     {
// #ifdef DS1302_LOGS
//         ESP_LOGW(RTC_TAG, "DATA IS NOT VALID!!!");
//         ESP_LOGW(RTC_TAG, "dateTime->second=%d",dateTime->second);
//         ESP_LOGW(RTC_TAG, "dateTime->minute=%d",dateTime->minute);
//         ESP_LOGW(RTC_TAG, "dateTime->hour=%d",dateTime->hour);
//         ESP_LOGW(RTC_TAG, "dateTime->dayMonth=%d",dateTime->dayMonth);
//         ESP_LOGW(RTC_TAG, "dateTime->month=%d",dateTime->month);
//         ESP_LOGW(RTC_TAG, "dateTime->dayWeek=%d",dateTime->dayWeek);
//         ESP_LOGW(RTC_TAG, "dateTime->year=%d",dateTime->year);
// #endif
//         memset(dateTime, 0x00, sizeof(DS1302_DateTime));
//         return false;
//     }

//     return true;
// }

// /*!
//  * \brief Set RTC time
//  * \param hour Hours
//  * \param minute Minutes
//  * \param second Seconds
//  */
// void DS1302_setTime(DS1302_Dev *dev, uint8_t hour, uint8_t minute, uint8_t second)
// {
//     DS1302_DateTime dt = { 0 };

//     DS1302_getDateTime(dev, &dt);
//     dt.hour = hour;
//     dt.minute = minute;
//     dt.second = second;
//     DS1302_setDateTime(dev, &dt);
// }

// /*!
//  * \brief Get RTC time
//  * \param hour Hours
//  * \param minute Minutes
//  * \param second Seconds
//  */
// bool DS1302_getTime(DS1302_Dev *dev, uint8_t *hour, uint8_t *minute, uint8_t *second)
// {
//     uint8_t buf[3];

//     // Read clock time registers
//     DS1302_transferBegin(dev);
//     DS1302_writeAddrCmd(dev, DS1302_CMD_READ_CLOCK_BURST);
//     DS1302_readBuffer(dev, &buf, sizeof(buf));
//     DS1302_transferEnd(dev);

//     // Convert BCD buffer to Decimal
//     *second = bcdToDec(buf[0] & 0x7f); // Without CH bit from seconds register
//     *minute = bcdToDec(buf[1]);
//     *hour = bcdToDec(buf[2]);

//     // Check buffer for valid data
//     if ((*second > 59) || (*minute > 59) || (*hour > 23)) {
//         *second = 0x00;
//         *minute = 0x00;
//         *hour = 0x00;
//         return false;
//     }

//     return true;
// }

// /*!
//  * \brief Write a byte to RAM
//  * \param addr
//  *      RAM address 0..0x1E
//  * \param value
//  *      RAM byte 0..0xFF
//  */
// void DS1302_writeByteRAM(DS1302_Dev *dev, uint8_t addr, uint8_t value)
// {
//     DS1302_transferBegin(dev);
//     DS1302_writeAddrCmd(dev, (uint8_t)DS1302_CMD_WRITE_RAM(addr));
//     DS1302_writeByte(dev, value);
//     DS1302_transferEnd(dev);
// }

// #ifndef min
// #define min(a,b)            (((a) < (b)) ? (a) : (b))
// #endif

// /*!
//  * \brief Write buffer to RAM address 0x00 (burst write)
//  * \param buf
//  *      Data buffer
//  * \param len
//  *      Buffer length 0x01..0x1E
//  */
// void DS1302_writeBufferRAM(DS1302_Dev *dev, uint8_t *buf, uint8_t len)
// {
//     DS1302_transferBegin(dev);
//     DS1302_writeAddrCmd(dev, DS1302_CMD_WRITE_RAM_BURST);
//     for (uint8_t i = 0; i < min((int)len, NUM_DS1302_RAM_REGS); i++) {
//         DS1302_writeByte(dev, *buf++);
//     }
//     DS1302_transferEnd(dev);
// }

// /*!
//  * \brief Read byte from RAM
//  * \param addr
//  *      RAM address 0..0x1E
//  * \return
//  *      RAM byte 0..0xFF
//  */
// uint8_t DS1302_readByteRAM(DS1302_Dev *dev, uint8_t addr)
// {
//     uint8_t value;

//     DS1302_transferBegin(dev);
//     DS1302_writeAddrCmd(dev, (uint8_t)DS1302_CMD_READ_RAM(addr));
//     value = DS1302_readByte(dev);
//     DS1302_transferEnd(dev);

//     return value;
// }

// /*!
//  * \brief Read buffer from RAM address 0x00 (burst read)
//  * \param buf
//  *      Data buffer
//  * \param len
//  *      Buffer length
//  */
// void DS1302_readBufferRAM(DS1302_Dev *dev, uint8_t *buf, uint8_t len)
// {
//     DS1302_transferBegin(dev);
//     DS1302_writeAddrCmd(dev, DS1302_CMD_READ_RAM_BURST);
//     for (uint8_t i = 0; i < min((int)len, NUM_DS1302_RAM_REGS); i++) {
//         *buf++ = DS1302_readByte(dev);
//     }
//     DS1302_transferEnd(dev);
// }

// // -------------------------------------------------------------------------------------------------
// /*!
//  * \brief Write clock register
//  * \param reg
//  *      RTC clock register (See datasheet)
//  * \param value
//  *      Register value (See datasheet)
//  */
// void DS1302_writeClockRegister(DS1302_Dev *dev, uint8_t reg, uint8_t value)
// {
//     DS1302_transferBegin(dev);
//     DS1302_writeAddrCmd(dev, (uint8_t)DS1302_CMD_WRITE_CLOCK_REG(reg));
//     DS1302_writeByte(dev, value);
//     DS1302_transferEnd(dev);
// }

// /*!
//  * \brief Read clock register
//  * \param reg
//  *      RTC clock register (See datasheet)
//  * \return
//  *      Register value (See datasheet)
//  */
// uint8_t DS1302_readClockRegister(DS1302_Dev *dev, uint8_t reg)
// {
//     uint8_t retval;

//     DS1302_transferBegin(dev);
//     DS1302_writeAddrCmd(dev, (uint8_t)DS1302_CMD_READ_CLOCK_REG(reg));
//     retval = DS1302_readByte(dev);
//     DS1302_transferEnd(dev);

//     return retval;
// }

// // -------------------------------------------------------------------------------------------------
// // Private functions
// // -------------------------------------------------------------------------------------------------
// /*!
//  * \brief Start RTC transfer
//  */
// void DS1302_transferBegin(DS1302_Dev *dev)
// {
//     gpio_set_level(dev->clkPin, 0);
//     gpio_set_level(dev->ioPin, 0);
//     gpio_set_direction(dev->ioPin, GPIO_MODE_OUTPUT);
//     gpio_set_level(dev->cePin, 1);
// }

// /*!
//  * \brief End RTC transfer
//  */
// void DS1302_transferEnd(DS1302_Dev *dev)
// {
//     gpio_set_level(dev->cePin, 0);
// }

// /*!
//  * \brief Write address/command byte
//  * \param value
//  *      Address/command byte
//  */
// void DS1302_writeAddrCmd(DS1302_Dev *dev, uint8_t value)
// {
//     // Write 8 bits to RTC
//     for (uint8_t i = 0; i < 8; i++) {
//         if (value & (1 << i)) {
//             gpio_set_level(dev->ioPin, 1);
//         } else {
//             gpio_set_level(dev->ioPin, 0);
//         }
//         vTaskDelay(1);
//         gpio_set_level(dev->clkPin, 1);
//         vTaskDelay(1);

//         if ((value & (1 << DS1302_BIT_READ)) && (i == 7)) {
//             gpio_set_direction(dev->ioPin, GPIO_MODE_INPUT);
//         } else {
//             gpio_set_level(dev->clkPin, 0);
//         }
//     }
// }

// /*!
//  * \brief Write byte
//  * \param value
//  *      Data byte
//  */
// void DS1302_writeByte(DS1302_Dev *dev, uint8_t value)
// {
//     // Write 8 bits to RTC
//     for (uint8_t i = 0; i < 8; i++) {
//         if (value & 0x01) {
//             gpio_set_level(dev->ioPin, 1);
//         } else {
//             gpio_set_level(dev->ioPin, 0);
//         }
//         value >>= 1;
//         gpio_set_level(dev->clkPin, 1);
//         vTaskDelay(1);
//         gpio_set_level(dev->clkPin, 0);
//     }
// }

// /*!
//  * \brief Read Byte from RTC
//  * \return
//  *      Data Byte
//  */
// uint8_t DS1302_readByte(DS1302_Dev *dev)
// {
//     uint8_t value = 0;

//     for (uint8_t i = 0; i < 8; i++) {
//         gpio_set_level(dev->clkPin, 1);
//         gpio_set_level(dev->clkPin, 0);
//         vTaskDelay(1);

//         value >>= 1;
//         if (gpio_get_level(dev->ioPin)) {
//             value |= 0x80;
//         } else {
//             value &= ~(0x80);
//         }
//     }

//     return value;
// }

// /*!
//  * \brief Read buffer from DS1302
//  * \param buf
//  *      Buffer
//  * \param len
//  *      Buffer length
//  */
// void DS1302_readBuffer(DS1302_Dev *dev, void *buf, uint8_t len)
// {
//     for (uint8_t i = 0; i < len; i++) {
//         ((uint8_t *)buf)[i] = DS1302_readByte(dev);
//     }
// }

// /*!
//  * \brief BCD to decimal conversion
//  * \param bcd
//  *      BCD encoded value
//  * \return
//  *      Decimal value
//  */
// uint8_t bcdToDec(uint8_t bcd)
// {
// //    return (uint8_t)(10 * ((bcd & 0xF0) >> 4) + (bcd & 0x0F));
//     return (bcd >> 4) * 10 + (bcd & 0x0f);
// }

// /*!
//  * \brief Decimal to BCD conversion
//  * \param dec
//  *      Decimal value
//  * \return
//  *      BCD encoded value
//  */
// uint8_t decToBcd(uint8_t dec)
// {
// //    return (uint8_t)(((dec / 10) << 4) | (dec % 10));
//     return ((dec / 10) << 4) + (dec % 10);
// }
//OLD RTC DS1302

//NEW RTC DS3231


#define CHECK_ARG(ARG) do { if (!ARG) return ESP_ERR_INVALID_ARG; } while (0)

uint8_t bcd2dec(uint8_t val)
{
	return (val >> 4) * 10 + (val & 0x0f);
}

uint8_t dec2bcd(uint8_t val)
{
	return ((val / 10) << 4) + (val % 10);
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
    if(internal_i2c_driver_status != ESP_OK)
    {
        //add diagnostic error
#ifdef I2C_COM_LOGS
        ESP_LOGE(I2C_COM_TAG, "I2C_NUM_1 i2c_dev_init ==>> ESP FAILURE !!!");
#endif
    }else
    {
#ifdef I2C_COM_LOGS
        ESP_LOGI(I2C_COM_TAG, "I2C_NUM_1  i2c_dev_init ==>> ESP OKK ...");
#endif
    }
	return internal_i2c_driver_status;//i2c_dev_init(port, sda_gpio, scl_gpio);
}

esp_err_t ds3231_set_time(i2c_dev_t *dev, struct tm *time)
{
	CHECK_ARG(dev);
	CHECK_ARG(time);

	uint8_t data[7];

	/* time/date data */
	data[0] = dec2bcd(time->tm_sec);
	data[1] = dec2bcd(time->tm_min);
	data[2] = dec2bcd(time->tm_hour);
	/* The week data must be in the range 1 to 7, and to keep the start on the
	 * same day as for tm_wday have it start at 1 on Sunday. */
	data[3] = dec2bcd(time->tm_wday + 1);
	data[4] = dec2bcd(time->tm_mday);
	data[5] = dec2bcd(time->tm_mon + 1);
	data[6] = dec2bcd(time->tm_year - 2000);

	return i2c_dev_write_reg(dev, DS3231_ADDR_TIME, data, 7);
}

esp_err_t ds3231_get_raw_temp(i2c_dev_t *dev, int16_t *temp)
{
	CHECK_ARG(dev);
	CHECK_ARG(temp);

	uint8_t data[2];

	esp_err_t res = i2c_dev_read_reg(dev, DS3231_ADDR_TEMP, data, sizeof(data));
	if (res == ESP_OK)
		*temp = (int16_t)(int8_t)data[0] << 2 | data[1] >> 6;

	return res;
}

esp_err_t ds3231_get_temp_integer(i2c_dev_t *dev, int8_t *temp)
{
	CHECK_ARG(temp);

	int16_t t_int;

	esp_err_t res = ds3231_get_raw_temp(dev, &t_int);
	if (res == ESP_OK)
		*temp = t_int >> 2;

	return res;
}

esp_err_t ds3231_get_temp_float(i2c_dev_t *dev, float *temp)
{
	CHECK_ARG(temp);

	int16_t t_int;

	esp_err_t res = ds3231_get_raw_temp(dev, &t_int);
	if (res == ESP_OK)
		*temp = t_int * 0.25;

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

	/* convert to unix time structure */
	time->tm_sec = bcd2dec(data[0]);
	time->tm_min = bcd2dec(data[1]);
	if (data[2] & DS3231_12HOUR_FLAG)
	{
		/* 12H */
		time->tm_hour = bcd2dec(data[2] & DS3231_12HOUR_MASK) - 1;
		/* AM/PM? */
		if (data[2] & DS3231_PM_FLAG) time->tm_hour += 12;
	}
	else time->tm_hour = bcd2dec(data[2]); /* 24H */
	time->tm_wday = bcd2dec(data[3]) - 1;
	time->tm_mday = bcd2dec(data[4]);
	time->tm_mon  = bcd2dec(data[5] & DS3231_MONTH_MASK) - 1;
	time->tm_year = bcd2dec(data[6]) + 2000;
	time->tm_isdst = 0;

	// apply a time zone (if you are not using localtime on the rtc or you want to check/apply DST)
	//applyTZ(time);

	return ESP_OK;
}

// DS1302_DateTime rtc_set_protocol_date;
// DS1302_DateTime rtc_get_protocol_date;
// DS1302_Dev rtc_protocol_rtc;
bool rtc_init_state;
// i2c_dev_t RTC_DEV;
void RTC_INIT(void)
{
    // rtc_protocol_date.dayMonth = 5; //change with signals on top level
    // rtc_protocol_date.hour     = 3;
    // rtc_protocol_date.month    = 1;
    // rtc_protocol_date.dayWeek  = 1;
    // rtc_protocol_date.year     = 2025;
    // rtc_protocol_date.minute   = 15;
    // rtc_protocol_date.second   = 10;
//     internal_i2c_driver_status = i2c_dev_init(I2C_NUM_1, 4, 15);
//     if(internal_i2c_driver_status != ESP_OK)
//     {
//         //add diagnostic error
// #ifdef I2C_COM_LOGS
//         ESP_LOGE(I2C_COM_TAG, "I2C_NUM_1 i2c_driver_install ==>> ESP FAILURE !!!");
// #endif
//     }else
//     {
// #ifdef I2C_COM_LOGS
//         ESP_LOGI(I2C_COM_TAG, "I2C_NUM_1 i2c_driver_install ==>> ESP OKK ...");
// #endif
//     }
//     esp_err_t 

//     rtc_init_state = DS1302_begin(&rtc_protocol_rtc, GPIO_NUM_15, GPIO_NUM_2, GPIO_NUM_4);
//     if(rtc_init_state == false)
//     {
// #ifdef REAL_TIME_CLOCK_LOGS
//         ESP_LOGE(REAL_TIME_CLOCK_TAG, "rtc device is halted plese check the driver connections!!!");
// #endif
//     }
//     else
//     {
// #ifdef REAL_TIME_CLOCK_LOGS
//         ESP_LOGI(REAL_TIME_CLOCK_TAG, "rtc was initialized successfully...");
// #endif
//     }

//SDA 15 CLK 4
    internal_i2c_driver_status = ds3231_init_desc(&rtc_protocol_rtc, I2C_NUM_1, GPIO_NUM_15, GPIO_NUM_4);
    if(internal_i2c_driver_status != ESP_OK)
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
    // DS1302_setDateTime(&rtc_protocol_rtc, &rtc_protocol_date);//optional change with signals
}


void RTC_SET_DATA(DS1302_DateTime *dateTime)
{
    rtc_time_data.tm_year = (int)dateTime->year;
    rtc_time_data.tm_mon = (int)dateTime->month;
    rtc_time_data.tm_mday = (int)dateTime->dayMonth;
    rtc_time_data.tm_hour = (int)dateTime->hour;
    rtc_time_data.tm_min = (int)dateTime->minute;
    rtc_time_data.tm_sec = (int)dateTime->second;
    internal_i2c_driver_status = ds3231_set_time(&rtc_protocol_rtc, &rtc_time_data);
    if(internal_i2c_driver_status != ESP_OK)
    {
    #ifdef REAL_TIME_CLOCK_LOGS
        ESP_LOGE(REAL_TIME_CLOCK_TAG, "ds3231_set_time!!!");
    #endif
    }
    else
    {
    #ifdef REAL_TIME_CLOCK_LOGS
        ESP_LOGI(REAL_TIME_CLOCK_TAG, "ds3231_set_time OK...");
        ESP_LOGI(REAL_TIME_CLOCK_TAG, "YEAR: %d month: %d day: %d hour: %d minute %d hour: %d",
            rtc_time_data.tm_year,
            rtc_time_data.tm_mon,
            rtc_time_data.tm_mday,
            rtc_time_data.tm_hour,
            rtc_time_data.tm_min,
            rtc_time_data.tm_sec);
    #endif
    }
    // DS1302_setDateTime(&rtc_protocol_rtc, dateTime);
}

DS1302_DateTime *RTC_GET_DATA(void)
{

    internal_i2c_driver_status = ds3231_get_time(&rtc_protocol_rtc, &rtc_time_data);
    rtc_get_protocol_date.year = (uint16_t) rtc_time_data.tm_year;
    rtc_get_protocol_date.month = (uint8_t) rtc_time_data.tm_mon;
    rtc_get_protocol_date.dayMonth = (uint8_t) rtc_time_data.tm_mday;
    rtc_get_protocol_date.hour = (uint8_t) rtc_time_data.tm_hour;
    rtc_get_protocol_date.minute = (uint8_t) rtc_time_data.tm_min;
    rtc_get_protocol_date.second = (uint8_t) rtc_time_data.tm_sec;
    if(internal_i2c_driver_status != ESP_OK)
    {
    #ifdef REAL_TIME_CLOCK_LOGS
        ESP_LOGE(REAL_TIME_CLOCK_TAG, "ds3231_get_time!!!");
    #endif
    }
    else
    {
    #ifdef REAL_TIME_CLOCK_LOGS
        ESP_LOGI(REAL_TIME_CLOCK_TAG, "ds3231_get_time OK...");
        ESP_LOGI(REAL_TIME_CLOCK_TAG, "YEAR: %d month: %d day: %d hour: %d minute %d hour: %d",
            rtc_get_protocol_date.year,
            rtc_get_protocol_date.month,
            rtc_get_protocol_date.dayMonth,
            rtc_get_protocol_date.hour,
            rtc_get_protocol_date.minute,
            rtc_get_protocol_date.second);
    #endif
    
    }


    return &rtc_get_protocol_date;
}
