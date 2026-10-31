#include <gtest/gtest.h>

#include "mock_esp_state.h"
#include "../inc/led_ctrl_application.h"
#include "../../../Services/PUMP_IO_COM/inc/pump_io_com.h"
#include "../../../Signals/Sensors/inc/sensor_ic_signals.h"

namespace {

void setHealthyLink() {
    uint8_t dist = 40U;
    uint8_t csum = static_cast<uint8_t>(FRAME_START_BYTE + INTERFACE_DISTANCE_SENSOR + dist);
    uint8_t frame[8] = {FRAME_START_BYTE, INTERFACE_DISTANCE_SENSOR, 0, 0, 0, dist, csum, FRAME_END_BYTE};
    PUMP_IO_COM_process_rx_bytes(frame, sizeof(frame));
}

} // namespace

class LedApplicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
        SENSORS_INIT_SIGNALS();
        PUMP_IO_COM_init_state();
    }
};

TEST_F(LedApplicationTest, PhotoperiodWindowKeepsLedsOnDuringDayWindowAndOffThroughoutDarkWindow) {
    setHealthyLink();
    SIGNALS_set_selected_fruit(1U); // Strawberry
    signal_ctrl_system_start = SENSOR_IC_SIGNAL_TRUE;
    signal_data_get_turn_on_time = 14U; // ON from 22:01:01 for 14h -> OFF from 12:01:01 to 22:01:00

    // Inside ON window (23:30:00)
    signal_data_get_rtc_data->hour = 23;
    signal_data_get_rtc_data->minute = 30;
    signal_data_get_rtc_data->second = 0;
    LED_CTRL_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_led_turn_on_led, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_STREQ(signal_data_set_text_led_status, "ON ");
    EXPECT_STREQ(signal_data_set_text_sys_status, "ON ");

    // Inside ON window after midnight rollover (08:00:00)
    signal_data_get_rtc_data->hour = 8;
    LED_CTRL_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_led_turn_on_led, SENSOR_IC_SIGNAL_TRUE);

    // Inside dark window (15:00:00 — NOT just 12:01:01!)
    signal_data_get_rtc_data->hour = 15;
    signal_data_get_rtc_data->minute = 0;
    signal_data_get_rtc_data->second = 0;
    LED_CTRL_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_led_turn_on_led, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_STREQ(signal_data_set_text_led_status, "OFF");

    // System STOP turns LED and both status texts OFF
    signal_ctrl_system_start = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_stop = SENSOR_IC_SIGNAL_TRUE;
    LED_CTRL_EXE_APPLICATION();
    EXPECT_EQ(signal_ctrl_led_turn_on_led, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_STREQ(signal_data_set_text_led_status, "OFF");
    EXPECT_STREQ(signal_data_set_text_sys_status, "OFF");
}
