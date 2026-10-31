#include <gtest/gtest.h>
#include <cmath>

#include "mock_esp_state.h"
#include "../inc/wl_application.h"
#include "../../../Services/PUMP_IO_COM/inc/pump_io_com.h"
#include "../../../Signals/Sensors/inc/sensor_ic_signals.h"

namespace {

void feedHealthyStm32Distance(uint8_t dist_cm) {
    uint8_t csum = static_cast<uint8_t>(FRAME_START_BYTE + INTERFACE_DISTANCE_SENSOR + dist_cm);
    uint8_t frame[8] = {FRAME_START_BYTE, INTERFACE_DISTANCE_SENSOR, 0, 0, 0, dist_cm, csum, FRAME_END_BYTE};
    PUMP_IO_COM_process_rx_bytes(frame, sizeof(frame));
    signal_data_get_water_level_data = static_cast<float>(dist_cm);
}

} // namespace

class WlApplicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
        SENSORS_INIT_SIGNALS();
        PUMP_IO_COM_init_state();
        WL_RESET_STATE();
    }
};

TEST_F(WlApplicationTest, PercentageCalculationNeverDividesByZeroOrReturnsOutOfRange) {
    EXPECT_FLOAT_EQ(calculate_percentage(120.0f, 250.0f, 120.0f, 120.0f), 0.0f);
    EXPECT_FLOAT_EQ(calculate_percentage(120.0f, 250.0f, 120.0f, 150.0f), 0.0f);
    EXPECT_FLOAT_EQ(calculate_percentage(120.0f, 250.0f, 120.0f, 0.0f), 100.0f);
    EXPECT_FLOAT_EQ(calculate_percentage(120.0f, 250.0f, 120.0f, NAN), 0.0f);
    float mid = calculate_percentage(120.0f, 250.0f, 120.0f, 30.0f);
    EXPECT_GT(mid, 60.0f);
    EXPECT_LE(mid, 100.0f);
}

TEST_F(WlApplicationTest, PowerLossAutoResumeWaitsForHealthyStm32LinkThenActivatesDosing) {
    SIGNALS_set_selected_fruit(1U); // Strawberry
    signal_ctrl_system_start = SENSOR_IC_SIGNAL_TRUE;
    signal_data_get_water_level_data = 30.0f; // Full range, but STM32 link NOT healthy yet

    WL_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_water_level_start_ph_ec_unit, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_system_start, SENSOR_IC_SIGNAL_TRUE); // Preserved for auto-resume!

    // Valid STM32 sensor frame arrives -> auto-resumes!
    feedHealthyStm32Distance(30U);
    WL_EXE_APPLICATION(); // IDLE -> WL_TANK_IS_FULL
    WL_EXE_APPLICATION(); // WL_TANK_IS_FULL -> enables PH/EC dosing unit
    EXPECT_EQ(signal_ctrl_water_level_start_ph_ec_unit, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_EQ(signal_ctrl_water_level_stop_ph_ec_unit, SENSOR_IC_SIGNAL_FALSE);
}

TEST_F(WlApplicationTest, ManualFillAndReleasePreserveCalibrationAndAutoStopAtThresholds) {
    feedHealthyStm32Distance(80U);
    signal_data_set_ph_cal_mid_V = 2.91f; // Custom calibration that MUST NOT be wiped

    // Manual FILL_TANK at 80 cm -> opens input valve
    signal_ctrl_system_fill_the_tank = SENSOR_IC_SIGNAL_TRUE;
    WL_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_main_tank_water_input, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_FLOAT_EQ(signal_data_set_ph_cal_mid_V, 2.91f);

    // Tank reaches high threshold (33 cm) -> auto-stops fill!
    feedHealthyStm32Distance(33U);
    WL_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_main_tank_water_input, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_system_fill_the_tank, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_system_stop, SENSOR_IC_SIGNAL_TRUE);

    // Manual RELEASE_TANK at 50 cm -> opens output valve
    signal_ctrl_system_stop = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_release_the_tank = SENSOR_IC_SIGNAL_TRUE;
    feedHealthyStm32Distance(50U);
    WL_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_main_tank_water_output, SENSOR_IC_SIGNAL_TRUE);

    // Tank reaches empty threshold (105 cm) -> auto-stops drain!
    feedHealthyStm32Distance(105U);
    WL_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_main_tank_water_output, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_system_release_the_tank, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_system_stop, SENSOR_IC_SIGNAL_TRUE);
}

TEST_F(WlApplicationTest, OneShotDeleteResetsDefaultsOnceWithoutEndlessNvsLoop) {
    feedHealthyStm32Distance(50U);
    SIGNALS_set_selected_fruit(3U);
    signal_ctrl_system_delete = SENSOR_IC_SIGNAL_TRUE;

    uint32_t before = mock_nvs_get_commit_count();
    WL_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_system_delete, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_data_set_selected_fruit, 18U);
    EXPECT_EQ(mock_nvs_get_commit_count(), before + 1U);

    // Next cycle must NOT re-trigger NVS write
    WL_EXE_APPLICATION();
    EXPECT_EQ(mock_nvs_get_commit_count(), before + 1U);
}
