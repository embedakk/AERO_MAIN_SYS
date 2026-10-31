#include <gtest/gtest.h>

#include "mock_esp_state.h"
#include "../inc/normal_op.h"
#include "../../../Services/PUMP_IO_COM/inc/pump_io_com.h"
#include "../../../Sensors/PH_sensor/inc/ph_sensor.h"
#include "../../../Sensors/EC_sensor/inc/ec_sensor.h"
#include "../../../Signals/Sensors/inc/sensor_ic_signals.h"

class NormalOpTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
        SENSORS_INIT_SIGNALS();
        PUMP_IO_COM_init_state();
        PH_SENSOR_INIT();
        EC_SENSOR_INIT();
    }
};

TEST_F(NormalOpTest, ActuatorWrappersPopulateExpectedStm32CommandSlots) {
    NORMAL_OP_SET_PH_EC_PUMPS(true, true);
    EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[0][2], COMMAND_ON_OR_UP); // EC pump ON
    EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[1][2], COMMAND_ON_OR_UP); // PH pump ON

    NORMAL_OP_SET_PH_EC_PUMPS(false, false);
    EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[0][2], COMMAND_EC_UP_OFF);
    EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[1][2], COMMAND_PH_UP_OFF);

    NORMAL_OP_SET_MAIN_PUMP(true);
    EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[6][2], COMMAND_ON_OR_UP);

    NORMAL_OP_SET_ROOM_LEDS(true);
    EXPECT_EQ(pump_io_transfer_to_stm_ptr_buffer[7][2], COMMAND_ON_OR_UP);
}

TEST_F(NormalOpTest, NullPointersSafeAndCalibrationUpdatesLiveVoltageSignals) {
    // Null output pointers must not crash
    NORMAL_OP_GET_PH_EC_SENSORS(nullptr, nullptr);

    pump_io_get_ph_duty_V_float_data = 2.88f / 0.05f;
    pump_io_get_ec_duty_V_float_data = 1.66f / 0.05f;
    float ph = 0.0f, ec = 0.0f;
    for (int i = 0; i < 300; ++i) {
        NORMAL_OP_GET_PH_EC_SENSORS(&ph, &ec);
    }

    EXPECT_NEAR(NORMAL_OP_PH_CALIBRATION_GET_VOLTAGE(), 2.88f, 0.02f);
    EXPECT_NEAR(signal_data_set_ph_voltage, 2.88f, 0.02f);
    EXPECT_NEAR(NORMAL_OP_EC_CALIBRATION_GET_VOLTAGE(), 1.66f, 0.02f);
    EXPECT_NEAR(signal_data_set_ec_voltage, 1.66f, 0.02f);

    // One-shot pH mid calibration clears flag and recalculates regression
    signal_ctrl_system_cal_ph_mid = SENSOR_IC_SIGNAL_TRUE;
    signal_data_set_ph_cal_mid_V = 2.88f;
    NORMAL_OP_PH_CALIBRATION();
    EXPECT_EQ(signal_ctrl_system_cal_ph_mid, SENSOR_IC_SIGNAL_FALSE);
}
