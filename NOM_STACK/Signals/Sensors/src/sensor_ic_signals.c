#include "../inc/sensor_ic_signals.h"

// #include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"

//rtc
DS1302_DateTime *signal_data_get_rtc_data;
DS1302_DateTime *signal_data_set_rtc_data;

void init_nvs()
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
    nvs_handle_t handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS!\n", esp_err_to_name(err));
        return;
    }

    err = nvs_set_blob(handle, "signal_data", data, sizeof(signal_data_t));
    if (err == ESP_OK) {
        nvs_commit(handle);
        // printf("Data saved successfully.\n");
    } else {
        printf("Failed to save data: %s\n", esp_err_to_name(err));
    }

    nvs_close(handle);
}

bool load_signal_data(signal_data_t *data)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &handle);
    if (err != ESP_OK) {
        printf("No stored data found.\n");
        return false;
    }

    size_t required_size = sizeof(signal_data_t);
    err = nvs_get_blob(handle, "signal_data", data, &required_size);
    nvs_close(handle);

    if (err == ESP_OK) {
        // printf("Data loaded successfully.\n");
        return true;
    } else {
        printf("Error reading data: %s\n", esp_err_to_name(err));
        return false;
    }
}

// void app_main(void)
// {
//     init_nvs();

//     signal_data_t data;

//     // Try loading existing data
//     if (!load_signal_data(&data)) {
//         printf("No saved data, using defaults.\n");
//         data = signal_data;  // use default values
//         save_signal_data(&data); // save defaults to NVS
//     }

//     // Example: modify a value
//     data.tank_capacity = 200;
//     save_signal_data(&data);

//     // Reload to confirm
//     signal_data_t loaded_data;
//     load_signal_data(&loaded_data);

//     printf("Loaded tank capacity: %d\n", loaded_data.tank_capacity);
// }

void SENSORS_INIT_SIGNALS(void)
{
/*
    data signals
*/
    //LED
    signal_data_get_turn_on_time                                        = 12;

    //water level
    signal_data_get_water_level_data                                    = 0.0f;

    //water temperature
    signal_data_get_water_temperature_data                              = 0.0f;

    //ph
    signal_data_get_ph_data                                             = 0.0f;
    signal_data_get_target_ph_data                                      = 0.0f;
    signal_data_get_ph_concentration_data                               = 0.0f;
    signal_data_get_ph_water_volume_data                                = 0.0f;
    signal_data_set_ph_difference_data                                  = 0.0f;
    signal_data_set_ph_req_volume                                       = 0.0f;
    signal_data_set_ph_cal_low_V                                        = 3.16f;
    signal_data_set_ph_cal_mid_V                                        = 3.01f;
    signal_data_set_ph_cal_high_V                                       = 2.58f;

    //ec
    signal_data_get_ec_data                                             = 0.0f;
    signal_data_get_target_ec_data                                      = 0.0f;
    signal_data_get_ec_concentration_data                               = 0.0f;
    signal_data_get_ec_water_volume_data                                = 0.0f;
    signal_data_set_ec_difference_data                                  = 0.0f;
    signal_data_set_ec_req_volume                                       = 0.0f;
    signal_data_set_ec_cal_low_V                                        = 0.0f;
    signal_data_set_ec_cal_mid_V                                        = 0.0f;
    signal_data_set_ec_cal_high_V                                       = 0.0f;

    //rtc
    // DS1302_DateTime *signal_data_get_rtc_data;

    //selected fruit
    signal_data_set_selected_fruit                                      = 18;
    // signal_data_set_text_selected_fruit[15];
    signal_data_set_text_selected_fruit[0]  = 'N';
    signal_data_set_text_selected_fruit[1]  = 'O';
    signal_data_set_text_selected_fruit[2]  = ' ';
    signal_data_set_text_selected_fruit[3]  = 'S';
    signal_data_set_text_selected_fruit[4]  = 'E';
    signal_data_set_text_selected_fruit[5]  = 'L';
    signal_data_set_text_selected_fruit[6]  = 'E';
    signal_data_set_text_selected_fruit[7]  = 'C';
    signal_data_set_text_selected_fruit[8]  = 'T';
    signal_data_set_text_selected_fruit[9]  = 'I';
    signal_data_set_text_selected_fruit[10] = 'O';
    signal_data_set_text_selected_fruit[11] = 'N';
    signal_data_set_text_selected_fruit[12] = ' ';
    signal_data_set_text_selected_fruit[13] = ' ';
    signal_data_set_text_selected_fruit[14] = ' ';

    //current led status on the pipes
    // signal_data_set_text_led_status[3];
    signal_data_set_text_led_status[0] = 'O';
    signal_data_set_text_led_status[1] = 'F';
    signal_data_set_text_led_status[2] = 'F';

    //current pump status 
    // signal_data_set_text_pump_status[3];
    signal_data_set_text_pump_status[0] = 'O';
    signal_data_set_text_pump_status[1] = 'F';
    signal_data_set_text_pump_status[2] = 'F';

    //current sys status 
    // signal_data_set_text_sys_status[3];
    signal_data_set_text_sys_status[0] = 'O';
    signal_data_set_text_sys_status[1] = 'F';
    signal_data_set_text_sys_status[2] = 'F';

    // uint8_t second;     //!< Second 0..59
    // uint8_t minute;     //!< Minute 0..59
    // uint8_t hour;       //!< Hour 0..23
    // uint8_t dayWeek;    //!< Day of the week (0 as Sunday/ 1 as Monday)
    // uint8_t dayMonth;   //!< Day of the month 1..31
    // uint8_t month;      //!< Month 1..12
    // uint16_t year;      //!< Year 2000..2099
    //simple local allocation to avoid crash
    ref_get_rtc_data.year = 2025;
    ref_get_rtc_data.month = 3;
    ref_get_rtc_data.dayWeek = 1;
    ref_get_rtc_data.dayMonth = 29;
    ref_get_rtc_data.hour = 3;
    ref_get_rtc_data.minute = 49;
    ref_get_rtc_data.second = 30;

    ref_set_rtc_data.year = 2025;
    ref_set_rtc_data.month = 3;
    ref_set_rtc_data.dayWeek = 1;
    ref_set_rtc_data.dayMonth = 29;
    ref_set_rtc_data.hour = 3;
    ref_set_rtc_data.minute = 49;
    ref_set_rtc_data.second = 30;

    signal_data_get_rtc_data = &ref_get_rtc_data;
    signal_data_set_rtc_data = &ref_set_rtc_data;

    signal_data_get_rtc_data->year = 2025;
    signal_data_get_rtc_data->month = 3;
    signal_data_get_rtc_data->dayWeek = 1;
    signal_data_get_rtc_data->dayMonth = 29;
    signal_data_get_rtc_data->hour = 3;
    signal_data_get_rtc_data->minute = 49;
    signal_data_get_rtc_data->second = 30;

    signal_data_set_rtc_data->year = 2025;
    signal_data_set_rtc_data->month = 3;
    signal_data_set_rtc_data->dayWeek = 1;
    signal_data_set_rtc_data->dayMonth = 29;
    signal_data_set_rtc_data->hour = 3;
    signal_data_set_rtc_data->minute = 49;
    signal_data_set_rtc_data->second = 30;
    // printf("get_year%d get month %d\r\n", signal_data_get_rtc_data->year, signal_data_get_rtc_data->month);

/*
    control signals
*/
    //system 
    signal_ctrl_system_start                                            = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_stop                                             = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_continue                                         = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_delete                                           = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_release_the_tank                                 = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_system_fill_the_tank                                    = SENSOR_IC_SIGNAL_FALSE;

    //water level
    signal_ctrl_water_level_activate_get_wl                             = SENSOR_IC_SIGNAL_TRUE;
    signal_ctrl_water_level_fill_the_tank                               = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_water_level_start_ph_ec_unit                            = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_water_level_stop_ph_ec_unit                             = SENSOR_IC_SIGNAL_TRUE;
    signal_ctrl_water_level_water_in_out_duty_to_another_unit           = SENSOR_IC_SIGNAL_FALSE;

    //peripherals

    signal_ctrl_peripheral_turn_on_main_tank_water_input                = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_off_main_tank_water_input               = SENSOR_IC_SIGNAL_TRUE;
    signal_ctrl_peripheral_turn_on_main_tank_water_output               = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_off_main_tank_water_output              = SENSOR_IC_SIGNAL_TRUE;

    signal_ctrl_peripheral_turn_on_main_pump                            = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_off_main_pump                           = SENSOR_IC_SIGNAL_TRUE;
    signal_ctrl_peripheral_turn_on_ph_pump                              = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_peripheral_turn_on_ec_pump                              = SENSOR_IC_SIGNAL_FALSE;

    //water temperature
    signal_ctrl_water_temperature_activate_get_wtemp                    = SENSOR_IC_SIGNAL_TRUE;

    //ph
    signal_ctrl_ph_activate_get_ph                                      = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_ph_disable_wl_unit                                      = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_pipe_water_enable_from_ph                               = SENSOR_IC_SIGNAL_FALSE;


    //ec
    signal_ctrl_ec_activate_get_ec                                      = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_ec_disable_wl_unit                                      = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_pipe_water_enable_from_ec                               = SENSOR_IC_SIGNAL_FALSE;

    //rtc
    signal_ctrl_rtc_activate_set_rtc                                    = SENSOR_IC_SIGNAL_FALSE;
    signal_ctrl_rtc_activate_get_rtc                                    = SENSOR_IC_SIGNAL_TRUE;  

    //leds
    signal_ctrl_led_turn_on_led                                         = SENSOR_IC_SIGNAL_FALSE;

    //circulation pumps
    signal_ctrl_circulation_pumps_turn_on                               = SENSOR_IC_SIGNAL_FALSE;
    //mixer motors
    signal_ctrl_mixer_motors_turn_on                                    = SENSOR_IC_SIGNAL_FALSE;

    //nvs or nvm storage logic can be added here
    signal_data_get_system_start_stop_status                            = 0; //uint8_t
    signal_data_set_system_stage_data                                   = 0;  //uint8_t
    signal_data_get_system_stage_data                                   = 0;  //uint8_t
    signal_data_set_selected_fruit                                      = 18; //uint16_t
    signal_data_set_ph_cal_low_V                                        = 3.16f;//float
    signal_data_set_ph_cal_mid_V                                        = 3.01f;//float
    signal_data_set_ph_cal_high_V                                       = 2.58f;//float
    signal_data_set_ec_cal_mid_V                                        = 0.0f; //float
    signal_data_get_tank_capacity                                       = 100.0f;//uint16_t
    signal_data_get_dosing_intensity                                    = 50; //uint16_t percentage
    signal_data_get_ph_concentration_data                               = 10; //uint32_t
    signal_data_get_ec_concentration_data                               = 10; //uint32_t
    signal_data_get_mixing_time                                         = 300; //uint16_t seconds

    init_nvs();

    signal_data_t loaded_data;
    if (load_signal_data(&loaded_data)) {
        // Apply loaded data
        signal_data_get_system_start_stop_status    = loaded_data.system_start_stop_status;
        signal_data_set_system_stage_data           = loaded_data.system_stage_data_set;
        signal_data_get_system_stage_data           = loaded_data.system_stage_data_get;
        signal_data_set_selected_fruit              = loaded_data.selected_fruit;
        signal_data_set_ph_cal_low_V                = loaded_data.ph_cal_low_V;
        signal_data_set_ph_cal_mid_V                = loaded_data.ph_cal_mid_V;
        signal_data_set_ph_cal_high_V               = loaded_data.ph_cal_high_V;
        signal_data_set_ec_cal_mid_V                = loaded_data.ec_cal_mid_V;
        signal_data_get_tank_capacity               = loaded_data.tank_capacity;
        signal_data_get_dosing_intensity            = loaded_data.dosing_intensity;
        signal_data_get_ph_concentration_data       = loaded_data.ph_concentration_data;
        signal_data_get_ec_concentration_data       = loaded_data.ec_concentration_data;
        signal_data_get_mixing_time                 = loaded_data.mixing_time;
    } else {
        // Save default data if no data was loaded
        nvs_signal_data.system_start_stop_status    = signal_data_get_system_start_stop_status;
        nvs_signal_data.system_stage_data_set       = signal_data_set_system_stage_data;
        nvs_signal_data.system_stage_data_get       = signal_data_get_system_stage_data;
        nvs_signal_data.selected_fruit              = signal_data_set_selected_fruit;
        nvs_signal_data.ph_cal_low_V                = signal_data_set_ph_cal_low_V;
        nvs_signal_data.ph_cal_mid_V                = signal_data_set_ph_cal_mid_V;
        nvs_signal_data.ph_cal_high_V               = signal_data_set_ph_cal_high_V;
        nvs_signal_data.ec_cal_mid_V                = signal_data_set_ec_cal_mid_V;
        nvs_signal_data.tank_capacity               = signal_data_get_tank_capacity;
        nvs_signal_data.dosing_intensity            = signal_data_get_dosing_intensity;
        nvs_signal_data.ph_concentration_data       = signal_data_get_ph_concentration_data;
        nvs_signal_data.ec_concentration_data       = signal_data_get_ec_concentration_data;
        nvs_signal_data.mixing_time                 = signal_data_get_mixing_time;

        save_signal_data(&nvs_signal_data);
    }
}