#include <gtest/gtest.h>

#include "mock_esp_state.h"
#include "../inc/pipe_water_ctrl_app.h"
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

class PipesApplicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
        SENSORS_INIT_SIGNALS();
        PUMP_IO_COM_init_state();
        PIPE_WATER_RESET_STATE();
        setHealthyLinkAndWaterLevel(40U);
        signal_ctrl_water_level_start_ph_ec_unit = SENSOR_IC_SIGNAL_TRUE;
        signal_ctrl_water_level_stop_ph_ec_unit = SENSOR_IC_SIGNAL_FALSE;
        signal_ctrl_pipe_water_enable_from_ph = SENSOR_IC_SIGNAL_TRUE;
        signal_ctrl_pipe_water_enable_from_ec = SENSOR_IC_SIGNAL_TRUE;
    }
};

TEST_F(PipesApplicationTest, OnOffCycleDrainBackGuardAndDryRunCutoffWorkCorrectly) {
    // Starts cleanly even when RTC minute == 0
    signal_data_get_rtc_data->minute = 0;
    PIPE_WATER_CTRL_EXE_APPLICATION();
    EXPECT_TRUE(pumpState);
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_main_pump, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_STREQ(signal_data_set_text_pump_status, "ON ");
    // Input valve forced closed while watering
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_main_tank_water_input, SENSOR_IC_SIGNAL_FALSE);

    // After 3 minutes ON -> turns OFF and holds input valve closed for 2-min drain-back
    signal_data_get_rtc_data->minute = 3;
    signal_ctrl_peripheral_turn_on_main_tank_water_input = SENSOR_IC_SIGNAL_TRUE;
    PIPE_WATER_CTRL_EXE_APPLICATION();
    EXPECT_FALSE(pumpState);
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_main_pump, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_STREQ(signal_data_set_text_pump_status, "OFF");
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_main_tank_water_input, SENSOR_IC_SIGNAL_FALSE);

    // After 7 minutes OFF (minute = 10) -> turns back ON
    signal_data_get_rtc_data->minute = 10;
    PIPE_WATER_CTRL_EXE_APPLICATION();
    EXPECT_TRUE(pumpState);

    // Dry-run protection: empty tank (>= 105 cm) immediately forces main pump OFF
    setHealthyLinkAndWaterLevel(105U);
    PIPE_WATER_CTRL_EXE_APPLICATION();
    EXPECT_FALSE(pumpState);
    EXPECT_EQ(signal_ctrl_peripheral_turn_on_main_pump, SENSOR_IC_SIGNAL_FALSE);
}
