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

#include "../../NOM_STACK/UI/DWIN_protocol/inc/dwin_protocol.h"
#include "../../NOM_STACK/Services/PUMP_IO_COM/inc/pump_io_com.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"

//rtc
extern DS1302_DateTime *signal_data_get_rtc_data;
extern DS1302_DateTime *signal_data_set_rtc_data;

uint16_t test_counter_signal = 0;

#define GPIO_OUTPUTA_PIN 21
#define GPIO_OUTPUTB_PIN 22

bool led_state_a = false, led_state_b = false;
uint8_t task_25_ms_rtc_counter = 0;

void scheduler_step_1ms(void)
{
    SIGNALS_lock();

    // Always sample pH and EC sensors so readings stay live both idle and during active dosing
    NORMAL_OP_GET_PH_EC_SENSORS(&signal_data_get_ph_data, &signal_data_get_ec_data);

    // Forward pH and EC peristaltic dosing pump commands to STM32 IO slots
    NORMAL_OP_SET_PH_EC_PUMPS(
        signal_ctrl_peripheral_turn_on_ph_pump == SENSOR_IC_SIGNAL_TRUE,
        signal_ctrl_peripheral_turn_on_ec_pump == SENSOR_IC_SIGNAL_TRUE);

    NORMAL_OP_SET_CIRCULATION_PUMPS(signal_ctrl_circulation_pumps_turn_on == SENSOR_IC_SIGNAL_TRUE);
    NORMAL_OP_SET_MIXER_MOTORS(signal_ctrl_mixer_motors_turn_on == SENSOR_IC_SIGNAL_TRUE);

    SIGNALS_unlock();
}

void scheduler_step_3ms(void)
{
    SIGNALS_lock();

    if (signal_ctrl_peripheral_turn_on_main_pump == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_MAIN_PUMP(true);
    }

    if (signal_ctrl_peripheral_turn_off_main_pump == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_MAIN_PUMP(false);
    }

    NORMAL_OP_SET_ROOM_LEDS(signal_ctrl_led_turn_on_led == SENSOR_IC_SIGNAL_TRUE);

    if (signal_ctrl_water_level_activate_get_wl == SENSOR_IC_SIGNAL_TRUE)
    {
        signal_data_get_water_level_data = NORMAL_OP_GET_WATER_LEVEL();
        signal_data_get_water_cap_percentage = NORMAL_OP_GET_WATER_LEVEL_PERCENTAGE();
    }

    SIGNALS_unlock();
}

void scheduler_step_5ms(void)
{
    SIGNALS_lock();

    if (signal_ctrl_peripheral_turn_on_main_tank_water_input == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_TANK_WATER_INPUT(true);
    }

    if (signal_ctrl_peripheral_turn_off_main_tank_water_input == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_TANK_WATER_INPUT(false);
    }

    if (signal_ctrl_peripheral_turn_on_main_tank_water_output == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_TANK_WATER_OUTPUT(true);
    }

    if (signal_ctrl_peripheral_turn_off_main_tank_water_output == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_TANK_WATER_OUTPUT(false);
    }

    if (signal_ctrl_water_temperature_activate_get_wtemp == SENSOR_IC_SIGNAL_TRUE)
    {
        signal_data_get_water_temperature_data = NORMAL_OP_GET_WATER_TEMPERATURE();
    }

    SIGNALS_unlock();

    // UI_DATA_OUT acquires/snapshots or transmits DWIN frames over UART
    SIGNALS_lock();
    UI_DATA_OUT();
    SIGNALS_unlock();
}

void scheduler_step_10ms(void)
{
    SIGNALS_lock();

    if (signal_ctrl_rtc_activate_get_rtc == SENSOR_IC_SIGNAL_TRUE)
    {
        signal_data_get_rtc_data = NORMAL_OP_GET_RTC();
        signal_ctrl_rtc_activate_get_rtc = SENSOR_IC_SIGNAL_FALSE;
    }

    if (signal_ctrl_rtc_activate_set_rtc == SENSOR_IC_SIGNAL_TRUE)
    {
        NORMAL_OP_SET_RTC(signal_data_set_rtc_data);
        signal_ctrl_rtc_activate_set_rtc = SENSOR_IC_SIGNAL_FALSE;
        signal_ctrl_rtc_activate_get_rtc = SENSOR_IC_SIGNAL_TRUE;
    }

    PUMP_IO_COM_EXE();

    SIGNALS_unlock();
}

void scheduler_step_15ms(void)
{
    SIGNALS_lock();

    NORMAL_OP_EC_CALIBRATION();
    NORMAL_OP_PH_CALIBRATION();
    WL_EXE_APPLICATION();
    EC_EXE_APPLICATION();
    PH_EXE_APPLICATION();
    LED_CTRL_EXE_APPLICATION();
    PIPE_WATER_CTRL_EXE_APPLICATION();

    SIGNALS_unlock();
}

void scheduler_step_20ms(void)
{
    SIGNALS_lock();
    UI_EXE();
    SIGNALS_unlock();
}

void scheduler_step_25ms(void)
{
    SIGNALS_lock();

    // Poll RTC every 4 x 135 ms (~540 ms) for 1-second dosing timer accuracy
    if (task_25_ms_rtc_counter < 3U)
    {
        task_25_ms_rtc_counter++;
    }
    else
    {
        task_25_ms_rtc_counter = 0U;
        signal_ctrl_rtc_activate_get_rtc = SENSOR_IC_SIGNAL_TRUE;
    }

    MAIN_FRUIT_EXE();

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

    // Dirty-checked + rate-limited NVS persistence (protects flash endurance)
    save_signal_data_if_changed(&nvs_signal_data);

    SIGNALS_unlock();
}

void task_1ms(void *arg) 
{
    (void)arg;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        scheduler_step_1ms();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
    }
}

void task_3ms(void *arg) 
{
    (void)arg;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        scheduler_step_3ms();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(25));
    }
}

void task_5ms(void *arg) 
{
    (void)arg;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        scheduler_step_5ms();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(45));
    }
}

void task_10ms(void *arg) 
{
    (void)arg;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        scheduler_step_10ms();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(65));
    }
}

void task_15ms(void *arg) 
{
    (void)arg;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        scheduler_step_15ms();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(85));
    }
}

void task_20ms(void *arg) 
{
    (void)arg;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        scheduler_step_20ms();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(110));
    }
}

void task_25ms(void *arg) 
{
    (void)arg;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        scheduler_step_25ms();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(135));
    }
}
