#include "../inc/normal_op.h"
#include "../../NOM_STACK/Services/PUMP_IO_COM/inc/pump_io_com.h"
#include "../../NOM_STACK/Services/I2C_COM/inc/i2c_com.h"
#include "../../NOM_STACK/Sensors/PH_sensor/inc/ph_sensor.h"
#include "../../NOM_STACK/Sensors/EC_sensor/inc/ec_sensor.h"
#include "../../NOM_STACK/Sensors/WL_sensor/inc/wl_sensor.h"
#include "../../NOM_STACK/Sensors/WTEMP_sensor/inc/wtemp_sensor.h"
#include "../../NOM_STACK/Sensors/RealTimeClock/inc/real_time_clock.h"

#include "../../NOM_STACK/Application/WL/inc/wl_application.h"

//signals
#include "../../NOM_STACK/Signals/Sensors/inc/sensor_ic_signals.h"


void NORMAL_OP_GET_I2C_COM_PH_EC_SENSOR_DATA(void)
{
    // normal_op_i2c_com_ph_ec_sensor_data = I2C_COM_GET_SENSOR_DATA();
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_GET_I2C_COM_PH_EC_SENSOR_DATA...");
#endif
    return;
}

void NORMAL_OP_GET_PH_EC_SENSORS(float *ph_data, float *ec_data)
{
    *ph_data = PH_SENSOR_READ();
    *ec_data = EC_SENSOR_READ();
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_GET_PH_EC_SENSORS...");
#endif
    return;
}

DS1302_DateTime *NORMAL_OP_GET_RTC(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_GET_RTC...");
#endif
    return RTC_GET_DATA();  
}

void NORMAL_OP_SET_RTC(DS1302_DateTime *rtc_data)
{
    RTC_SET_DATA(rtc_data);
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_SET_RTC...");
#endif
    return;  
}

float NORMAL_OP_GET_WATER_TEMPERATURE(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_GET_WATER_TEMPERATURE...");
#endif
    return WTEMP_SENSOR_GET_DATA();
}

float NORMAL_OP_GET_WATER_LEVEL(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_GET_WATER_LEVEL...");
#endif
    return WL_SENSOR_GET_DISTANCE_CM();
}

float NORMAL_OP_GET_WATER_LEVEL_PERCENTAGE(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_GET_WATER_LEVEL...");
#endif
    return WL_GET_PERCENTAGE();
}

void NORMAL_OP_SET_PH_EC_PUMPS(bool ph_pump_state, bool ec_pump_state)
{
    //mix pump is not defined yet
    if(ph_pump_state == true)
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_PH_PUMPS, COMMAND_ON_OR_UP);
    }else
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_PH_PUMPS, COMMAND_PH_UP_OFF);
        // PUMP_IO_COM_send_pump_command(INTERFACE_PH_PUMPS, COMMAND_PH_DOWN_OFF);
    }

    if(ec_pump_state == true)
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_EC_PUMPS, COMMAND_ON_OR_UP);
    }else
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_EC_PUMPS, COMMAND_EC_UP_OFF);
        // PUMP_IO_COM_send_pump_command(INTERFACE_EC_PUMPS, COMMAND_EC_UP_OFF);
    }
    //assign the values to signals 
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_SET_PH_EC_PUMPS...");
#endif
    return;
}

void NORMAL_OP_SET_MAIN_PUMP(bool main_pump_state)
{
    if(main_pump_state == true)
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_P_PUMP, COMMAND_ON_OR_UP);
    }else
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_P_PUMP, COMMAND_OFF_OR_DOWN);
    }
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_SET_MAIN_PUMP...");
#endif
    return;
}

void NORMAL_OP_SET_CIRCULATION_PUMPS(bool circulation_pumps_state)
{
    if(circulation_pumps_state == true)
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_OXY_PUMPS, COMMAND_ON_OR_UP);
    }else
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_OXY_PUMPS, COMMAND_OFF_OR_DOWN);
    }
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_SET_CIRCULATION_PUMPS...");
#endif
    return;
}

void NORMAL_OP_SET_MIXER_MOTORS(bool mixer_motors_state)
{
    if(mixer_motors_state == true)
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_MIX_PUMP, COMMAND_ON_OR_UP);
    }else
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_MIX_PUMP, COMMAND_OFF_OR_DOWN);
    }
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_SET_CIRCULATION_PUMPS...");
#endif
    return;   
}

void NORMAL_OP_SET_TANK_WATER_INPUT(bool tank_water_input_state)
{
    if(tank_water_input_state == true)
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_M_IN_VALVE, COMMAND_ON_OR_UP);
    }else
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_M_IN_VALVE, COMMAND_OFF_OR_DOWN);
    }
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_SET_TANK_WATER_INPUT...");
#endif
    return;
}

void NORMAL_OP_SET_TANK_WATER_OUTPUT(bool tank_water_output_state)
{
    if(tank_water_output_state == true)
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_M_OUT_VALVE, COMMAND_ON_OR_UP);
    }else
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_M_OUT_VALVE, COMMAND_OFF_OR_DOWN);
    }
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_SET_TANK_WATER_OUTPUT...");
#endif
    return;
}

float NORMAL_OP_GET_ROOM_TEMPERATURE(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_GET_ROOM_TEMPERATURE...");
#endif
    return 0.0f;
}

float NORMAL_OP_GET_ROOM_HUMIDITY(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_GET_ROOM_HUMIDITY...");
#endif
    return 0.0f;
}

void NORMAL_OP_SET_ROOM_AIR_CONDITIONER(float room_air_conditioner_data)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_SET_ROOM_AIR_CONDITIONER...");
#endif
    return;
}

void NORMAL_OP_SET_ROOM_LEDS(bool room_leds_state)
{
    if(room_leds_state == true)
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_GROW_LEDS, COMMAND_ON_OR_UP);
    }else
    {
        PUMP_IO_COM_send_pump_command(INTERFACE_GROW_LEDS, COMMAND_OFF_OR_DOWN);
    }
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_SET_ROOM_LEDS...");
#endif
    return;
}

void NORMAL_OP_SET_PIPE_VENTILATION(bool pipe_ventilation_state)
{
    //ventilation will be same with leds so delete but for now leave here for any case
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_SET_PIPE_VENTILATION...");
#endif
    return;
}


void NORMAL_OP_EC_PID(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_EC_PID...");
#endif
    return;
}

void NORMAL_OP_PH_PID(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_PH_PID...");
#endif
    return;
}

void NORMAL_OP_PH_CALIBRATION(void)
{
    //check all signals if are true and then lineer regression
    if( signal_ctrl_system_cal_ph_low == SENSOR_IC_SIGNAL_TRUE && 
        signal_ctrl_system_cal_ph_mid == SENSOR_IC_SIGNAL_TRUE &&
        signal_ctrl_system_cal_ph_high == SENSOR_IC_SIGNAL_TRUE)
        {
            NORMAL_OP_PH_CALIBRATION_CALCULATE_LIN_REGRESSION_VALUES();
            signal_ctrl_system_cal_ph_low = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_cal_ph_mid = SENSOR_IC_SIGNAL_FALSE;
            signal_ctrl_system_cal_ph_high = SENSOR_IC_SIGNAL_FALSE;
        }
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_PH_CALIBRATION...");
#endif
    return;
}

void NORMAL_OP_EC_CALIBRATION(void)
{
    //check only for mid
    if( signal_ctrl_system_cal_ec_mid == SENSOR_IC_SIGNAL_TRUE )
    {
        NORMAL_OP_EC_CALIBRATION_CALCULATE_LIN_REGRESSION_VALUES();
        signal_ctrl_system_cal_ec_mid = SENSOR_IC_SIGNAL_FALSE;
    }
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_EC_CALIBRATION...");
#endif
    return;
}


void NORMAL_OP_PH_CALIBRATION_SET_LOW(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_PH_CALIBRATION_SET_LOW...");
#endif
    return;
}

void NORMAL_OP_EC_CALIBRATION_SET_LOW(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_EC_CALIBRATION_SET_LOW...");
#endif
    return;
}

void NORMAL_OP_PH_CALIBRATION_SET_MID(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_PH_CALIBRATION_SET_MID...");
#endif
    return;
}

void NORMAL_OP_EC_CALIBRATION_SET_MID(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_EC_CALIBRATION_SET_MID...");
#endif
    return;
}

void NORMAL_OP_PH_CALIBRATION_SET_HIGH(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_PH_CALIBRATION_SET_HIGH...");
#endif
    return;
}

void NORMAL_OP_EC_CALIBRATION_SET_HIGH(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_EC_CALIBRATION_SET_HIGH...");
#endif
    return;
}


float NORMAL_OP_PH_CALIBRATION_GET_VOLTAGE(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_PH_CALIBRATION_GET_VOLTAGE...");
#endif
    return 0.0f;
}

float NORMAL_OP_EC_CALIBRATION_GET_VOLTAGE(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_EC_CALIBRATION_GET_VOLTAGE...");
#endif
    return 0.0f;
}


void NORMAL_OP_PH_CALIBRATION_CALCULATE_LIN_REGRESSION_VALUES(void)
{
    PH_SENSOR_CALCULATE_LIN_REGRESSUION();
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_PH_CALIBRATION_CALCULATE_LIN_REGRESSION_VALUES...");
#endif
    return;
}

void NORMAL_OP_EC_CALIBRATION_CALCULATE_LIN_REGRESSION_VALUES(void)
{
#ifdef NORMAL_OP_LOGS
    ESP_LOGI(NORMAL_OP_TAG, "NORMAL_OP_EC_CALIBRATION_CALCULATE_LIN_REGRESSION_VALUES...");
#endif
    return;
}