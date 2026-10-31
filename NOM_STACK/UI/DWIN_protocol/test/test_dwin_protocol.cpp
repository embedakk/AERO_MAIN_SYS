#include <gtest/gtest.h>
#include <cstring>
#include <vector>

#include "mock_esp_state.h"
#include "../inc/dwin_protocol.h"
#include "../../../Signals/Sensors/inc/sensor_ic_signals.h"

namespace {

// DWIN 2-byte VP upload frame: 5A A5 06 83 <addr_hi> <addr_lo> 01 <val_hi> <val_lo>
std::vector<uint8_t> dwinVp16(uint16_t vp, uint16_t val) {
    return {
        0x5A, 0xA5, 0x06, 0x83,
        static_cast<uint8_t>(vp >> 8), static_cast<uint8_t>(vp & 0xFF),
        0x01,
        static_cast<uint8_t>(val >> 8), static_cast<uint8_t>(val & 0xFF)
    };
}

// DWIN 4-byte VP upload frame: 5A A5 08 83 <addr_hi> <addr_lo> 02 <b3> <b2> <b1> <b0>
std::vector<uint8_t> dwinVp32(uint16_t vp, uint32_t val) {
    return {
        0x5A, 0xA5, 0x08, 0x83,
        static_cast<uint8_t>(vp >> 8), static_cast<uint8_t>(vp & 0xFF),
        0x02,
        static_cast<uint8_t>((val >> 24) & 0xFF),
        static_cast<uint8_t>((val >> 16) & 0xFF),
        static_cast<uint8_t>((val >> 8) & 0xFF),
        static_cast<uint8_t>(val & 0xFF)
    };
}

// DWIN 0x82 write ACK frame: 5A A5 03 82 4F 4B
std::vector<uint8_t> dwinWriteAck() {
    return {0x5A, 0xA5, 0x03, 0x82, 0x4F, 0x4B};
}

} // namespace

class DwinProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
        SENSORS_INIT_SIGNALS();
        UI_reset_parser();
    }
};

TEST_F(DwinProtocolTest, ConcatenatedAckAndTouchFramesParseDeterministically) {
    // Concatenate: [0x82 ACK] + [Select Strawberry 0x5510] + [0x82 ACK] + [START 0x5100]
    std::vector<uint8_t> stream;
    auto ack = dwinWriteAck();
    auto selStrawberry = dwinVp16(UI_DWIN_READ_EDIT_STRAWBERRY_ADDRESS, 1);
    auto cmdStart = dwinVp16(UI_DWIN_READ_SETTING_SYS_START_ADDRESS, 1);

    stream.insert(stream.end(), ack.begin(), ack.end());
    stream.insert(stream.end(), selStrawberry.begin(), selStrawberry.end());
    stream.insert(stream.end(), ack.begin(), ack.end());
    stream.insert(stream.end(), cmdStart.begin(), cmdStart.end());

    // Feed 2 bytes at a time to test split + concatenated stream reassembly
    for (size_t i = 0; i < stream.size(); i += 2) {
        size_t chunk = (i + 2 <= stream.size()) ? 2 : (stream.size() - i);
        parse_received_byte(chunk, &stream[i]);
    }

    EXPECT_EQ(signal_data_set_selected_fruit, 1U); // STRAWBERRY
    EXPECT_STREQ(signal_data_set_text_selected_fruit, "STRAWBERRY     ");
    EXPECT_EQ(signal_ctrl_system_start, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_EQ(signal_ctrl_system_stop, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_data_get_system_start_stop_status, 1U);
}

TEST_F(DwinProtocolTest, StartAndContinueRejectedSafelyWhenNoCropSelected) {
    // Default selected_fruit == 18 (NO SELECTION)
    auto cmdStart = dwinVp16(UI_DWIN_READ_SETTING_SYS_START_ADDRESS, 1);
    parse_received_byte(cmdStart.size(), cmdStart.data());
    EXPECT_EQ(signal_ctrl_system_start, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_system_stop, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_EQ(signal_data_get_system_start_stop_status, 0U);

    auto cmdCont = dwinVp16(UI_DWIN_READ_SETTING_SYS_CONTINUE_ADDRESS, 1);
    parse_received_byte(cmdCont.size(), cmdCont.data());
    EXPECT_EQ(signal_ctrl_system_start, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_system_continue, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_system_stop, SENSOR_IC_SIGNAL_TRUE);
}

TEST_F(DwinProtocolTest, ContinueStopDeleteFillReleaseTransitionsAreMutuallyExclusive) {
    SIGNALS_set_selected_fruit(UI_LETTUCE_ADDRESS);

    // CONTINUE sets start=TRUE, continue=TRUE, stop=FALSE, status=2
    auto cmdCont = dwinVp16(UI_DWIN_READ_SETTING_SYS_CONTINUE_ADDRESS, 1);
    parse_received_byte(cmdCont.size(), cmdCont.data());
    EXPECT_EQ(signal_ctrl_system_start, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_EQ(signal_ctrl_system_continue, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_EQ(signal_ctrl_system_stop, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_data_get_system_start_stop_status, 2U);

    // STOP clears start & continue
    auto cmdStop = dwinVp16(UI_DWIN_READ_SETTING_SYS_STOP_ADDRESS, 1);
    parse_received_byte(cmdStop.size(), cmdStop.data());
    EXPECT_EQ(signal_ctrl_system_start, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_system_continue, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_system_stop, SENSOR_IC_SIGNAL_TRUE);

    // FILL_TANK sets fill=TRUE, release=FALSE, stop=FALSE
    auto cmdFill = dwinVp16(UI_DWIN_READ_SETTING_FILL_TANK_ADDRESS, 1);
    parse_received_byte(cmdFill.size(), cmdFill.data());
    EXPECT_EQ(signal_ctrl_system_fill_the_tank, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_EQ(signal_ctrl_system_release_the_tank, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_system_stop, SENSOR_IC_SIGNAL_FALSE);

    // RELEASE_TANK sets release=TRUE, fill=FALSE
    auto cmdRel = dwinVp16(UI_DWIN_READ_SETTING_RELEASE_TANK_ADDRESS, 1);
    parse_received_byte(cmdRel.size(), cmdRel.data());
    EXPECT_EQ(signal_ctrl_system_release_the_tank, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_EQ(signal_ctrl_system_fill_the_tank, SENSOR_IC_SIGNAL_FALSE);

    // DELETE sets delete=TRUE
    auto cmdDel = dwinVp16(UI_DWIN_READ_SETTING_SYS_DELETE_ADDRESS, 1);
    parse_received_byte(cmdDel.size(), cmdDel.data());
    EXPECT_EQ(signal_ctrl_system_delete, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_EQ(signal_ctrl_system_release_the_tank, SENSOR_IC_SIGNAL_FALSE);
}

TEST_F(DwinProtocolTest, PartialRtcEditPreservesUneditedDateFieldsAndClampsConfigs) {
    signal_data_get_rtc_data->year = 2026;
    signal_data_get_rtc_data->month = 10;
    signal_data_get_rtc_data->dayMonth = 31;
    signal_data_get_rtc_data->hour = 16;
    signal_data_get_rtc_data->minute = 10;
    signal_data_get_rtc_data->second = 45;

    // Edit ONLY minute -> year/month/day/hour must be preserved from signal_data_get_rtc_data
    auto editMin = dwinVp16(UI_DWIN_READ_RTC_MINUTE_ADDRESS, 55);
    parse_received_byte(editMin.size(), editMin.data());
    EXPECT_EQ(signal_ctrl_rtc_activate_set_rtc, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_EQ(signal_data_set_rtc_data->year, 2026U);
    EXPECT_EQ(signal_data_set_rtc_data->month, 10U);
    EXPECT_EQ(signal_data_set_rtc_data->dayMonth, 31U);
    EXPECT_EQ(signal_data_set_rtc_data->hour, 16U);
    EXPECT_EQ(signal_data_set_rtc_data->minute, 55U);

    // Zero config uploads must be clamped to safe minimums (>= 1)
    auto zeroTank = dwinVp16(UI_DWIN_READ_CONFIG_TANK_CAP_ADDRESS, 0);
    auto zeroDose = dwinVp16(UI_DWIN_READ_CONFIG_DOSING_INT_ADDRESS, 0);
    auto zeroPhSol = dwinVp32(UI_DWIN_READ_CONFIG_PH_SOLUTION_ADDRESS, 0);
    auto zeroEcSol = dwinVp32(UI_DWIN_READ_CONFIG_EC_SOLUTION_ADDRESS, 0);
    parse_received_byte(zeroTank.size(), zeroTank.data());
    parse_received_byte(zeroDose.size(), zeroDose.data());
    parse_received_byte(zeroPhSol.size(), zeroPhSol.data());
    parse_received_byte(zeroEcSol.size(), zeroEcSol.data());

    EXPECT_GE(signal_data_get_tank_capacity, 1U);
    EXPECT_GE(signal_data_get_dosing_intensity, 1U);
    EXPECT_GE(signal_data_get_ph_concentration_data, 1UL);
    EXPECT_GE(signal_data_get_ec_concentration_data, 1UL);

    // Unknown VP address must NOT corrupt selected_fruit
    SIGNALS_set_selected_fruit(UI_TOMATO_ADDRESS);
    auto unknownVp = dwinVp16(0x5999, 1);
    parse_received_byte(unknownVp.size(), unknownVp.data());
    EXPECT_EQ(signal_data_set_selected_fruit, static_cast<uint16_t>(UI_TOMATO_ADDRESS));
}

TEST_F(DwinProtocolTest, TextCommandFormattingNeverReadsPastBufferAndPadsSpaces) {
    uint8_t cmd[21] = {0x5A, 0xA5, 0x06, 0x82, 0x10, 0x20};
    char status[4] = {'O', 'N', ' ', '\0'};
    createDwinTextCommand(status, cmd);
    EXPECT_EQ(cmd[6], 'O');
    EXPECT_EQ(cmd[7], 'N');
    EXPECT_EQ(cmd[8], ' ');

    UI_EXE();
    UI_DATA_OUT();
    EXPECT_GT(mock_uart_get_tx_len(UART_NUM), 0U);
}

