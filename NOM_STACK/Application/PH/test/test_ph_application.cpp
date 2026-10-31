#include <gtest/gtest.h>
#include <cmath>

#include "mock_esp_state.h"
#include "../inc/ph_application.h"
#include "../../WL/inc/wl_application.h"
#include "../../../Services/PUMP_IO_COM/inc/pump_io_com.h"
#include "../../../Signals/Sensors/inc/sensor_ic_signals.h"

namespace {

void setHealthyLinkAndWaterLevel(uint8_t dist_cm) {
    uint8_t csum = static_cast<uint8_t>(FRAME_START_BYTE + INTERFACE_DISTANCE_SENSOR + dist_cm);
    uint8_t frame[8] = {FRAME_START_BYTE, INTERFACE_DISTANCE_SENSOR, 0, 0, 0, dist_cm, csum, FRAME_END_BYTE};
    PUMP_IO_COM_process_rx_bytes(frame, sizeof(frame));
    signal_data_get_water_level_data = static_cast<float>(dist_cm);
}

} // namespace

class PhApplicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
        SENSORS_INIT_SIGNALS();
        PUMP_IO_COM_init_state();
        PH_RESET_STATE();
        setHealthyLinkAndWaterLevel(40U);
        signal_ctrl_water_level_start_ph_ec_unit = SENSOR_IC_SIGNAL_TRUE;
        signal_ctrl_water_level_stop_ph_ec_unit = SENSOR_IC_SIGNAL_FALSE;
    }
};

TEST_F(PhApplicationTest, AdjustmentVolumeGuardsZeroConcentrationAndExtremePh) {
    EXPECT_DOUBLE_EQ(calculate_adjustment_volume(100.0, 7.0, 6.0, 0.0), 0.0);
    EXPECT_DOUBLE_EQ(calculate_adjustment_volume(0.0, 7.0, 6.0, 1470.0), 0.0);
    EXPECT_DOUBLE_EQ(calculate_adjustment_volume(100.0, NAN, 6.0, 1470.0), 0.0);

    double valid = calculate_adjustment_volume(100.0, 7.0, 6.0, 1470.0);
    EXPECT_TRUE(std::isfinite(valid));
    EXPECT_GT(valid, 0.0);
}

TEST_F(PhApplicationTest, AcidDosingLifecycleAndBufferMeanRemainBounded) {
    // Current pH 7.2 > Target pH 6.0 -> negative diff (-1.2) -> turns ON pH acid pump
    signal_data_get_target_ph_data = 6.0f;
    signal_data_get_ph_data = 7.2f;
    signal_data_get_rtc_data->hour = 12;
    signal_data_get_rtc_data->minute = 0;
    signal_data_get_rtc_data->second = 0;

    PH_EXE_APPLICATION();
    EXPECT_TRUE(ph_increase_get_current_time_assigned);
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_ph_pump, SENSOR_IC_SIGNAL_TRUE);

    // Run buffer accumulator > 250 steps and verify mean does not grow unboundedly
    for (int i = 0; i < 350; ++i) {
        CALCULATE_ADDED_PH_VOL_DIFF_ON_TANK();
    }
    EXPECT_TRUE(std::isfinite(ph_ph_diff_buffer_mean_value));
    EXPECT_GE(ph_ph_diff_buffer_mean_value, -1.5f);
    EXPECT_LE(ph_ph_diff_buffer_mean_value, 0.0f);

    // Critical overflow (<= 23 cm) immediately cuts off pH pump
    setHealthyLinkAndWaterLevel(20U);
    PH_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_ph_pump, SENSOR_IC_SIGNAL_FALSE);

    // Link loss resets pH dosing latch cleanly
    mock_esp_advance_time_us(PUMP_IO_COM_LINK_TIMEOUT_US + 1000LL);
    PH_EXE_APPLICATION();
    EXPECT_FALSE(ph_increase_get_current_time_assigned);
}
