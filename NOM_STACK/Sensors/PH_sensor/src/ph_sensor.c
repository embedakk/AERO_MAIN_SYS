#include "../inc/ph_sensor.h"

#define PH_DUTY_MULT_CONSTANT   0.05f

#define OVERSAMPLE 200//200     // Number of ADC readings to average
#define MEDIAN_SIZE 50//50 //25/7     // Median filter window
#define Q_PROCESS 0.005f//0.0005f // Kalman process noise
#define R_MEASURE 0.2f//0.02f   // Kalman measurement noise


// --- Filter state ---
float medianBuffer[MEDIAN_SIZE];
int medianIndex = 0;
int medianCount = 0;
float k_x = 0.0f;  // estimated voltage
float k_P = 1.0f;  // estimation covariance

uint8_t ph_sensor_counter = 0;
float ph_sensor_voltage_readings[OVERSAMPLE];

float rawV, medV, kalV;
float phValue;

sensor_ic_signal_status prev_signal_ctrl_peripheral_turn_on_main_tank_water_input = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status prev_signal_ctrl_peripheral_turn_off_main_tank_water_input = SENSOR_IC_SIGNAL_TRUE;
sensor_ic_signal_status prev_signal_ctrl_peripheral_turn_on_main_tank_water_output = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status prev_signal_ctrl_peripheral_turn_off_main_tank_water_output = SENSOR_IC_SIGNAL_TRUE;
sensor_ic_signal_status prev_signal_ctrl_led_turn_on_led = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status prev_signal_ctrl_peripheral_turn_on_main_pump = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status prev_signal_ctrl_peripheral_turn_off_main_pump = SENSOR_IC_SIGNAL_TRUE;
sensor_ic_signal_status prev_signal_ctrl_mixer_motors_turn_on = SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status prev_signal_ctrl_circulation_pumps_turn_on = SENSOR_IC_SIGNAL_FALSE;

sensor_ic_signal_status ph_sensor_signal0_active, ph_sensor_signal1_active, ph_sensor_signal2_active, 
                        ph_sensor_signal3_active, ph_sensor_signal4_active, ph_sensor_signal5_active, 
                        ph_sensor_signal6_active, ph_sensor_signal7_active, ph_sensor_signal8_active; 

uint8_t                 ph_sensor_deactive_time_tick = 0;
uint8_t                 ph_sensor_deactive_time_counter = 0;
// ----- Utility: simple insertion sort -----
void sortArray(float *arr, int n) {
  for (int i = 1; i < n; i++) {
    float key = arr[i];
    int j = i - 1;
    while (j >= 0 && arr[j] > key) {
      arr[j + 1] = arr[j];
      j--;
    }
    arr[j + 1] = key;
  }
}

float readPHVoltage() {
  float sum = 0;
  for (int i = 0; i < OVERSAMPLE; i++) 
  {
    sum += ph_sensor_voltage_readings[i];
  }
  float avg = (float)(sum / ((float)OVERSAMPLE));
  return avg;// * (5.0 / 1023.0);  // convert ADC count → voltage
}

// ----- Convert voltage to pH -----
float voltageToPH(float v) {
  float ph = (ph_sensor_b * v) + ph_sensor_a;
  ph = ph + (((signal_data_get_water_temperature_data - 25.0) / 10.0) * 0.03);
  if (ph < 0.0f) ph = 0.0f;
  if (ph > 14.0f) ph = 14.0f;
  return ph;
}

// ----- 1D Kalman filter -----
float kalmanUpdate(float measurement) {
  // Predict
  float x_pred = k_x;
  float P_pred = k_P + Q_PROCESS;

  // Update
  float K = P_pred / (P_pred + R_MEASURE);
  k_x = x_pred + K * (measurement - x_pred);
  k_P = (1.0f - K) * P_pred;

  return k_x;
}

// ----- Median filter -----
float medianFilter(float v) {
  medianBuffer[medianIndex] = v;
  medianIndex = (medianIndex + 1) % MEDIAN_SIZE;
  if (medianCount < MEDIAN_SIZE) medianCount++;

  float temp[MEDIAN_SIZE];
  for (int i = 0; i < medianCount; i++) temp[i] = medianBuffer[i];
  sortArray(temp, medianCount);
  return temp[medianCount / 2];
}


void PH_SENSOR_INIT(void)
{
    //nohting while already we did it on i2c_comm
    PH_SENSOR_CALCULATE_LIN_REGRESSUION();
}

//put here the median and kalman filter
float PH_SENSOR_READ(void)
{
    //logic to not read input signals
    if(prev_signal_ctrl_peripheral_turn_on_main_tank_water_input != signal_ctrl_peripheral_turn_on_main_tank_water_input)
    {
        //dont read any ph value for 500 ms
        ph_sensor_signal0_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ph_sensor_signal0_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(prev_signal_ctrl_peripheral_turn_off_main_tank_water_input != signal_ctrl_peripheral_turn_off_main_tank_water_input)
    {
        //dont read any ph value for 500 ms
        ph_sensor_signal1_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ph_sensor_signal1_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(prev_signal_ctrl_peripheral_turn_on_main_tank_water_output != signal_ctrl_peripheral_turn_on_main_tank_water_output)
    {
        //dont read any ph value for 500 ms
        ph_sensor_signal2_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ph_sensor_signal2_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(prev_signal_ctrl_peripheral_turn_off_main_tank_water_output != signal_ctrl_peripheral_turn_off_main_tank_water_output)
    {
        //dont read any ph value for 500 ms
        ph_sensor_signal3_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ph_sensor_signal3_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(prev_signal_ctrl_led_turn_on_led != signal_ctrl_led_turn_on_led)
    {
        //dont read any ph value for 500 ms
        ph_sensor_signal4_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ph_sensor_signal4_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(prev_signal_ctrl_peripheral_turn_on_main_pump != signal_ctrl_peripheral_turn_on_main_pump)
    {
        //dont read any ph value for 500 ms
        ph_sensor_signal5_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ph_sensor_signal5_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(prev_signal_ctrl_peripheral_turn_off_main_pump != signal_ctrl_peripheral_turn_off_main_pump)
    {
        //dont read any ph value for 500 ms
        ph_sensor_signal6_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ph_sensor_signal6_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(prev_signal_ctrl_mixer_motors_turn_on != signal_ctrl_mixer_motors_turn_on)
    {
        //dont read any ph value for 500 ms
        ph_sensor_signal7_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ph_sensor_signal7_active = SENSOR_IC_SIGNAL_FALSE;
    }
    if(prev_signal_ctrl_circulation_pumps_turn_on != signal_ctrl_circulation_pumps_turn_on)
    {
        //dont read any ph value for 500 ms
        ph_sensor_signal8_active = SENSOR_IC_SIGNAL_TRUE;
    }else
    {
        ph_sensor_signal8_active = SENSOR_IC_SIGNAL_FALSE;
    }

    if(ph_sensor_deactive_time_counter < ph_sensor_deactive_time_tick)
    {
        ph_sensor_deactive_time_counter++;
    }else
    {
        ph_sensor_deactive_time_counter = 0;
        if(ph_sensor_signal0_active == SENSOR_IC_SIGNAL_TRUE ||
           ph_sensor_signal1_active == SENSOR_IC_SIGNAL_TRUE ||
           ph_sensor_signal2_active == SENSOR_IC_SIGNAL_TRUE ||
           ph_sensor_signal3_active == SENSOR_IC_SIGNAL_TRUE ||
           ph_sensor_signal4_active == SENSOR_IC_SIGNAL_TRUE ||
           ph_sensor_signal5_active == SENSOR_IC_SIGNAL_TRUE ||
           ph_sensor_signal6_active == SENSOR_IC_SIGNAL_TRUE ||
           ph_sensor_signal7_active == SENSOR_IC_SIGNAL_TRUE ||
           ph_sensor_signal8_active == SENSOR_IC_SIGNAL_TRUE)
        {
            // printf("TICK ACT\r\n");
            //put the passive counter here
            ph_sensor_deactive_time_tick = 50;
        }else
        {
            ph_sensor_deactive_time_tick = 0;
            //read ph values only here
            if(ph_sensor_counter < OVERSAMPLE)
            {
                ph_sensor_voltage_value = (pump_io_get_ph_duty_V_float_data * PH_DUTY_MULT_CONSTANT);/// (float)PH_SENSOR_ADC_MAX_READING) * PH_SENSOR_ADC_REF_VOLTAGE;
                ph_sensor_voltage_readings[ph_sensor_counter] = ph_sensor_voltage_value;
                    // 1. Read raw voltage (oversampled)
                rawV = readPHVoltage();
                // printf("%d RAW V : %f RV %f\r\n",ph_sensor_counter, rawV, ph_sensor_voltage_value);

                // 2. Median filter
                medV = medianFilter(rawV);

                // 3. Kalman filter
                kalV = kalmanUpdate(medV);
                // kalV = medV;
                signal_data_set_ph_voltage = kalV;

                // 4. Convert to pH
                phValue = voltageToPH(kalV);
                ph_sensor_counter++;
            }else
            {
                ph_sensor_counter = 0;
            }
        }
    }

    ph_sensor_ph_value = phValue;

    prev_signal_ctrl_peripheral_turn_on_main_tank_water_input = signal_ctrl_peripheral_turn_on_main_tank_water_input;
    prev_signal_ctrl_peripheral_turn_off_main_tank_water_input = signal_ctrl_peripheral_turn_off_main_tank_water_input;
    prev_signal_ctrl_peripheral_turn_on_main_tank_water_output = signal_ctrl_peripheral_turn_on_main_tank_water_output;
    prev_signal_ctrl_peripheral_turn_off_main_tank_water_output = signal_ctrl_peripheral_turn_off_main_tank_water_output;
    prev_signal_ctrl_led_turn_on_led = signal_ctrl_led_turn_on_led;
    prev_signal_ctrl_peripheral_turn_on_main_pump = signal_ctrl_peripheral_turn_on_main_pump;
    prev_signal_ctrl_peripheral_turn_off_main_pump = signal_ctrl_peripheral_turn_off_main_pump;
    prev_signal_ctrl_mixer_motors_turn_on = signal_ctrl_mixer_motors_turn_on;
    prev_signal_ctrl_circulation_pumps_turn_on = signal_ctrl_circulation_pumps_turn_on;

#ifdef PH_SENSOR_LOGS
    ESP_LOGI(PH_SENSOR_TAG, "PH_VALUE : %f", ph_sensor_ph_value);
#endif
    return ph_sensor_ph_value; 
}

//water flow rate to the small tank 1-5 L/min
void PH_SENSOR_CALCULATE_LIN_REGRESSUION(void)
{
    ph_sensor_sumX = 0; 
    ph_sensor_sumX2 = 0; 
    ph_sensor_sumY = 0;
    ph_sensor_sumXY = 0; 
    ph_sensor_a = 0; 
    ph_sensor_b = 0;
    ph_sensor_n = PH_SENSOR_LR_SAMPLE_NR;
    ph_sensor_x[1] = signal_data_set_ph_cal_high_V;     //2.58; //these values will be assigned also on the real time with real time signals
    ph_sensor_x[2] = signal_data_set_ph_cal_mid_V;      //3.01; 
    ph_sensor_x[3] = signal_data_set_ph_cal_low_V;      //3.16;
    ph_sensor_y[1] = PH_SENSOR_HIGH_SOLUTION; 
    ph_sensor_y[2] = PH_SENSOR_MID_SOLUTION; 
    ph_sensor_y[3] = PH_SENSOR_LOW_SOLUTION; 

    for(ph_sensor_cnt=1; ph_sensor_cnt <= ph_sensor_n; ph_sensor_cnt++)
    {
        ph_sensor_sumX = ph_sensor_sumX + ph_sensor_x[ph_sensor_cnt];
        ph_sensor_sumX2 = ph_sensor_sumX2 + (ph_sensor_x[ph_sensor_cnt] * ph_sensor_x[ph_sensor_cnt]);
        ph_sensor_sumY = ph_sensor_sumY + ph_sensor_y[ph_sensor_cnt];
        ph_sensor_sumXY = ph_sensor_sumXY + (ph_sensor_x[ph_sensor_cnt] * ph_sensor_y[ph_sensor_cnt]);
    }

    ph_sensor_b = ((ph_sensor_n * ph_sensor_sumXY)-(ph_sensor_sumX * ph_sensor_sumY))/((ph_sensor_n * ph_sensor_sumX2)-(ph_sensor_sumX * ph_sensor_sumX));
    ph_sensor_a = (ph_sensor_sumY - (ph_sensor_b * ph_sensor_sumX)) / ph_sensor_n;

#ifdef PH_SENSOR_LOGS
    ESP_LOGI(PH_SENSOR_TAG, "Values are: a=%0.2f and b = %0.2f", ph_sensor_a, ph_sensor_b);
#endif
}