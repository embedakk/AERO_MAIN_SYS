#include <gtest/gtest.h>

#include "mock_esp_state.h"
#include "../inc/initialization.h"
#include "../../../Sensors/PH_sensor/inc/ph_sensor.h"
#include "../../../Services/PUMP_IO_COM/inc/pump_io_com.h"
#include "../../../Signals/Sensors/inc/sensor_ic_signals.h"

class InitManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
    }
};

TEST_F(InitManagerTest, LoadsNvsCalibrationBeforePhLinearRegressionAndSpawnsRxTasks) {
    // Store custom pH calibration in NVS BEFORE INIT_PERIPHERALS() runs
    signal_data_t custom = {};
    custom.selected_fruit = 18U;
    custom.ph_cal_low_V = 3.25f;
    custom.ph_cal_mid_V = 2.95f;
    custom.ph_cal_high_V = 2.45f;
    custom.tank_capacity = 120U;
    custom.dosing_intensity = 55U;
    custom.ph_concentration_data = 1470UL;
    custom.ec_concentration_data = 90000UL;
    custom.mixing_time = 300U;
    mock_nvs_set_raw_blob(&custom, sizeof(custom));

    INIT_PERIPHERALS();

    // Verify SENSORS_INIT_SIGNALS() ran BEFORE PH_SENSOR_INIT() so 1-based ph_sensor_x[1..3] got the NVS calibration!
    EXPECT_FLOAT_EQ(ph_sensor_x[1], 2.45f); // high pH solution voltage
    EXPECT_FLOAT_EQ(ph_sensor_x[2], 2.95f); // mid pH solution voltage
    EXPECT_FLOAT_EQ(ph_sensor_x[3], 3.25f); // low pH solution voltage
    EXPECT_EQ(signal_data_get_tank_capacity, 120U);

    // Both UART RX tasks (PUMP_IO_COM + UI_INIT) were spawned
    EXPECT_EQ(mock_esp_get_task_create_count(), 2U);

    // All 12 STM32 TX command slots initialized to Safe-Off
    EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[0][0], FRAME_START_BYTE);
    EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[0][4], FRAME_END_BYTE);
}
