#include "../inc/scheduler.h"
#include "../../NOM_STACK/Manager/INIT/inc/initialization.h"
#include "../../NOM_STACK/Manager/NormalOp/inc/normal_op.h"
#include "../../NOM_STACK/Sensors/PH_sensor/inc/ph_sensor.h"
#include "../../NOM_STACK/Sensors/EC_sensor/inc/ec_sensor.h"
#include "../../NOM_STACK/Sensors/WL_sensor/inc/wl_sensor.h"
#include "../../NOM_STACK/Sensors/WTEMP_sensor/inc/wtemp_sensor.h"

#include <stdio.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "../../NOM_STACK/Application/WL/inc/wl_application.h"
#include "../../NOM_STACK/Application/EC/inc/ec_application.h"
#include "../../NOM_STACK/Application/PH/inc/ph_application.h"
#include "../../NOM_STACK/Application/LED/inc/led_ctrl_application.h"
#include "../../NOM_STACK/Application/Pipes/inc/pipe_water_ctrl_app.h"
#include "../../NOM_STACK/Application/MAIN_SYS/inc/main_application.h"

//signals
#include "../../NOM_STACK/UI/DWIN_protocol/inc/dwin_protocol.h"
#include "../../NOM_STACK/Services/PUMP_IO_COM/inc/pump_io_com.h"

//rtc
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;

uint16_t test_counter_signal = 0;


#define GPIO_OUTPUTA_PIN 21  // Change this to your desired GPIO pin
#define GPIO_OUTPUTB_PIN 22

bool led_state_a = false, led_state_b = false;

void task_1ms(void *arg) 
{
    // if(test_counter_signal < 500)
    // {
    //     test_counter_signal++;
    // }else
    // {
    //     test_counter_signal = 0;
    // }

    if(signal_ctrl_ph_activate_get_ph == SENSOR_IC_SIGNAL_TRUE && signal_ctrl_ec_activate_get_ec == SENSOR_IC_SIGNAL_TRUE)
    {
        // NORMAL_OP_GET_I2C_COM_PH_EC_SENSOR_DATA();
        // NORMAL_OP_GET_PH_EC_SENSORS(&signal_data_get_ph_data, &signal_data_get_ec_data);
        // printf("signal_data_get_ph_data %f \r\n", signal_data_get_ph_data);
        // printf("signal_data_get_ec_data %f \r\n", signal_data_get_ec_data);
    }else
    {
        // signal_data_get_ph_data = 0.0f;
        // signal_data_get_ec_data = 0.0f;
        //just for test
        NORMAL_OP_GET_PH_EC_SENSORS(&signal_data_get_ph_data, &signal_data_get_ec_data);
        // printf("signal_data_get_ph_data %f \r\n", signal_data_get_ph_data);
        // printf("signal_data_get_ec_data %f \r\n", signal_data_get_ec_data);

    }

    //ph and ec pump control
    // NORMAL_OP_SET_PH_EC_PUMPS(signal_ctrl_peripheral_turn_on_ph_pump, signal_ctrl_peripheral_turn_on_ec_pump);****
    // if(test_counter_signal < 250)
    // {
    //     NORMAL_OP_SET_CIRCULATION_PUMPS(false);
    //     NORMAL_OP_SET_MIXER_MOTORS(false);
    //     NORMAL_OP_SET_PH_EC_PUMPS(false, false);
    //     signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
    //     signal_ctrl_circulation_pumps_turn_on = SENSOR_IC_SIGNAL_FALSE;
    // }else
    // {
    //     NORMAL_OP_SET_CIRCULATION_PUMPS(true);
    //     NORMAL_OP_SET_MIXER_MOTORS(true);
    //     NORMAL_OP_SET_PH_EC_PUMPS(true, true);
    //     signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_TRUE;
    //     signal_ctrl_circulation_pumps_turn_on = SENSOR_IC_SIGNAL_TRUE;
    // }
    if(signal_ctrl_circulation_pumps_turn_on == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_CIRCULATION_PUMPS(true);
    }else
    {
        NORMAL_OP_SET_CIRCULATION_PUMPS(false);
    }

    if(signal_ctrl_mixer_motors_turn_on == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_MIXER_MOTORS(true);
    }else
    {
        NORMAL_OP_SET_MIXER_MOTORS(false);
    }

    // if(led_state_b == false)
    // {
    //     gpio_set_level(GPIO_OUTPUTB_PIN, led_state_b);
    //     led_state_b = true;
    // }else
    // {
    //     gpio_set_level(GPIO_OUTPUTB_PIN, led_state_b);
    //     led_state_b = false;  
    // }
#ifdef SCHEDULER_LOGS
    printf("task_1ms...\r\n");
    // ESP_LOGI(SCHEDULER_TAG, "task_1ms...");
#endif
    vTaskDelete(NULL);
}

void task_3ms(void *arg) 
{
    // if(test_counter_signal < 250)
    // {
    //     NORMAL_OP_SET_MAIN_PUMP(false);
    //     NORMAL_OP_SET_ROOM_LEDS(false);
    //     signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_FALSE;
    //     signal_ctrl_peripheral_turn_on_main_pump = SENSOR_IC_SIGNAL_FALSE;
    //     signal_ctrl_peripheral_turn_off_main_pump = SENSOR_IC_SIGNAL_TRUE;
    // }else
    // {
    //     NORMAL_OP_SET_MAIN_PUMP(true);
    //     NORMAL_OP_SET_ROOM_LEDS(true);
    //     signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_TRUE;
    //     signal_ctrl_peripheral_turn_on_main_pump = SENSOR_IC_SIGNAL_TRUE;
    //     signal_ctrl_peripheral_turn_off_main_pump = SENSOR_IC_SIGNAL_FALSE;
    // }
    if(signal_ctrl_peripheral_turn_on_main_pump == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_MAIN_PUMP(true);
    }
    
    if(signal_ctrl_peripheral_turn_off_main_pump == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_MAIN_PUMP(false);
    }

    if(signal_ctrl_led_turn_on_led == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_ROOM_LEDS(true);
    }else
    {
        NORMAL_OP_SET_ROOM_LEDS(false);
    }

    if(signal_ctrl_water_level_activate_get_wl == SENSOR_IC_SIGNAL_TRUE)
    {
        signal_data_get_water_level_data = NORMAL_OP_GET_WATER_LEVEL();
        signal_data_get_water_cap_percentage = NORMAL_OP_GET_WATER_LEVEL_PERCENTAGE();
        // printf("signal_data_get_water_level_data %f \r\n", signal_data_get_water_level_data);
        // printf("signal_data_get_water_cap_percentage %f \r\n", signal_data_get_water_cap_percentage);
    }
    // if(led_state_a == false)
    // {
    //     gpio_set_level(GPIO_OUTPUTA_PIN, led_state_a);
    //     led_state_a = true;
    // }else
    // {
    //     gpio_set_level(GPIO_OUTPUTA_PIN, led_state_a);
    //     led_state_a = false;
    // }
#ifdef SCHEDULER_LOGS
    printf("task_3ms...\r\n");
    // ESP_LOGI(SCHEDULER_TAG, "task_3ms...");
#endif
    vTaskDelete(NULL);
}

void task_5ms(void *arg) 
{
    // if(test_counter_signal < 250)
    // {
    //     NORMAL_OP_SET_TANK_WATER_INPUT(false);
    //     NORMAL_OP_SET_TANK_WATER_OUTPUT(false);
    //     signal_ctrl_peripheral_turn_on_main_tank_water_input = SENSOR_IC_SIGNAL_FALSE;
    //     signal_ctrl_peripheral_turn_off_main_tank_water_input = SENSOR_IC_SIGNAL_TRUE;
    //     signal_ctrl_peripheral_turn_on_main_tank_water_output = SENSOR_IC_SIGNAL_FALSE;
    //     signal_ctrl_peripheral_turn_off_main_tank_water_output = SENSOR_IC_SIGNAL_TRUE;
    // }else
    // {
    //     NORMAL_OP_SET_TANK_WATER_INPUT(true);
    //     NORMAL_OP_SET_TANK_WATER_OUTPUT(true);
    //     signal_ctrl_peripheral_turn_on_main_tank_water_input = SENSOR_IC_SIGNAL_TRUE;
    //     signal_ctrl_peripheral_turn_off_main_tank_water_input = SENSOR_IC_SIGNAL_FALSE;
    //     signal_ctrl_peripheral_turn_on_main_tank_water_output = SENSOR_IC_SIGNAL_TRUE;
    //     signal_ctrl_peripheral_turn_off_main_tank_water_output = SENSOR_IC_SIGNAL_FALSE;
    // }
    
    if(signal_ctrl_peripheral_turn_on_main_tank_water_input == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_TANK_WATER_INPUT(true);
    }

    if(signal_ctrl_peripheral_turn_off_main_tank_water_input == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_TANK_WATER_INPUT(false);
    }

    if(signal_ctrl_peripheral_turn_on_main_tank_water_output == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_TANK_WATER_OUTPUT(true);
    }

    if(signal_ctrl_peripheral_turn_off_main_tank_water_output == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_TANK_WATER_OUTPUT(false);
    }

    if(signal_ctrl_water_temperature_activate_get_wtemp == SENSOR_IC_SIGNAL_TRUE)
    {
    // when this one is not connected or its connected creates delays in processing timeclock cycles
    // current sensor has 700ms delay which is mandatory    
        // signal_data_get_water_temperature_data =25.0f;
        signal_data_get_water_temperature_data = NORMAL_OP_GET_WATER_TEMPERATURE();
        // printf("signal_data_get_water_temperature_data %f \r\n", signal_data_get_water_temperature_data);
    }
    UI_DATA_OUT();

#ifdef SCHEDULER_LOGS
    printf("task_5ms...\r\n");
    // ESP_LOGI(SCHEDULER_TAG, "task_5ms...");
#endif
    vTaskDelete(NULL);
}

void task_10ms(void *arg) 
{
    if(signal_ctrl_rtc_activate_get_rtc == SENSOR_IC_SIGNAL_TRUE)
    {
        signal_data_get_rtc_data = NORMAL_OP_GET_RTC();
        signal_ctrl_rtc_activate_get_rtc = SENSOR_IC_SIGNAL_FALSE;
    }

    if(signal_ctrl_rtc_activate_set_rtc == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_RTC(signal_data_set_rtc_data);
        signal_ctrl_rtc_activate_set_rtc = SENSOR_IC_SIGNAL_FALSE;
        signal_ctrl_rtc_activate_get_rtc = SENSOR_IC_SIGNAL_TRUE;
    }
    PUMP_IO_COM_EXE();
    //here put the pump io com output

#ifdef SCHEDULER_LOGS
    printf("task_10ms...\r\n");
    // ESP_LOGI(SCHEDULER_TAG, "task_10ms...");
#endif
    vTaskDelete(NULL);
}

void task_15ms(void *arg) 
{
    NORMAL_OP_EC_CALIBRATION();
    NORMAL_OP_PH_CALIBRATION();
    WL_EXE_APPLICATION();//Execution of the Water Level Unit logic application 
    EC_EXE_APPLICATION();
    PH_EXE_APPLICATION();
    LED_CTRL_EXE_APPLICATION();
    PIPE_WATER_CTRL_EXE_APPLICATION();

#ifdef SCHEDULER_LOGS
    printf("task_15ms...\r\n");
    // ESP_LOGI(SCHEDULER_TAG, "task_15ms...");
#endif
    vTaskDelete(NULL);
}

void task_20ms(void *arg) 
{
    UI_EXE();
    //maybe some signal executions also put here
#ifdef SCHEDULER_LOGS
    printf("task_20ms...\r\n");
    // ESP_LOGI(SCHEDULER_TAG, "task_20ms...");
#endif
    vTaskDelete(NULL);
}

uint8_t task_25_ms_rtc_counter = 0;
void task_25ms(void *arg) 
{
    //signal management
    if(task_25_ms_rtc_counter < 100)
    {
        task_25_ms_rtc_counter++;
    }else
    {
        task_25_ms_rtc_counter = 0;
        signal_ctrl_rtc_activate_get_rtc = SENSOR_IC_SIGNAL_TRUE;    
    }
    //put non voltaile memory application here
    //nvm data
    // rtc
    // fruit
    // process state
    MAIN_FRUIT_EXE();
    //nvs/nvm storage exe
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

    
    //also diagnostics here
#ifdef SCHEDULER_LOGS
    printf("task_25ms...\r\n");
    // ESP_LOGI(SCHEDULER_TAG, "task_25ms...");
#endif
    vTaskDelete(NULL);
}