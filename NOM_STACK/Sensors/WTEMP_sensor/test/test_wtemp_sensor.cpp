#include <gtest/gtest.h>
#include <cmath>

#include "mock_esp_state.h"
#include "../inc/wtemp_sensor.h"
#include "../../../Signals/Sensors/inc/sensor_ic_signals.h"

class WtempSensorTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
        SENSORS_INIT_SIGNALS();
        WTEMP_SENSOR_INIT();
    }
};

TEST_F(WtempSensorTest, Signed12BitScratchpadCalculationHandlesPositiveAndNegativeTemps) {
    DeviceAddress rom = {0x28, 0, 0, 0, 0, 0, 0, 0}; // DS18B20 family 0x28

    // +25.0 C -> 0x0190 (400 * 1/16 C -> raw = 400 * 8 = 3200 in 1/128 C units)
    ScratchPad sp_pos = {0x90, 0x01, 0, 0, TEMP_12_BIT, 0, 0, 0x10, 0};
    int16_t raw_pos = calculateTemperature(&rom, sp_pos);
    EXPECT_FLOAT_EQ(static_cast<float>(raw_pos) / 128.0f, 25.0f);

    // -10.5 C -> -168 * 1/16 C = 0xFF58 -> raw = -1344 -> -10.5 C
    ScratchPad sp_neg = {0x58, 0xFF, 0, 0, TEMP_12_BIT, 0, 0, 0x10, 0};
    int16_t raw_neg = calculateTemperature(&rom, sp_neg);
    EXPECT_FLOAT_EQ(static_cast<float>(raw_neg) / 128.0f, -10.5f);
}

TEST_F(WtempSensorTest, GetDataReturnsValidStm32TempAndClampsDisconnectOrNanTo25C) {
    pump_io_get_temperature_float_data = 22.75f;
    EXPECT_FLOAT_EQ(WTEMP_SENSOR_GET_DATA(), 22.75f);

    // -127 C (disconnected DS18B20) must clamp to safe 25.0 C fallback
    pump_io_get_temperature_float_data = -127.0f;
    EXPECT_FLOAT_EQ(WTEMP_SENSOR_GET_DATA(), 25.0f);

    // > 85 C power-on reset / fault or NaN must also fall back to 25.0 C
    pump_io_get_temperature_float_data = 125.0f;
    EXPECT_FLOAT_EQ(WTEMP_SENSOR_GET_DATA(), 25.0f);

    pump_io_get_temperature_float_data = NAN;
    EXPECT_FLOAT_EQ(WTEMP_SENSOR_GET_DATA(), 25.0f);
}

