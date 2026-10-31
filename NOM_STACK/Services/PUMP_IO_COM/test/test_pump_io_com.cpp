#include <gtest/gtest.h>
#include <cstring>
#include <vector>

#include "mock_esp_state.h"
#include "../inc/pump_io_com.h"
#include "../../../Signals/Sensors/inc/sensor_ic_signals.h"

namespace {

std::vector<uint8_t> buildStm32Frame(uint8_t iface, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
    uint8_t csum = static_cast<uint8_t>(FRAME_START_BYTE + iface + d0 + d1 + d2 + d3);
    return {FRAME_START_BYTE, iface, d0, d1, d2, d3, csum, FRAME_END_BYTE};
}

std::vector<uint8_t> buildStm32FloatFrame(uint8_t iface, float val) {
    union {
        float f;
        uint8_t b[4];
    } u;
    u.f = val;
    // bytesToFloat expects big-endian byte order
    return buildStm32Frame(iface, u.b[3], u.b[2], u.b[1], u.b[0]);
}

} // namespace

class PumpIoComTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
        SENSORS_INIT_SIGNALS();
        PUMP_IO_COM_init_state();
    }
};

TEST_F(PumpIoComTest, InitStatePopulatesAll12SafeOffFramesValidly) {
    EXPECT_FALSE(PUMP_IO_COM_is_link_healthy());

    PUMP_IO_COM_EXE();
    // All 12 slots (60 bytes) should be transmitted immediately with valid framing
    EXPECT_EQ(mock_uart_get_tx_len(UART_NUM_2), 60U);
    const uint8_t *tx = mock_uart_get_tx_data(UART_NUM_2);
    for (int i = 0; i < 12; ++i) {
        const uint8_t *slot = &tx[i * 5];
        EXPECT_EQ(slot[0], FRAME_START_BYTE);
        EXPECT_EQ(slot[1], static_cast<uint8_t>(0x30 + i));
        EXPECT_EQ(slot[3], static_cast<uint8_t>(slot[0] + slot[1] + slot[2]));
        EXPECT_EQ(slot[4], FRAME_END_BYTE);
    }
}

TEST_F(PumpIoComTest, StreamParserSyncsAcrossNoiseSplitChunksAndGrowLeds0x37Ambiguity) {
    // Prefix garbage + 0x37 false start + GROW_LEDS ACK (iface 0x37!) + PH float + EC float + Temp float + Distance
    std::vector<uint8_t> stream = {0x99, 0x00, 0x37, 0xFF};

    auto growAck = buildStm32Frame(INTERFACE_GROW_LEDS, 0x00, 0x00, 0x00, TRANSFER_COMP_SUCC);
    auto phFrame = buildStm32FloatFrame(INTERFACE_PH_SENSOR, 2.95f);
    auto ecFrame = buildStm32FloatFrame(INTERFACE_EC_SENSOR, 1.45f);
    auto wtFrame = buildStm32FloatFrame(INTERFACE_TEMPERATURE_SENSOR, 24.5f);
    auto wlFrame = buildStm32Frame(INTERFACE_DISTANCE_SENSOR, 0x00, 0x00, 0x00, 42U);

    stream.insert(stream.end(), growAck.begin(), growAck.end());
    stream.insert(stream.end(), phFrame.begin(), phFrame.end());
    stream.insert(stream.end(), ecFrame.begin(), ecFrame.end());
    stream.insert(stream.end(), wtFrame.begin(), wtFrame.end());
    stream.insert(stream.end(), wlFrame.begin(), wlFrame.end());

    // Feed 3 bytes at a time to exercise split-chunk reassembly
    for (size_t i = 0; i < stream.size(); i += 3) {
        size_t chunk = (i + 3 <= stream.size()) ? 3 : (stream.size() - i);
        PUMP_IO_COM_process_rx_bytes(&stream[i], chunk);
    }

    EXPECT_TRUE(PUMP_IO_COM_is_link_healthy());
    EXPECT_EQ(diagnostics_response[7], PASS); // GROW_LEDS
    EXPECT_FLOAT_EQ(pump_io_get_ph_duty_V_float_data, 2.95f);
    EXPECT_FLOAT_EQ(pump_io_get_ec_duty_V_float_data, 1.45f);
    EXPECT_FLOAT_EQ(pump_io_get_temperature_float_data, 24.5f);
    EXPECT_EQ(pump_io_get_distance_data, 42U);
}

TEST_F(PumpIoComTest, BadChecksumNanFloatAndZeroDistanceAreRejectedCleanly) {
    // Establish initial valid readings
    auto wlValid = buildStm32Frame(INTERFACE_DISTANCE_SENSOR, 0, 0, 0, 55U);
    auto phValid = buildStm32FloatFrame(INTERFACE_PH_SENSOR, 3.01f);
    PUMP_IO_COM_process_rx_bytes(wlValid.data(), wlValid.size());
    PUMP_IO_COM_process_rx_bytes(phValid.data(), phValid.size());
    EXPECT_TRUE(PUMP_IO_COM_is_link_healthy());
    EXPECT_EQ(pump_io_get_distance_data, 55U);

    // Corrupt checksum on PH sensor must not overwrite 3.01f
    auto badCsum = phValid;
    badCsum[6] ^= 0x55; // corrupt checksum byte
    PUMP_IO_COM_process_rx_bytes(badCsum.data(), badCsum.size());
    EXPECT_FLOAT_EQ(pump_io_get_ph_duty_V_float_data, 3.01f);

    // NaN float on PH sensor (0x7FC00000) with valid checksum must be rejected
    auto nanPh = buildStm32Frame(INTERFACE_PH_SENSOR, 0x7F, 0xC0, 0x00, 0x00);
    PUMP_IO_COM_process_rx_bytes(nanPh.data(), nanPh.size());
    EXPECT_FLOAT_EQ(pump_io_get_ph_duty_V_float_data, 3.01f);

    // Zero distance (single glitch) keeps last valid distance, flags diagnostic FAILURE,
    // and if zero-distance persists past watchdog timeout, marks link unhealthy!
    auto zeroDist = buildStm32Frame(INTERFACE_DISTANCE_SENSOR, 0, 0, 0, 0U);
    PUMP_IO_COM_process_rx_bytes(zeroDist.data(), zeroDist.size());
    EXPECT_EQ(pump_io_get_distance_data, 55U);
    EXPECT_EQ(diagnostics_response[11], FAILURE);

    mock_esp_advance_time_us(PUMP_IO_COM_LINK_TIMEOUT_US + 1000LL);
    PUMP_IO_COM_process_rx_bytes(zeroDist.data(), zeroDist.size());
    EXPECT_FALSE(PUMP_IO_COM_is_link_healthy());
}

TEST_F(PumpIoComTest, LinkWatchdogExpiresAfterTimeoutAndRecoversOnFreshFrame) {
    auto wlValid = buildStm32Frame(INTERFACE_DISTANCE_SENSOR, 0, 0, 0, 60U);
    PUMP_IO_COM_process_rx_bytes(wlValid.data(), wlValid.size());
    EXPECT_TRUE(PUMP_IO_COM_is_link_healthy());

    // Advance past 2.5s watchdog
    mock_esp_advance_time_us(PUMP_IO_COM_LINK_TIMEOUT_US + 1000LL);
    EXPECT_FALSE(PUMP_IO_COM_is_link_healthy());

    // Fresh frame restores link health immediately
    PUMP_IO_COM_process_rx_bytes(wlValid.data(), wlValid.size());
    EXPECT_TRUE(PUMP_IO_COM_is_link_healthy());
}
