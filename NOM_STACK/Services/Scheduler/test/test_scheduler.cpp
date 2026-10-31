#include <gtest/gtest.h>
#include <vector>

#include "../../../Services/PUMP_IO_COM/inc/pump_io_com.h"
#include "../../../Signals/Sensors/inc/sensor_ic_signals.h"
#include "../../../UI/DWIN_protocol/inc/dwin_protocol.h"
#include "../inc/scheduler.h"
#include "mock_esp_state.h"

namespace {

std::vector<uint8_t> makeDwinVp16(uint16_t vp, uint16_t val) {
  return {0x5A,
          0xA5,
          0x06,
          0x83,
          static_cast<uint8_t>(vp >> 8),
          static_cast<uint8_t>(vp & 0xFF),
          0x01,
          static_cast<uint8_t>(val >> 8),
          static_cast<uint8_t>(val & 0xFF)};
}

std::vector<uint8_t> makeStm32Float(uint8_t iface, float val) {
  union {
    float f;
    uint8_t b[4];
  } u;
  u.f = val;
  uint8_t csum =
      static_cast<uint8_t>(FRAME_START_BYTE + iface +
                           u.b[0]); // legacy STM32 start + iface + low byte
  return {FRAME_START_BYTE, iface,  u.b[3], u.b[2],
          u.b[1],           u.b[0], csum,   FRAME_END_BYTE};
}

std::vector<uint8_t> makeStm32Dist(uint8_t cm) {
  uint8_t csum =
      static_cast<uint8_t>(FRAME_START_BYTE + INTERFACE_DISTANCE_SENSOR + cm);
  return {FRAME_START_BYTE, INTERFACE_DISTANCE_SENSOR, 0, 0, 0, cm, csum,
          FRAME_END_BYTE};
}

void pumpSensorOversample(int steps = 340) {
  for (int i = 0; i < steps; ++i) {
    scheduler_step_1ms();
  }
}

void runFullSchedulerCycle() {
  scheduler_step_1ms();
  scheduler_step_3ms();
  scheduler_step_5ms();
  scheduler_step_10ms();
  scheduler_step_15ms();
  scheduler_step_20ms();
  scheduler_step_25ms();
}

} // namespace

class SchedulerIntegrationTest : public ::testing::Test {
protected:
  void SetUp() override { mock_esp_reset_all(); }
};

TEST_F(SchedulerIntegrationTest, SpawnsPersistentTasksOnceWithoutDynamicChurn) {
  SCHEDULER_INIT();
  uint32_t after_init = mock_esp_get_task_create_count();
  EXPECT_EQ(after_init, 2U); // 2 UART RX tasks

  SCHEDULER_MAIN();
  EXPECT_EQ(mock_esp_get_task_create_count(),
            after_init + 7U); // 7 persistent periodic tasks

  // Calling SCHEDULER_MAIN again is idempotent (no duplicate tasks)
  SCHEDULER_MAIN();
  EXPECT_EQ(mock_esp_get_task_create_count(), after_init + 7U);
}

TEST_F(SchedulerIntegrationTest,
       ClosedLoopDwinAndStm32ProtocolEndToEndHardwareSafety) {
  SCHEDULER_INIT();
  SCHEDULER_MAIN();

  // 1. Select Strawberry (VP 0x5510) + press START (VP 0x5100) on DWIN panel
  auto selCrop = makeDwinVp16(UI_DWIN_READ_EDIT_STRAWBERRY_ADDRESS, 1);
  auto cmdStart = makeDwinVp16(UI_DWIN_READ_SETTING_SYS_START_ADDRESS, 1);
  parse_received_byte(selCrop.size(), selCrop.data());
  parse_received_byte(cmdStart.size(), cmdStart.data());

  // Without STM32 frames yet, full scheduler cycle keeps ALL pumps/valves
  // Safe-Off
  runFullSchedulerCycle();
  EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[0][2], COMMAND_EC_UP_OFF);
  EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[1][2], COMMAND_PH_UP_OFF);
  EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[4][2],
            COMMAND_OFF_OR_DOWN); // IN valve OFF
  EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[6][2],
            COMMAND_OFF_OR_DOWN); // Main pump OFF

  // 2. Feed valid STM32 frames: Tank full (30 cm), pH high (2.75V = 55% duty ->
  // pH > 6.5 -> needs acid), EC low (1.0V = 20% duty -> needs EC)
  auto dist30 = makeStm32Dist(30U);
  auto phHigh = makeStm32Float(INTERFACE_PH_SENSOR, 2.75f / 0.05f);
  auto ecLow = makeStm32Float(INTERFACE_EC_SENSOR, 1.00f / 0.05f);
  auto temp24 = makeStm32Float(INTERFACE_TEMPERATURE_SENSOR, 24.0f);

  PUMP_IO_COM_process_rx_bytes(dist30.data(), dist30.size());
  PUMP_IO_COM_process_rx_bytes(phHigh.data(), phHigh.size());
  PUMP_IO_COM_process_rx_bytes(ecLow.data(), ecLow.size());
  PUMP_IO_COM_process_rx_bytes(temp24.data(), temp24.size());

  // First cycle turns ON circulation/LEDs (triggering 50-sample EMI blanking),
  // then oversample settles pH/EC
  runFullSchedulerCycle();
  pumpSensorOversample(340);
  for (int cycle = 0; cycle < 6; ++cycle) {
    PUMP_IO_COM_process_rx_bytes(dist30.data(), dist30.size());
    runFullSchedulerCycle();
  }

  // Dosing pumps MUST now be forwarded to STM32 TX slots (fixed commented-out
  // NORMAL_OP_SET_PH_EC_PUMPS bug!)
  EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[0][2],
            COMMAND_ON_OR_UP); // EC pump ON
  EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[1][2],
            COMMAND_ON_OR_UP); // PH pump ON

  // Verify pH & EC sensors do NOT freeze while dosing is active!
  float ph_before = signal_data_get_ph_data;
  auto phMid = makeStm32Float(INTERFACE_PH_SENSOR, 3.05f / 0.05f);
  PUMP_IO_COM_process_rx_bytes(dist30.data(), dist30.size());
  PUMP_IO_COM_process_rx_bytes(phMid.data(), phMid.size());
  pumpSensorOversample(340);
  runFullSchedulerCycle();
  EXPECT_LT(signal_data_get_ph_data, ph_before);

  // 3. Simulate STM32 cable unplug (advance past 2.5s link watchdog) ->
  // immediate Safe-Off!
  mock_esp_advance_time_us(PUMP_IO_COM_LINK_TIMEOUT_US + 50000LL);
  runFullSchedulerCycle();
  runFullSchedulerCycle();
  EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[0][2], COMMAND_EC_UP_OFF);
  EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[1][2], COMMAND_PH_UP_OFF);
  EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[3][2],
            COMMAND_OFF_OR_DOWN); // Mixer OFF

  // 4. Send DWIN STOP command -> stays cleanly stopped even after STM32 link
  // returns
  auto cmdStop = makeDwinVp16(UI_DWIN_READ_SETTING_SYS_STOP_ADDRESS, 1);
  parse_received_byte(cmdStop.size(), cmdStop.data());
  PUMP_IO_COM_process_rx_bytes(dist30.data(), dist30.size());
  runFullSchedulerCycle();
  EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[0][2], COMMAND_EC_UP_OFF);
  EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[1][2], COMMAND_PH_UP_OFF);
  EXPECT_EQ(mock_esp_get_mutex_lock_depth(), 0U);
}
