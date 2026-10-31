#include <cmath>
#include <gtest/gtest.h>

#include "../../../Signals/Sensors/inc/sensor_ic_signals.h"
#include "../inc/ph_sensor.h"
#include "mock_esp_state.h"

class PhSensorTest : public ::testing::Test {
protected:
  void SetUp() override {
    mock_esp_reset_all();
    SENSORS_INIT_SIGNALS();
    PH_SENSOR_INIT();
  }
};

TEST_F(PhSensorTest, LinearRegressionReadsExpectedPhAtMidCalibrationVoltage) {
  // Calibrate with collinear Nernstian 3-point voltages: 3.317V -> 4.01,
  // 2.804V -> 6.86, 2.386V -> 9.18
  signal_data_set_ph_cal_low_V = 3.317f;
  signal_data_set_ph_cal_mid_V = 2.804f;
  signal_data_set_ph_cal_high_V = 2.386f;
  PH_SENSOR_INIT();

  // STM32 reports PWM duty in % where voltage = duty * 0.05f (56.08% * 0.05
  // = 2.804V -> 6.86 pH)
  pump_io_get_ph_duty_V_float_data = 2.804f / 0.05f;
  signal_data_get_water_temperature_data = 25.0f;

  float ph = 0.0f;
  for (int i = 0; i < 300; ++i) {
    ph = PH_SENSOR_READ();
  }
  EXPECT_TRUE(std::isfinite(ph));
  EXPECT_NEAR(ph, 6.86f, 0.05f);
}

TEST_F(PhSensorTest,
       CollinearCalibrationVoltagesDoNotProduceNanOrDivideByZero) {
  // Identical voltages across all 3 calibration points -> denom == 0
  signal_data_set_ph_cal_low_V = 3.00f;
  signal_data_set_ph_cal_mid_V = 3.00f;
  signal_data_set_ph_cal_high_V = 3.00f;
  PH_SENSOR_INIT();

  pump_io_get_ph_duty_V_float_data = 2.50f / 0.05f;
  float ph = PH_SENSOR_READ();
  EXPECT_TRUE(std::isfinite(ph));
  EXPECT_GE(ph, 0.0f);
  EXPECT_LE(ph, 14.0f);
}

TEST_F(PhSensorTest,
       DisconnectedTemperatureSensorAndNanAdcDutyRemainFiniteAndBounded) {
  // -127 C DS18B20 disconnect error must fall back to 25 C (identical reading
  // to 25 C!)
  signal_data_get_water_temperature_data = 25.0f;
  pump_io_get_ph_duty_V_float_data = 3.01f / 0.05f;
  float ph_at_25c = 0.0f;
  for (int i = 0; i < 300; ++i) {
    ph_at_25c = PH_SENSOR_READ();
  }

  PH_SENSOR_INIT();
  signal_data_get_water_temperature_data = -127.0f;
  float ph_disconnect = 0.0f;
  for (int i = 0; i < 300; ++i) {
    ph_disconnect = PH_SENSOR_READ();
  }
  EXPECT_TRUE(std::isfinite(ph_disconnect));
  EXPECT_FLOAT_EQ(ph_disconnect, ph_at_25c);

  // NaN duty from UART/ADC must also stay finite
  pump_io_get_ph_duty_V_float_data = NAN;
  float ph_nan = PH_SENSOR_READ();
  EXPECT_TRUE(std::isfinite(ph_nan));
}