#include <gtest/gtest.h>
#include <cmath>
#include <cstring>

#include "mock_esp_state.h"
#include "../inc/sensor_ic_signals.h"

class SensorIcSignalsTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_esp_reset_all();
    }
};

TEST_F(SensorIcSignalsTest, FreshBootInitializesSafeDefaultsAndNullTerminatedStrings) {
    SENSORS_INIT_SIGNALS();

    EXPECT_EQ(signal_data_get_tank_capacity, DEFAULT_TANK_CAPACITY_L);
    EXPECT_EQ(signal_data_get_dosing_intensity, DEFAULT_DOSING_INTENSITY_ML_MIN);
    EXPECT_EQ(signal_data_get_ph_concentration_data, DEFAULT_PH_CONCENTRATION_DATA);
    EXPECT_EQ(signal_data_get_ec_concentration_data, DEFAULT_EC_CONCENTRATION_DATA);
    EXPECT_EQ(signal_data_get_mixing_time, DEFAULT_MIXING_TIME_S);

    EXPECT_EQ(signal_data_set_selected_fruit, 18U);
    EXPECT_STREQ(signal_data_set_text_selected_fruit, "NO SELECTION   ");
    EXPECT_STREQ(signal_data_set_text_led_status, "OFF");
    EXPECT_STREQ(signal_data_set_text_pump_status, "OFF");
    EXPECT_STREQ(signal_data_set_text_sys_status, "OFF");

    EXPECT_EQ(signal_ctrl_system_start, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_water_level_start_ph_ec_unit, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_ctrl_water_level_stop_ph_ec_unit, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_EQ(mock_nvs_get_commit_count(), 1U);
}

TEST_F(SensorIcSignalsTest, DirtyCheckAndRateLimiterPreventFlashWearOut) {
    SENSORS_INIT_SIGNALS();
    uint32_t initial_commits = mock_nvs_get_commit_count();

    // Unchanged snapshot must not write NVS even after 10 seconds
    mock_esp_advance_time_us(10000000LL);
    EXPECT_FALSE(save_signal_data_if_changed(&nvs_signal_data));
    EXPECT_EQ(mock_nvs_get_commit_count(), initial_commits);

    // Changed snapshot within 2-second cooldown must defer NVS write
    nvs_signal_data.tank_capacity = 250U;
    mock_esp_set_time_us(1500000LL); // only 0.5s since boot save at 1.0s
    EXPECT_FALSE(save_signal_data_if_changed(&nvs_signal_data));
    EXPECT_EQ(mock_nvs_get_commit_count(), initial_commits);

    // Once >= 2.0s elapsed, changed snapshot commits once
    mock_esp_advance_time_us(2000000LL);
    EXPECT_TRUE(save_signal_data_if_changed(&nvs_signal_data));
    EXPECT_EQ(mock_nvs_get_commit_count(), initial_commits + 1U);

    // Subsequent identical call does not commit again
    mock_esp_advance_time_us(5000000LL);
    EXPECT_FALSE(save_signal_data_if_changed(&nvs_signal_data));
    EXPECT_EQ(mock_nvs_get_commit_count(), initial_commits + 1U);
}

TEST_F(SensorIcSignalsTest, PowerLossAutoResumesWhenPersistedStartWithValidCrop) {
    signal_data_t stored = {};
    stored.system_start_stop_status = 1U; // Start
    stored.system_stage_data_set = 2U;
    stored.system_stage_data_get = 2U;
    stored.selected_fruit = 3U; // TOMATO
    stored.ph_cal_low_V = 3.20f;
    stored.ph_cal_mid_V = 3.00f;
    stored.ph_cal_high_V = 2.55f;
    stored.tank_capacity = 150U;
    stored.dosing_intensity = 60U;
    stored.ph_concentration_data = 1500UL;
    stored.ec_concentration_data = 85000UL;
    stored.mixing_time = 240U;
    mock_nvs_set_raw_blob(&stored, sizeof(stored));

    SENSORS_INIT_SIGNALS();

    EXPECT_EQ(signal_ctrl_system_start, SENSOR_IC_SIGNAL_TRUE);
    EXPECT_EQ(signal_ctrl_system_stop, SENSOR_IC_SIGNAL_FALSE);
    EXPECT_EQ(signal_data_set_selected_fruit, 3U);
    EXPECT_STREQ(signal_data_set_text_selected_fruit, "TOMATO         ");
    EXPECT_EQ(signal_data_set_system_stage_data, 2U);
    // Dosing unit stays gated OFF until WL + healthy STM32 link activate it
    EXPECT_EQ(signal_ctrl_water_level_start_ph_ec_unit, SENSOR_IC_SIGNAL_FALSE);
}

TEST_F(SensorIcSignalsTest, CorruptNvsBlobIsSanitizedSafely) {
    signal_data_t corrupt = {};
    corrupt.system_start_stop_status = 99U;
    corrupt.system_stage_data_set = 55U;
    corrupt.selected_fruit = 999U;
    corrupt.ph_cal_low_V = NAN;
    corrupt.ph_cal_mid_V = INFINITY;
    corrupt.ph_cal_high_V = -INFINITY;
    corrupt.ec_cal_mid_V = NAN;
    corrupt.tank_capacity = 0U;
    corrupt.dosing_intensity = 0U;
    corrupt.ph_concentration_data = 0UL;
    corrupt.ec_concentration_data = 0UL;
    corrupt.mixing_time = 2U;
    mock_nvs_set_raw_blob(&corrupt, sizeof(corrupt));

    SENSORS_INIT_SIGNALS();

    EXPECT_EQ(signal_data_get_system_start_stop_status, 0U);
    EXPECT_EQ(signal_data_set_system_stage_data, 0U);
    EXPECT_EQ(signal_data_set_selected_fruit, 17U); // clamped to UNKNOWN FRUIT
    EXPECT_TRUE(std::isfinite(signal_data_set_ph_cal_low_V));
    EXPECT_FLOAT_EQ(signal_data_set_ph_cal_low_V, 3.16f);
    EXPECT_EQ(signal_data_get_tank_capacity, DEFAULT_TANK_CAPACITY_L);
    EXPECT_EQ(signal_data_get_dosing_intensity, DEFAULT_DOSING_INTENSITY_ML_MIN);
    EXPECT_EQ(signal_data_get_ph_concentration_data, DEFAULT_PH_CONCENTRATION_DATA);
    EXPECT_EQ(signal_data_get_ec_concentration_data, DEFAULT_EC_CONCENTRATION_DATA);
    EXPECT_EQ(signal_data_get_mixing_time, DEFAULT_MIXING_TIME_S);
    EXPECT_EQ(signal_ctrl_system_start, SENSOR_IC_SIGNAL_FALSE);
}

TEST_F(SensorIcSignalsTest, RecursiveMutexLockAndResetToDefaultsWorkCleanly) {
    SENSORS_INIT_SIGNALS();
    SIGNALS_lock();
    SIGNALS_lock();
    EXPECT_EQ(mock_esp_get_mutex_lock_depth(), 2U);
    SIGNALS_unlock();
    SIGNALS_unlock();
    EXPECT_EQ(mock_esp_get_mutex_lock_depth(), 0U);

    SIGNALS_set_selected_fruit(1U);
    signal_ctrl_system_start = SENSOR_IC_SIGNAL_TRUE;
    SENSORS_RESET_TO_DEFAULTS();
    EXPECT_EQ(signal_data_set_selected_fruit, 18U);
    EXPECT_EQ(signal_ctrl_system_start, SENSOR_IC_SIGNAL_FALSE);
}

