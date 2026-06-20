#include "../inc/initialization.h"
#include "../../NOM_STACK/Services/PUMP_IO_COM/inc/pump_io_com.h"
#include "../../NOM_STACK/Services/I2C_COM/inc/i2c_com.h"
#include "../../NOM_STACK/Sensors/PH_sensor/inc/ph_sensor.h"
#include "../../NOM_STACK/Sensors/EC_sensor/inc/ec_sensor.h"
#include "../../NOM_STACK/Sensors/WL_sensor/inc/wl_sensor.h"
#include "../../NOM_STACK/Sensors/WTEMP_sensor/inc/wtemp_sensor.h"
#include "../../NOM_STACK/UI/DWIN_protocol/inc/dwin_protocol.h"
#include "../../NOM_STACK/Sensors/RealTimeClock/inc/real_time_clock.h"
#include "../../NOM_STACK/Services/PUMP_IO_COM/inc/pump_io_com.h"
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"

void INIT_PH_EC_SENSORS(void)
{
    // I2C_COM_INIT(); moved to pump_io_com
    PH_SENSOR_INIT(); //for lineer regression
    EC_SENSOR_INIT();
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_PH_EC_SENSORS...");
#endif
    return;
}

void INIT_RTC(void)
{
    RTC_INIT();
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_RTC...");
#endif
    return;
}

void INIT_WATER_TEMPERATURE(void)
{
    WTEMP_SENSOR_INIT();
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_WATER_TEMPERATURE...");
#endif
    return;
}

void INIT_WATER_LEVEL(void)
{
    WL_SENSOR_INIT();
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_WATER_LEVEL...");
#endif
    return;
}

void INIT_PH_EC_PUMPS(void)
{
    PUMP_IO_COM_INIT_the_main_pump_ctrl(); //all peripherals initialized
    PUMP_IO_COM_send_pump_command(INTERFACE_EC_PUMPS, COMMAND_EC_UP_OFF);
    PUMP_IO_COM_send_pump_command(INTERFACE_PH_PUMPS, COMMAND_PH_UP_OFF);
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_PH_EC_PUMPS...");
#endif
    return;
}

void INIT_MAIN_PUMP(void)
{
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_MAIN_PUMP...");
#endif
    return;
}

void INIT_CIRCULATION_PUMPS(void)
{
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_CIRCULATION_PUMPS...");
#endif
    return;
}

void INIT_TANK_WATER_INPUT(void)
{
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_TANK_WATER_INPUT...");
#endif
    return;
}

void INIT_TANK_WATER_OUTPUT(void)
{
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_TANK_WATER_OUTPUT...");
#endif
    return;
}

void INIT_MIXER_MOTORS(void)
{
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_MIXER_MOTORS...");
#endif
    return;
}

void INIT_ROOM_TEMPERATURE(void)
{
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_ROOM_TEMPERATURE...");
#endif
    return;
}

void INIT_ROOM_HUMIDITY(void)
{
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_ROOM_HUMIDITY...");
#endif
    return;
}

void INIT_ROOM_AIR_CONDITIONER(void)
{
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_ROOM_AIR_CONDITIONER...");
#endif
    return;
}

void INIT_ROOM_LEDS(void)
{
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_ROOM_LEDS...");
#endif
    return;
}

void INIT_PIPE_VENTILATION(void)
{
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_PIPE_VENTILATION...");
#endif
    return;
}

void INIT_WIFI(void)
{
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_WIFI...");
#endif
    return;
}

void INIT_BLE(void)
{
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_BLE...");
#endif
    return;
}

void INIT_UI(void)
{
    UI_INIT();
    // ESP_LOGI(INIT_TAG, "INIT_UI...");
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_UI...");
#endif
    return;
}

void INIT_PERIPHERALS(void)
{
    INIT_PH_EC_SENSORS();
    INIT_WATER_TEMPERATURE();
    INIT_WATER_LEVEL();
    INIT_PH_EC_PUMPS();
    INIT_MAIN_PUMP();
    INIT_RTC();
    INIT_UI();
    INIT_CIRCULATION_PUMPS();
    INIT_TANK_WATER_INPUT();
    INIT_TANK_WATER_OUTPUT();
    INIT_ROOM_TEMPERATURE();
    INIT_ROOM_HUMIDITY();
    INIT_ROOM_AIR_CONDITIONER();
    INIT_ROOM_LEDS();
    INIT_PIPE_VENTILATION();
    SENSORS_INIT_SIGNALS();
#ifdef INIT_LOGS
    ESP_LOGI(INIT_TAG, "INIT_PERIPHERALS...");
#endif
    return;
}