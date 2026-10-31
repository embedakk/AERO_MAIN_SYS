#include <gtest/gtest.h>

#include "mock_esp_state.h"
#include "../inc/real_time_clock.h"

class RealTimeClockTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
        RTC_INIT();
    }
};

TEST_F(RealTimeClockTest, SetAndGetDateTimeRoundTripsOneBasedMonthAndBcdCorrectly) {
    DS1302_DateTime dt = {};
    dt.year = 2026;
    dt.month = 10;     // October (1..12)
    dt.dayMonth = 31;
    dt.dayWeek = 6;
    dt.hour = 23;
    dt.minute = 59;
    dt.second = 45;

    RTC_SET_DATA(&dt);
    DS1302_DateTime *readBack = RTC_GET_DATA();
    ASSERT_NE(readBack, nullptr);
    EXPECT_TRUE(RTC_is_hardware_healthy());
    EXPECT_EQ(readBack->year, 2026U);
    EXPECT_EQ(readBack->month, 10U);
    EXPECT_EQ(readBack->dayMonth, 31U);
    EXPECT_EQ(readBack->hour, 23U);
    EXPECT_EQ(readBack->minute, 59U);
    EXPECT_EQ(readBack->second, 45U);
}

TEST_F(RealTimeClockTest, CorruptedBcdOrBusFailureTriggersMonotonicClockFallback) {
    DS1302_DateTime dt = {50, 59, 23, 1, 28, 2, 2025};
    RTC_SET_DATA(&dt);
    ASSERT_NE(RTC_GET_DATA(), nullptr);
    EXPECT_TRUE(RTC_is_hardware_healthy());

    // Simulate I2C bus failure: clock must advance monotonically via esp_timer_get_time()
    mock_i2c_set_next_result(ESP_FAIL);
    mock_esp_advance_time_us(15000000LL); // +15 seconds across midnight on Feb 28, 2025 -> Mar 1, 00:00:05

    DS1302_DateTime *fallback = RTC_GET_DATA();
    ASSERT_NE(fallback, nullptr);
    EXPECT_FALSE(RTC_is_hardware_healthy());
    EXPECT_EQ(fallback->month, 3U);
    EXPECT_EQ(fallback->dayMonth, 1U);
    EXPECT_EQ(fallback->hour, 0U);
    EXPECT_EQ(fallback->minute, 0U);
    EXPECT_EQ(fallback->second, 5U);

    // Corrupt BCD nibble (0xFF from pulled-up dead bus) must also be rejected cleanly
    mock_i2c_set_next_result(ESP_OK);
    uint8_t all_ff[7] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    mock_i2c_set_regs(DS3231_ADDR_TIME, all_ff, sizeof(all_ff));

    struct tm parsed = {};
    EXPECT_NE(ds3231_get_time(&rtc_protocol_rtc, &parsed), ESP_OK);
}

