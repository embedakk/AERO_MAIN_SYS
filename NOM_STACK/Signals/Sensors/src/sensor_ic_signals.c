#include "../inc/sensor_ic_signals.h"

#include "esp_timer.h"
#include "nvs_flash.h"
#include "nvs.h"

/*
    data signals
*/
uint8_t signal_data_get_turn_on_time = 12;

float signal_data_get_water_level_data = 0.0f;
float signal_data_get_water_cap_percentage = 0.0f;

float signal_data_get_water_temperature_data = 25.0f;

float signal_data_get_ph_data = 0.0f;
float signal_data_get_target_ph_data = 0.0f;
uint32_t signal_data_get_ph_concentration_data = DEFAULT_PH_CONCENTRATION_DATA;
float signal_data_get_ph_water_volume_data = 0.0f;
float signal_data_set_ph_difference_data = 0.0f;
float signal_data_set_ph_req_volume = 0.0f;
float signal_data_set_ph_voltage = 0.0f;

float signal_data_set_ph_cal_low_V = 3.16f;
float signal_data_set_ph_cal_mid_V = 3.01f;
float signal_data_set_ph_cal_high_V = 2.58f;

float signal_data_get_ec_data = 0.0f;
float signal_data_get_target_ec_data = 0.0f;
uint32_t signal_data_get_ec_concentration_data = DEFAULT_EC_CONCENTRATION_DATA;
float signal_data_get_ec_water_volume_data = 0.0f;
float signal_data_set_ec_difference_data = 0.0f;
float signal_data_set_ec_req_volume = 0.0f;
float signal_data_set_ec_voltage = 0.0f;

float signal_data_set_ec_cal_low_V = 0.0f;
float signal_data_set_ec_cal_mid_V = 0.0f;
float signal_data_set_ec_cal_high_V = 0.0f;

float signal_data_get_room_temperature_data = 0.0f;
float signal_data_get_room_humidity_data = 0.0f;

uint16_t signal_data_set_selected_fruit = 18;

char signal_data_set_text_selected_fruit[16] = "NO SELECTION   ";
char signal_data_set_text_led_status[4] = "OFF";
char signal_data_set_text_pump_status[4] = "OFF";
char signal_data_set_text_sys_status[4] = "OFF";

uint16_t signal_data_get_tank_capacity = DEFAULT_TANK_CAPACITY_L;
uint16_t signal_data_get_dosing_intensity = DEFAULT_DOSING_INTENSITY_ML_MIN;
uint16_t signal_data_get_mixing_time = DEFAULT_MIXING_TIME_S;

uint8_t signal_data_get_system_stage_data = 0;
uint8_t signal_data_set_system_stage_data = 0;

DS1302_DateTime ref_get_rtc_data = {30, 49, 3, 1, 29, 3, 2025};
DS1302_DateTime ref_set_rtc_data = {30, 49, 3, 1, 29, 3, 2025};
DS1302_DateTime *signal_data_get_rtc_data = &ref_get_rtc_data;
DS1302_DateTime *signal_data_set_rtc_data = &ref_set_rtc_data;

uint8_t pump_io_get_ec_duty_V_byte_data[4] = {0};
uint8_t pump_io_get_ph_duty_V_byte_data[4] = {0};
uint8_t pump_io_get_temperature_byte_data[4] = {0};
uint8_t pump_io_get_distance_data = 0;

float pump_io_get_ec_duty_V_float_data = 0.0f;
float pump_io_get_ph_duty_V_float_data = 0.0f;
float pump_io_get_temperature_float_data = 25.0f;

/*
    control signals
*/
uint8_t signal_data_get_system_start_stop_status = 0;
sensor_ic_signal_status signal_ctrl_system_start = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_system_stop = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_system_continue = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_system_delete = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_system_release_the_tank = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_system_fill_the_tank = SENSOR_IC_SIGNAL_FALSE;

sensor_ic_signal_status signal_ctrl_system_cal_ph_low = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_system_cal_ph_mid = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_system_cal_ph_high = SENSOR_IC_SIGNAL_FALSE;

sensor_ic_signal_status signal_ctrl_system_cal_ec_low = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_system_cal_ec_mid = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_system_cal_ec_high = SENSOR_IC_SIGNAL_FALSE;

sensor_ic_signal_status signal_ctrl_water_level_activate_get_wl = SENSOR_IC_SIGNAL_TRUE;
sensor_ic_signal_status signal_ctrl_water_level_fill_the_tank = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_water_level_start_ph_ec_unit = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_water_level_stop_ph_ec_unit = SENSOR_IC_SIGNAL_TRUE;
sensor_ic_signal_status signal_ctrl_water_level_water_in_out_duty_to_another_unit = SENSOR_IC_SIGNAL_FALSE;

sensor_ic_signal_status signal_ctrl_peripheral_turn_on_main_tank_water_input = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_peripheral_turn_off_main_tank_water_input = SENSOR_IC_SIGNAL_TRUE;
sensor_ic_signal_status signal_ctrl_peripheral_turn_on_main_tank_water_output = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_peripheral_turn_off_main_tank_water_output = SENSOR_IC_SIGNAL_TRUE;

sensor_ic_signal_status signal_ctrl_peripheral_turn_on_main_pump = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_peripheral_turn_off_main_pump = SENSOR_IC_SIGNAL_TRUE;
sensor_ic_signal_status signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;

sensor_ic_signal_status signal_ctrl_water_temperature_activate_get_wtemp = SENSOR_IC_SIGNAL_TRUE;

sensor_ic_signal_status signal_ctrl_ph_activate_get_ph = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_pipe_water_enable_from_ph = SENSOR_IC_SIGNAL_FALSE;

sensor_ic_signal_status signal_ctrl_ec_activate_get_ec = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_pipe_water_enable_from_ec = SENSOR_IC_SIGNAL_FALSE;

sensor_ic_signal_status signal_ctrl_rtc_activate_set_rtc = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_rtc_activate_get_rtc = SENSOR_IC_SIGNAL_TRUE;

sensor_ic_signal_status signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_circulation_pumps_turn_on = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;

signal_data_t nvs_signal_data;

static SemaphoreHandle_t s_signals_mutex = NULL;
static signal_data_t s_last_saved_nvs_data;
static bool s_has_saved_snapshot = false;
static int64_t s_last_nvs_save_us = 0;

static const char s_fruit_names[19][16] = {
    "BROCCOLI       ",
    "STRAWBERRY     ",
    "ROCKET         ",
    "TOMATO         ",
    "LETTUCE        ",
    "LEAK           ",
    "EGGPLANT       ",
    "PEAS           ",
    "BRUSSEL SPROUTS",
    "CABBAGE        ",
    "CHILLI         ",
    "SPINACH        ",
    "PARSLEY        ",
    "GREAN BEANS    ",
    "CUCUMBER       ",
    "KALE           ",
    "ASIAN GREANS   ",
    "UNKNOWN FRUIT  ",
    "NO SELECTION   "
};

void SIGNALS_init_sync(void)
{
    if (s_signals_mutex == NULL) {
        s_signals_mutex = xSemaphoreCreateRecursiveMutex();
    }
}

void SIGNALS_lock(void)
{
    if (s_signals_mutex != NULL) {
        xSemaphoreTakeRecursive(s_signals_mutex, portMAX_DELAY);
    }
}

void SIGNALS_unlock(void)
{
    if (s_signals_mutex != NULL) {
        xSemaphoreGiveRecursive(s_signals_mutex);
    }
}

void SIGNALS_set_selected_fruit(uint16_t fruit_id)
{
    uint16_t safe_id = (fruit_id <= 18U) ? fruit_id : 17U;
    signal_data_set_selected_fruit = safe_id;
    memcpy(signal_data_set_text_selected_fruit, s_fruit_names[safe_id], 15U);
    signal_data_set_text_selected_fruit[15] = '\0';
}

void init_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

void save_signal_data(signal_data_t *data)
{
    if (data == NULL) {
        return;
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS!\n", esp_err_to_name(err));
        return;
    }

    err = nvs_set_blob(handle, "signal_data", data, sizeof(signal_data_t));
    if (err == ESP_OK) {
        nvs_commit(handle);
        s_last_saved_nvs_data = *data;
        s_has_saved_snapshot = true;
        s_last_nvs_save_us = esp_timer_get_time();
    } else {
        printf("Failed to save data: %s\n", esp_err_to_name(err));
    }

    nvs_close(handle);
}

bool save_signal_data_if_changed(signal_data_t *data)
{
    if (data == NULL) {
        return false;
    }

    if (s_has_saved_snapshot &&
        memcmp(data, &s_last_saved_nvs_data, sizeof(signal_data_t)) == 0) {
        return false;
    }

    int64_t now_us = esp_timer_get_time();
    if (s_has_saved_snapshot &&
        (now_us - s_last_nvs_save_us) >= 0 &&
        (now_us - s_last_nvs_save_us) < NVS_MIN_SAVE_INTERVAL_US) {
        return false;
    }

    save_signal_data(data);
    return true;
}

bool load_signal_data(signal_data_t *data)
{
    if (data == NULL) {
        return false;
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &handle);
    if (err != ESP_OK) {
        printf("No stored data found.\n");
        return false;
    }

    size_t required_size = sizeof(signal_data_t);
    err = nvs_get_blob(handle, "signal_data", data, &required_size);
    nvs_close(handle);

    if (err == ESP_OK && required_size == sizeof(signal_data_t)) {
        return true;
    } else {
        printf("Error reading data: %s\n", esp_err_to_name(err));
        return false;
    }
}

static void populate_nvs_snapshot_from_signals(signal_data_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    dst->system_start_stop_status = signal_data_get_system_start_stop_status;
    dst->system_stage_data_set    = signal_data_set_system_stage_data;
    dst->system_stage_data_get    = signal_data_get_system_stage_data;
    dst->selected_fruit           = signal_data_set_selected_fruit;
    dst->ph_cal_low_V             = signal_data_set_ph_cal_low_V;
    dst->ph_cal_mid_V             = signal_data_set_ph_cal_mid_V;
    dst->ph_cal_high_V            = signal_data_set_ph_cal_high_V;
    dst->ec_cal_mid_V             = signal_data_set_ec_cal_mid_V;
    dst->tank_capacity            = signal_data_get_tank_capacity;
    dst->dosing_intensity         = signal_data_get_dosing_intensity;
    dst->ph_concentration_data    = signal_data_get_ph_concentration_data;
    dst->ec_concentration_data    = signal_data_get_ec_concentration_data;
    dst->mixing_time              = signal_data_get_mixing_time;
}

static void reset_signals_in_ram(void)
{
    signal_data_get_turn_on_time = 12;

    signal_data_get_water_level_data = 0.0f;
    signal_data_get_water_cap_percentage = 0.0f;
    signal_data_get_water_temperature_data = 25.0f;

    signal_data_get_ph_data = 0.0f;
    signal_data_get_target_ph_data = 0.0f;
    signal_data_get_ph_concentration_data = DEFAULT_PH_CONCENTRATION_DATA;
    signal_data_get_ph_water_volume_data = 0.0f;
    signal_data_set_ph_difference_data = 0.0f;
    signal_data_set_ph_req_volume = 0.0f;
    signal_data_set_ph_voltage = 0.0f;
    signal_data_set_ph_cal_low_V = 3.16f;
    signal_data_set_ph_cal_mid_V = 3.01f;
    signal_data_set_ph_cal_high_V = 2.58f;

    signal_data_get_ec_data = 0.0f;
    signal_data_get_target_ec_data = 0.0f;
    signal_data_get_ec_concentration_data = DEFAULT_EC_CONCENTRATION_DATA;
    signal_data_get_ec_water_volume_data = 0.0f;
    signal_data_set_ec_difference_data = 0.0f;
    signal_data_set_ec_req_volume = 0.0f;
    signal_data_set_ec_voltage = 0.0f;
    signal_data_set_ec_cal_low_V = 0.0f;
    signal_data_set_ec_cal_mid_V = 0.0f;
    signal_data_set_ec_cal_high_V = 0.0f;

    signal_data_get_room_temperature_data = 0.0f;
    signal_data_get_room_humidity_data = 0.0f;

    SIGNALS_set_selected_fruit(18U);
    memcpy(signal_data_set_text_led_status, "OFF", 4U);
    memcpy(signal_data_set_text_pump_status, "OFF", 4U);
    memcpy(signal_data_set_text_sys_status, "OFF", 4U);

    signal_data_get_system_start_stop_status = 0;
    signal_data_set_system_stage_data = 0;
    signal_data_get_system_stage_data = 0;
    signal_data_get_tank_capacity = DEFAULT_TANK_CAPACITY_L;
    signal_data_get_dosing_intensity = DEFAULT_DOSING_INTENSITY_ML_MIN;
    signal_data_get_mixing_time = DEFAULT_MIXING_TIME_S;

    signal_ctrl_system_start = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_stop = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_continue = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_delete = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_release_the_tank = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_fill_the_tank = SENSOR_IC_SIGNAL_FALSE;

    signal_ctrl_system_cal_ph_low = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_cal_ph_mid = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_cal_ph_high = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_cal_ec_low = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_cal_ec_mid = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_cal_ec_high = SENSOR_IC_SIGNAL_FALSE;

    signal_ctrl_water_level_activate_get_wl = SENSOR_IC_SIGNAL_TRUE;
    signal_ctrl_water_level_fill_the_tank = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_water_level_start_ph_ec_unit = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_water_level_stop_ph_ec_unit = SENSOR_IC_SIGNAL_TRUE;
    signal_ctrl_water_level_water_in_out_duty_to_another_unit = SENSOR_IC_SIGNAL_FALSE;

    signal_ctrl_peripheral_turn_on_main_tank_water_input = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_off_main_tank_water_input = SENSOR_IC_SIGNAL_TRUE;
    signal_ctrl_peripheral_turn_on_main_tank_water_output = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_off_main_tank_water_output = SENSOR_IC_SIGNAL_TRUE;

    signal_ctrl_peripheral_turn_on_main_pump = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_off_main_pump = SENSOR_IC_SIGNAL_TRUE;
    signal_ctrl_peripheral_turn_on_ph_pump = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_on_ec_pump = SENSOR_IC_SIGNAL_FALSE;

    signal_ctrl_water_temperature_activate_get_wtemp = SENSOR_IC_SIGNAL_TRUE;

    signal_ctrl_ph_activate_get_ph = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_ph_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_pipe_water_enable_from_ph = SENSOR_IC_SIGNAL_FALSE;

    signal_ctrl_ec_activate_get_ec = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_ec_disable_wl_unit = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_pipe_water_enable_from_ec = SENSOR_IC_SIGNAL_FALSE;

    signal_ctrl_rtc_activate_set_rtc = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_rtc_activate_get_rtc = SENSOR_IC_SIGNAL_TRUE;

    signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_circulation_pumps_turn_on = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
}

void SENSORS_RESET_TO_DEFAULTS(void)
{
    reset_signals_in_ram();
    populate_nvs_snapshot_from_signals(&nvs_signal_data);
    save_signal_data(&nvs_signal_data);
}

void SENSORS_INIT_SIGNALS(void)
{
    SIGNALS_init_sync();
    s_has_saved_snapshot = false;
    s_last_nvs_save_us = 0;

    ref_get_rtc_data.year = 2025;
    ref_get_rtc_data.month = 3;
    ref_get_rtc_data.dayWeek = 1;
    ref_get_rtc_data.dayMonth = 29;
    ref_get_rtc_data.hour = 3;
    ref_get_rtc_data.minute = 49;
    ref_get_rtc_data.second = 30;

    ref_set_rtc_data = ref_get_rtc_data;
    signal_data_get_rtc_data = &ref_get_rtc_data;
    signal_data_set_rtc_data = &ref_set_rtc_data;

    memset(pump_io_get_ec_duty_V_byte_data, 0, sizeof(pump_io_get_ec_duty_V_byte_data));
    memset(pump_io_get_ph_duty_V_byte_data, 0, sizeof(pump_io_get_ph_duty_V_byte_data));
    memset(pump_io_get_temperature_byte_data, 0, sizeof(pump_io_get_temperature_byte_data));
    pump_io_get_distance_data = 0;
    pump_io_get_ec_duty_V_float_data = 0.0f;
    pump_io_get_ph_duty_V_float_data = 0.0f;
    pump_io_get_temperature_float_data = 25.0f;

    reset_signals_in_ram();

    init_nvs();

    signal_data_t loaded_data;
    memset(&loaded_data, 0, sizeof(loaded_data));
    if (load_signal_data(&loaded_data)) {
        signal_data_get_system_start_stop_status = (loaded_data.system_start_stop_status <= 3U) ? loaded_data.system_start_stop_status : 0U;
        signal_data_set_system_stage_data        = (loaded_data.system_stage_data_set <= 2U) ? loaded_data.system_stage_data_set : 0U;
        signal_data_get_system_stage_data        = (loaded_data.system_stage_data_get <= 2U) ? loaded_data.system_stage_data_get : signal_data_set_system_stage_data;

        SIGNALS_set_selected_fruit(loaded_data.selected_fruit);

        signal_data_set_ph_cal_low_V  = isfinite(loaded_data.ph_cal_low_V)  ? loaded_data.ph_cal_low_V  : 3.16f;
        signal_data_set_ph_cal_mid_V  = isfinite(loaded_data.ph_cal_mid_V)  ? loaded_data.ph_cal_mid_V  : 3.01f;
        signal_data_set_ph_cal_high_V = isfinite(loaded_data.ph_cal_high_V) ? loaded_data.ph_cal_high_V : 2.58f;
        signal_data_set_ec_cal_mid_V  = isfinite(loaded_data.ec_cal_mid_V)  ? loaded_data.ec_cal_mid_V  : 0.0f;

        signal_data_get_tank_capacity         = (loaded_data.tank_capacity >= 1U && loaded_data.tank_capacity <= 5000U) ? loaded_data.tank_capacity : DEFAULT_TANK_CAPACITY_L;
        signal_data_get_dosing_intensity      = (loaded_data.dosing_intensity >= 1U && loaded_data.dosing_intensity <= 1000U) ? loaded_data.dosing_intensity : DEFAULT_DOSING_INTENSITY_ML_MIN;
        signal_data_get_ph_concentration_data = (loaded_data.ph_concentration_data >= 1UL) ? loaded_data.ph_concentration_data : DEFAULT_PH_CONCENTRATION_DATA;
        signal_data_get_ec_concentration_data = (loaded_data.ec_concentration_data >= 1UL) ? loaded_data.ec_concentration_data : DEFAULT_EC_CONCENTRATION_DATA;
        signal_data_get_mixing_time           = (loaded_data.mixing_time >= 10U && loaded_data.mixing_time <= 3600U) ? loaded_data.mixing_time : DEFAULT_MIXING_TIME_S;

        s_last_saved_nvs_data = loaded_data;
        s_has_saved_snapshot = true;
        s_last_nvs_save_us = esp_timer_get_time();

        /* Auto-resume after power loss once valid STM32 sensor frames arrive */
        if ((signal_data_get_system_start_stop_status == 1U || signal_data_get_system_start_stop_status == 2U) &&
            signal_data_set_selected_fruit < 17U) {
            signal_ctrl_system_start = SENSOR_IC_SIGNAL_TRUE;
            signal_ctrl_system_continue = (signal_data_get_system_start_stop_status == 2U)
                                              ? SENSOR_IC_SIGNAL_TRUE
                                              : SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_stop = SENSOR_IC_SIGNAL_FALSE;
        } else if (signal_data_get_system_start_stop_status == 1U || signal_data_get_system_start_stop_status == 2U) {
            /* Invalid crop persisted with Start/Continue -> force safe Stop */
            signal_data_get_system_start_stop_status = 0U;
        }
    } else {
        populate_nvs_snapshot_from_signals(&nvs_signal_data);
        save_signal_data(&nvs_signal_data);
    }

    populate_nvs_snapshot_from_signals(&nvs_signal_data);
}
