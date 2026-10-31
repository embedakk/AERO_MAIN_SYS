#include <gtest/gtest.h>

#include "mock_esp_state.h"
#include "../inc/wl_sensor.h"
#include "../../../Signals/Sensors/inc/sensor_ic_signals.h"

namespace {

int64_t g_pulse_start_us = 0;
int64_t g_pulse_end_us = 0;

int simulatedEchoCallback(int gpio_num, int64_t now_us) {
    if (gpio_num != WL_SENSOR_ECHO_PIN) {
        return 0;
    }
    mock_esp_advance_time_us(29); // advance monotonic clock on each poll
    now_us = mock_esp_get_time_us();
    return (now_us >= g_pulse_start_us && now_us < g_pulse_end_us) ? 1 : 0;
}

} // namespace

class WlSensorTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
        SENSORS_INIT_SIGNALS();
    }
};

TEST_F(WlSensorTest, UltrasonicMeasureCmComputesExpectedDistanceAndDetectsEchoTimeout) {
    ultrasonic_sensor_t dev = {WL_SENSOR_TRIGGER_PIN, WL_SENSOR_ECHO_PIN};
    WL_SENSOR_ultrasonic_init(&dev);

    // 20 cm -> 20 * 58 = 1160 us echo high pulse
    mock_esp_set_time_us(10000LL);
    g_pulse_start_us = 10200LL;
    g_pulse_end_us   = 10200LL + (20LL * ROUNDTRIP);
    mock_gpio_set_read_callback(simulatedEchoCallback);

    uint32_t cm = 0;
    EXPECT_EQ(WL_SENSOR_ultrasonic_measure_cm(&dev, MAX_DISTANCE_CM, &cm), ESP_OK);
    EXPECT_NEAR(static_cast<double>(cm), 20.0, 1.0);

    // Echo stuck high beyond max_distance * ROUNDTRIP -> must return ESP_ERR_ULTRASONIC_ECHO_TIMEOUT
    mock_esp_set_time_us(50000LL);
    g_pulse_start_us = 50100LL;
    g_pulse_end_us   = 50100LL + 1000000LL;
    EXPECT_EQ(WL_SENSOR_ultrasonic_measure_cm(&dev, 50U, &cm), ESP_ERR_ULTRASONIC_ECHO_TIMEOUT);
}

TEST_F(WlSensorTest, GetDistanceCmReflectsStm32DistanceSignalAndRejectsNullArgs) {
    pump_io_get_distance_data = 47U;
    EXPECT_FLOAT_EQ(WL_SENSOR_GET_DISTANCE_CM(), 47.0f);

    uint32_t out = 0;
    ultrasonic_sensor_t dev = {WL_SENSOR_TRIGGER_PIN, WL_SENSOR_ECHO_PIN};
    EXPECT_EQ(ultrasonic_measure_cm(nullptr, 100, &out), ESP_ERR_INVALID_ARG);
    EXPECT_EQ(ultrasonic_measure_cm(&dev, 100, nullptr), ESP_ERR_INVALID_ARG);
}

