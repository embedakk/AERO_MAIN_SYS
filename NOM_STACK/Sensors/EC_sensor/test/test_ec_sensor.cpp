#include <gtest/gtest.h>
#include <cmath>

#include "mock_esp_state.h"
#include "../inc/ec_sensor.h"
#include "../../../Signals/Sensors/inc/sensor_ic_signals.h"

class EcSensorTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
        SENSORS_INIT_SIGNALS();
        EC_SENSOR_INIT();
    }
};

TEST_F(EcSensorTest, ConvertsVoltageToMicroSiemensPerCmMonotonically) {
    signal_data_get_water_temperature_data = 25.0f;

    pump_io_get_ec_duty_V_float_data = 0.0f;
    float ec0 = 0.0f;
    for (int i = 0; i < 55; ++i) {
        ec0 = EC_SENSOR_READ();
    }
    EXPECT_FLOAT_EQ(ec0, 0.0f);

    // 30% duty * 0.05 = 1.50V -> ~580 uS/cm (2 full 50-sample windows so previous buffer is flushed)
    pump_io_get_ec_duty_V_float_data = 1.5f / 0.05f;
    float ec1 = 0.0f;
    for (int i = 0; i < 105; ++i) {
        ec1 = EC_SENSOR_READ();
    }
    EXPECT_GT(ec1, 500.0f); // uS/cm scale

    // 50% duty * 0.05 = 2.50V -> ~1313 uS/cm
    pump_io_get_ec_duty_V_float_data = 2.5f / 0.05f;
    float ec2 = 0.0f;
    for (int i = 0; i < 105; ++i) {
        ec2 = EC_SENSOR_READ();
    }
    EXPECT_GT(ec2, 1200.0f);
    EXPECT_GT(ec2, ec1);
}

TEST_F(EcSensorTest, DisconnectedWaterTempUses25CFallbackWithoutNegativeEc) {
    // Disconnected -127 C DS18B20 temperature must fall back cleanly to 25 C
    signal_data_get_water_temperature_data = -127.0f;
    pump_io_get_ec_duty_V_float_data = 1.5f / 0.05f;

    float ec = 0.0f;
    for (int i = 0; i < 55; ++i) {
        ec = EC_SENSOR_READ();
    }
    EXPECT_TRUE(std::isfinite(ec));
    EXPECT_GT(ec, 500.0f);

    // Negative or NaN ADC duty clamps cleanly to >= 0
    pump_io_get_ec_duty_V_float_data = NAN;
    float ec_nan = 0.0f;
    for (int i = 0; i < 55; ++i) {
        ec_nan = EC_SENSOR_READ();
    }
    EXPECT_TRUE(std::isfinite(ec_nan));
    EXPECT_GE(ec_nan, 0.0f);
}
