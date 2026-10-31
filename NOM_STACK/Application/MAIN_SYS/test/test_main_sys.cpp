#include <gtest/gtest.h>

#include "mock_esp_state.h"
#include "../inc/main_application.h"

class MainSysTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
        SENSORS_INIT_SIGNALS();
    }
};

TEST_F(MainSysTest, CropTargetsUseMicroSiemensPerCmAndRespectStage0To2) {
    signal_data_set_selected_fruit = STRAWBERRY_ADDRESS;

    signal_data_set_system_stage_data = FIRST_PHASE;
    MAIN_FRUIT_EXE();
    EXPECT_EQ(signal_data_get_turn_on_time, 14U);
    EXPECT_FLOAT_EQ(signal_data_get_target_ph_data, 5.9f);
    EXPECT_FLOAT_EQ(signal_data_get_target_ec_data, 2000.0f);

    signal_data_set_system_stage_data = SECOND_PHASE;
    MAIN_FRUIT_EXE();
    EXPECT_EQ(signal_data_get_turn_on_time, 17U);
    EXPECT_FLOAT_EQ(signal_data_get_target_ph_data, 6.1f);
    EXPECT_FLOAT_EQ(signal_data_get_target_ec_data, 2300.0f);

    signal_data_set_system_stage_data = THIRD_PHASE;
    MAIN_FRUIT_EXE();
    EXPECT_EQ(signal_data_get_turn_on_time, 20U);
    EXPECT_FLOAT_EQ(signal_data_get_target_ph_data, 6.2f);
    EXPECT_FLOAT_EQ(signal_data_get_target_ec_data, 2700.0f);
}

TEST_F(MainSysTest, OutOfBoundsStageAndInvalidCropFallbackSafely) {
    // Stage > 2 clamps to stage 2
    signal_data_set_selected_fruit = LETTUCE_ADDRESS;
    signal_data_set_system_stage_data = 99U;
    MAIN_FRUIT_EXE();
    EXPECT_EQ(signal_data_get_system_stage_data, THIRD_PHASE);
    EXPECT_FLOAT_EQ(signal_data_get_target_ec_data, 1200.0f);

    // Invalid crop ID (> 16) sets zero targets
    signal_data_set_selected_fruit = 999U;
    MAIN_FRUIT_EXE();
    EXPECT_EQ(signal_data_get_turn_on_time, 0U);
    EXPECT_FLOAT_EQ(signal_data_get_target_ph_data, 0.0f);
    EXPECT_FLOAT_EQ(signal_data_get_target_ec_data, 0.0f);
}

