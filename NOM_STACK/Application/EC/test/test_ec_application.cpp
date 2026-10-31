#include <gtest/gtest.h>
#include <cmath>

#include "mock_esp_state.h"
#include "../inc/ec_application.h"
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

class EcApplicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
        SENSORS_INIT_SIGNALS();
        PUMP_IO_COM_init_state();
        EC_RESET_STATE();
        setHealthyLinkAndWaterLevel(40U);
        signal_ctrl_water_level_start_ph_ec_unit = SENSOR_IC_SIGNAL_TRUE;
        signal_ctrl_water_level_stop_ph_ec_unit = SENSOR_IC_SIGNAL_FALSE;
    }
};

TEST_F(EcApplicationTest, SolutionVolumeGuardsZeroOrLowerStockConcentration) {
    // Valid stock concentration (90000 uS/cm) vs current 1500 uS/cm, target 2200 uS/cm
    float vol = calculate_solution_volume(2200.0f, 1500.0f, 90000.0f, 100.0f);
    EXPECT_TRUE(std::isfinite(vol));
    EXPECT_GT(vol, 0.0f);

    // Equal or lower stock concentration when raising EC must return 0 instead of Inf/negative
    EXPECT_FLOAT_EQ(calculate_solution_volume(2200.0f, 1500.0f, 1500.0f, 100.0f), 0.0f);
    EXPECT_FLOAT_EQ(calculate_solution_volume(2200.0f, 1500.0f, 500.0f, 100.0f), 0.0f);
}

TEST_F(EcApplicationTest, PositiveDosingMixingAndEarlyCutoffLifecycle) {
    signal_data_get_target_ec_data = 2200.0f;
    signal_data_get_ec_data = 1500.0f;
    signal_data_get_rtc_data->hour = 10;
    signal_data_get_rtc_data->minute = 0;
    signal_data_get_rtc_data->second = 0;

    EC_EXE_APPLICATION();
    EXPECT_TRUE(ec_increase_get_current_time_assigned);
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_ec_pump, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_EQ(signal_ctrl_mixer_motors_turn_on, SENSOR_IC_SIGNAL_TRUE);

    // Early target reach stops EC pump immediately during dosing window
    signal_data_get_ec_data = 2150.0f; // diff = +50 uS/cm <= 100 uS/cm tolerance
    signal_data_get_rtc_data->second = 10;
    EC_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_ec_pump, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_mixer_motors_turn_on, SENSOR_IC_SIGNAL_TRUE);

    // Advance past dosing + mixing window -> completes cycle and enables pipe water
    int finish_sec = required_time_for_ec_dosing + signal_data_get_mixing_time + 5;
    DS1302_DateTime end_dt = ec_secondsToTime((10 * 3600) + finish_sec);
    signal_data_get_rtc_data->hour = end_dt.hour;
    signal_data_get_rtc_data->minute = end_dt.minute;
    signal_data_get_rtc_data->second = end_dt.second;

    EC_EXE_APPLICATION();
    EXPECT_FALSE(ec_increase_get_current_time_assigned);

    EC_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_pipe_water_enable_from_ec, SENSOR_IC_SIGNAL_TRUE);
}

TEST_F(EcApplicationTest, CriticalOrEmptyWaterLevelAndStopImmediatelyShutOffEcPump) {
    signal_data_get_target_ec_data = 2200.0f;
    signal_data_get_ec_data = 1000.0f;
    EC_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_ec_pump, SENSOR_IC_SIGNAL_TRUE);

    // Empty water level (>= 105 cm) cuts off pump immediately (fixed impossible && bug)
    setHealthyLinkAndWaterLevel(110U);
    EC_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_ec_pump, SENSOR_IC_SIGNAL_FALSE);

    // Re-enable at normal level, then stop unit -> resets timer latch
    setHealthyLinkAndWaterLevel(40U);
    EC_RESET_STATE();
    EC_EXE_APPLICATION();
    EXPECT_TRUE(ec_increase_get_current_time_assigned);

    signal_ctrl_water_level_stop_ph_ec_unit = SENSOR_IC_SIGNAL_TRUE;
    EC_EXE_APPLICATION();
    EXPECT_FALSE(ec_increase_get_current_time_assigned);
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_ec_pump, SENSOR_IC_SIGNAL_FALSE);
}
