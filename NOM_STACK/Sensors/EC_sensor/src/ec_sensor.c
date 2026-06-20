#include "../inc/ec_sensor.h"

#define PH_DUTY_MULT_CONSTANT 0.05
#define OVERSAMPLE 50//200     // Number of ADC readings to average

static sensor_ic_signal_status ec_prev_signal_ctrl_peripheral_turn_on_main_tank_water_input = SENSOR_IC_SIGNAL_FALSE;
static sensor_ic_signal_status ec_prev_signal_ctrl_peripheral_turn_off_main_tank_water_input = SENSOR_IC_SIGNAL_TRUE;
static sensor_ic_signal_status ec_prev_signal_ctrl_peripheral_turn_on_main_tank_water_output = SENSOR_IC_SIGNAL_FALSE;
static sensor_ic_signal_status ec_prev_signal_ctrl_peripheral_turn_off_main_tank_water_output = SENSOR_IC_SIGNAL_TRUE;
static sensor_ic_signal_status ec_prev_signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_FALSE;
static sensor_ic_signal_status ec_prev_signal_ctrl_peripheral_turn_on_main_pump = SENSOR_IC_SIGNAL_FALSE;
static sensor_ic_signal_status ec_prev_signal_ctrl_peripheral_turn_off_main_pump = SENSOR_IC_SIGNAL_TRUE;
static sensor_ic_signal_status ec_prev_signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
static sensor_ic_signal_status ec_prev_signal_ctrl_circulation_pumps_turn_on = SENSOR_IC_SIGNAL_FALSE;

static sensor_ic_signal_status  ec_sensor_signal0_active, ec_sensor_signal1_active, ec_sensor_signal2_active, 
                                ec_sensor_signal3_active, ec_sensor_signal4_active, ec_sensor_signal5_active, 
                                ec_sensor_signal6_active, ec_sensor_signal7_active, ec_sensor_signal8_active; 

uint8_t                 ec_sensor_deactive_time_tick = 0;
uint8_t                 ec_sensor_deactive_time_counter = 0;

uint8_t ec_sensor_counter = 0;
float ec_sensor_voltage_readings[OVERSAMPLE];

void EC_SENSOR_INIT(void)
{

}

float EC_SENSOR_READ(void)
{
    //logic to not read input signals
    if(ec_prev_signal_ctrl_peripheral_turn_on_main_tank_water_input != signal_ctrl_peripheral_turn_on_main_tank_water_input)
    {
        //dont read any ph value for 500 ms
        ec_sensor_signal0_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ec_sensor_signal0_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(ec_prev_signal_ctrl_peripheral_turn_off_main_tank_water_input != signal_ctrl_peripheral_turn_off_main_tank_water_input)
    {
        //dont read any ph value for 500 ms
        ec_sensor_signal1_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ec_sensor_signal1_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(ec_prev_signal_ctrl_peripheral_turn_on_main_tank_water_output != signal_ctrl_peripheral_turn_on_main_tank_water_output)
    {
        //dont read any ph value for 500 ms
        ec_sensor_signal2_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ec_sensor_signal2_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(ec_prev_signal_ctrl_peripheral_turn_off_main_tank_water_output != signal_ctrl_peripheral_turn_off_main_tank_water_output)
    {
        //dont read any ph value for 500 ms
        ec_sensor_signal3_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ec_sensor_signal3_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(ec_prev_signal_ctrl_led_turn_on_led != signal_ctrl_led_turn_on_led)
    {
        //dont read any ph value for 500 ms
        ec_sensor_signal4_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ec_sensor_signal4_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(ec_prev_signal_ctrl_peripheral_turn_on_main_pump != signal_ctrl_peripheral_turn_on_main_pump)
    {
        //dont read any ph value for 500 ms
        ec_sensor_signal5_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ec_sensor_signal5_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(ec_prev_signal_ctrl_peripheral_turn_off_main_pump != signal_ctrl_peripheral_turn_off_main_pump)
    {
        //dont read any ph value for 500 ms
        ec_sensor_signal6_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ec_sensor_signal6_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(ec_prev_signal_ctrl_mixer_motors_turn_on != signal_ctrl_mixer_motors_turn_on)
    {
        //dont read any ph value for 500 ms
        ec_sensor_signal7_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ec_sensor_signal7_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(ec_prev_signal_ctrl_circulation_pumps_turn_on != signal_ctrl_circulation_pumps_turn_on)
    {
        //dont read any ph value for 500 ms
        ec_sensor_signal8_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ec_sensor_signal8_active = SENSOR_IC_SIGNAL_FALSE;
    }

    if(ec_sensor_deactive_time_counter < ec_sensor_deactive_time_tick)
    {
        ec_sensor_deactive_time_counter++;
    }else
    {
        ec_sensor_deactive_time_counter = 0;
        if(ec_sensor_signal0_active == SENSOR_IC_SIGNAL_TRUE ||
           ec_sensor_signal1_active == SENSOR_IC_SIGNAL_TRUE ||
           ec_sensor_signal2_active == SENSOR_IC_SIGNAL_TRUE ||
           ec_sensor_signal3_active == SENSOR_IC_SIGNAL_TRUE ||
           ec_sensor_signal4_active == SENSOR_IC_SIGNAL_TRUE ||
           ec_sensor_signal5_active == SENSOR_IC_SIGNAL_TRUE ||
           ec_sensor_signal6_active == SENSOR_IC_SIGNAL_TRUE ||
           ec_sensor_signal7_active == SENSOR_IC_SIGNAL_TRUE ||
           ec_sensor_signal8_active == SENSOR_IC_SIGNAL_TRUE)
        {
            // printf("TICK ACT\r\n");
            //put the passive counter here
            ec_sensor_deactive_time_tick = 50;
        }else
        {
            ec_sensor_deactive_time_tick = 0;
            //read ec values only here
            if(ec_sensor_counter < OVERSAMPLE)
            {
                // ec_sensor_voltage_value = (i2c_com_ec_data / (float)EC_SENSOR_ADC_MAX_READING) * EC_SENSOR_ADC_REF_VOLTAGE;
                // ec_sensor_voltage_value = (pump_io_get_ec_duty_V_float_data * PH_DUTY_MULT_CONSTANT);// (float)EC_SENSOR_ADC_MAX_READING) * EC_SENSOR_ADC_REF_VOLTAGE;
                ec_sensor_voltage_readings[ec_sensor_counter] = (pump_io_get_ec_duty_V_float_data * PH_DUTY_MULT_CONSTANT);
                ec_sensor_counter++;
            }else
            {
                //calculate average now
                float sum = 0;
                for (int i = 0; i < OVERSAMPLE; i++) 
                {
                    sum += ec_sensor_voltage_readings[i];
                }
                ec_sensor_voltage_value = (float)(sum / ((float)OVERSAMPLE));
                signal_data_set_ec_voltage = ec_sensor_voltage_value;
                ec_sensor_ec_value = (133.42 * ec_sensor_voltage_value * ec_sensor_voltage_value * ec_sensor_voltage_value - 
                255.86 * ec_sensor_voltage_value * ec_sensor_voltage_value +
                857.39 * ec_sensor_voltage_value) * 0.5; // 0.5 is the TDS factor // momve to 0.4 or 0.45 2300 - 2500 mS/cm 
                ec_sensor_ec_value = ec_sensor_ec_value + (((signal_data_get_water_temperature_data - 25.0) / 10.0) * 0.02);
                ec_sensor_ec_value = ec_sensor_ec_value;// / 500; // get us with higher value like 100us
                ec_sensor_counter = 0;
            }
        }
    }

    ec_prev_signal_ctrl_peripheral_turn_on_main_tank_water_input = signal_ctrl_peripheral_turn_on_main_tank_water_input;
    ec_prev_signal_ctrl_peripheral_turn_off_main_tank_water_input = signal_ctrl_peripheral_turn_off_main_tank_water_input;
    ec_prev_signal_ctrl_peripheral_turn_on_main_tank_water_output = signal_ctrl_peripheral_turn_on_main_tank_water_output;
    ec_prev_signal_ctrl_peripheral_turn_off_main_tank_water_output = signal_ctrl_peripheral_turn_off_main_tank_water_output;
    ec_prev_signal_ctrl_led_turn_on_led = signal_ctrl_led_turn_on_led;
    ec_prev_signal_ctrl_peripheral_turn_on_main_pump = signal_ctrl_peripheral_turn_on_main_pump;
    ec_prev_signal_ctrl_peripheral_turn_off_main_pump = signal_ctrl_peripheral_turn_off_main_pump;
    ec_prev_signal_ctrl_mixer_motors_turn_on = signal_ctrl_mixer_motors_turn_on;
    ec_prev_signal_ctrl_circulation_pumps_turn_on = signal_ctrl_circulation_pumps_turn_on;

    return ec_sensor_ec_value;
}

void EC_SENSOR_CALCULATE_LIN_REGRESSUION(void)
{

}