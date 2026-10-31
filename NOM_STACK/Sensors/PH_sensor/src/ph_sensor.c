#include "../inc/ph_sensor.h"

#define PH_DUTY_MULT_CONSTANT 0.05f

#define OVERSAMPLE 200   // 200     // Number of ADC readings to average
#define MEDIAN_SIZE 50   // 50 //25/7     // Median filter window
#define Q_PROCESS 0.005f // 0.0005f // Kalman process noise
#define R_MEASURE 0.2f   // 0.02f   // Kalman measurement noise

int ph_sensor_n = 0, ph_sensor_cnt = 0;
float ph_sensor_x[PH_SENSOR_BUFFER_LEN] = {0};
float ph_sensor_y[PH_SENSOR_BUFFER_LEN] = {0};
float ph_sensor_sumX = 0.0f, ph_sensor_sumX2 = 0.0f, ph_sensor_sumY = 0.0f,
      ph_sensor_sumXY = 0.0f;
float ph_sensor_a = 0.0f, ph_sensor_b = 0.0f;

float ph_sensor_voltage_value = 0.0f;
float ph_sensor_ph_value = 0.0f;

// --- Filter state ---
float medianBuffer[MEDIAN_SIZE];
int medianIndex = 0;
int medianCount = 0;
float k_x = 0.0f; // estimated voltage
float k_P = 1.0f; // estimation covariance
static bool ph_kalman_seeded = false;

uint8_t ph_sensor_counter = 0;
static uint16_t ph_sensor_valid_samples = 0;
float ph_sensor_voltage_readings[OVERSAMPLE];

float rawV = 0.0f, medV = 0.0f, kalV = 0.0f;
float phValue = 0.0f;

sensor_ic_signal_status
    prev_signal_ctrl_peripheral_turn_on_main_tank_water_input =
        SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status
    prev_signal_ctrl_peripheral_turn_off_main_tank_water_input =
        SENSOR_IC_SIGNAL_TRUE;
sensor_ic_signal_status
    prev_signal_ctrl_peripheral_turn_on_main_tank_water_output =
        SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status
    prev_signal_ctrl_peripheral_turn_off_main_tank_water_output =
        SENSOR_IC_SIGNAL_TRUE;
sensor_ic_signal_status prev_signal_ctrl_led_turn_on_led =
    SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status prev_signal_ctrl_peripheral_turn_on_main_pump =
    SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status prev_signal_ctrl_peripheral_turn_off_main_pump =
    SENSOR_IC_SIGNAL_TRUE;
sensor_ic_signal_status prev_signal_ctrl_mixer_motors_turn_on =
    SENSOR_IC_SIGNAL_FALSE;
sensor_ic_signal_status prev_signal_ctrl_circulation_pumps_turn_on =
    SENSOR_IC_SIGNAL_FALSE;

sensor_ic_signal_status ph_sensor_signal0_active, ph_sensor_signal1_active,
    ph_sensor_signal2_active, ph_sensor_signal3_active,
    ph_sensor_signal4_active, ph_sensor_signal5_active,
    ph_sensor_signal6_active, ph_sensor_signal7_active,
    ph_sensor_signal8_active;

uint8_t ph_sensor_deactive_time_tick = 0;
uint8_t ph_sensor_deactive_time_counter = 0;

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

float readPHVoltage(void) {
  if (ph_sensor_valid_samples == 0U) {
    return 0.0f;
  }
  float sum = 0.0f;
  for (uint16_t i = 0; i < ph_sensor_valid_samples; i++) {
    sum += ph_sensor_voltage_readings[i];
  }
  float avg = (float)(sum / ((float)ph_sensor_valid_samples));
  return avg;
}

// ----- Convert voltage to pH -----
float voltageToPH(float v) {
  float safe_temp = signal_data_get_water_temperature_data;
  if (!isfinite(safe_temp) || safe_temp < 0.0f || safe_temp > 60.0f) {
    safe_temp = 25.0f;
  }
  float safe_v = isfinite(v) ? v : 0.0f;
  float ph = (ph_sensor_b * safe_v) + ph_sensor_a;
  ph = ph + (((safe_temp - 25.0f) / 10.0f) * 0.03f);
  if (!isfinite(ph) || ph < 0.0f)
    ph = 0.0f;
  if (ph > 14.0f)
    ph = 14.0f;
  return ph;
}

// ----- 1D Kalman filter -----
float kalmanUpdate(float measurement) {
  if (!isfinite(measurement)) {
    return k_x;
  }
  if (!ph_kalman_seeded) {
    k_x = measurement;
    k_P = R_MEASURE;
    ph_kalman_seeded = true;
    return k_x;
  }
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
  float safe_v = isfinite(v) ? v : 0.0f;
  medianBuffer[medianIndex] = safe_v;
  medianIndex = (medianIndex + 1) % MEDIAN_SIZE;
  if (medianCount < MEDIAN_SIZE)
    medianCount++;

  float temp[MEDIAN_SIZE];
  for (int i = 0; i < medianCount; i++)
    temp[i] = medianBuffer[i];
  sortArray(temp, medianCount);
  return temp[medianCount / 2];
}

void PH_SENSOR_INIT(void) {
  memset(medianBuffer, 0, sizeof(medianBuffer));
  memset(ph_sensor_voltage_readings, 0, sizeof(ph_sensor_voltage_readings));
  medianIndex = 0;
  medianCount = 0;
  k_x = 0.0f;
  k_P = 1.0f;
  ph_kalman_seeded = false;
  ph_sensor_counter = 0;
  ph_sensor_valid_samples = 0;
  ph_sensor_deactive_time_tick = 0;
  ph_sensor_deactive_time_counter = 0;
  rawV = 0.0f;
  medV = 0.0f;
  kalV = 0.0f;
  phValue = 0.0f;
  ph_sensor_ph_value = 0.0f;

  prev_signal_ctrl_peripheral_turn_on_main_tank_water_input =
      signal_ctrl_peripheral_turn_on_main_tank_water_input;
  prev_signal_ctrl_peripheral_turn_off_main_tank_water_input =
      signal_ctrl_peripheral_turn_off_main_tank_water_input;
  prev_signal_ctrl_peripheral_turn_on_main_tank_water_output =
      signal_ctrl_peripheral_turn_on_main_tank_water_output;
  prev_signal_ctrl_peripheral_turn_off_main_tank_water_output =
      signal_ctrl_peripheral_turn_off_main_tank_water_output;
  prev_signal_ctrl_led_turn_on_led = signal_ctrl_led_turn_on_led;
  prev_signal_ctrl_peripheral_turn_on_main_pump =
      signal_ctrl_peripheral_turn_on_main_pump;
  prev_signal_ctrl_peripheral_turn_off_main_pump =
      signal_ctrl_peripheral_turn_off_main_pump;
  prev_signal_ctrl_mixer_motors_turn_on = signal_ctrl_mixer_motors_turn_on;
  prev_signal_ctrl_circulation_pumps_turn_on =
      signal_ctrl_circulation_pumps_turn_on;

  PH_SENSOR_CALCULATE_LIN_REGRESSUION();
}

// put here the median and kalman filter
float PH_SENSOR_READ(void) {
  // logic to not read input signals
  if (prev_signal_ctrl_peripheral_turn_on_main_tank_water_input !=
      signal_ctrl_peripheral_turn_on_main_tank_water_input) {
    ph_sensor_signal0_active = SENSOR_IC_SIGNAL_TRUE;
  } else {
    ph_sensor_signal0_active = SENSOR_IC_SIGNAL_FALSE;
  }
  if (prev_signal_ctrl_peripheral_turn_off_main_tank_water_input !=
      signal_ctrl_peripheral_turn_off_main_tank_water_input) {
    ph_sensor_signal1_active = SENSOR_IC_SIGNAL_TRUE;
  } else {
    ph_sensor_signal1_active = SENSOR_IC_SIGNAL_FALSE;
  }
  if (prev_signal_ctrl_peripheral_turn_on_main_tank_water_output !=
      signal_ctrl_peripheral_turn_on_main_tank_water_output) {
    ph_sensor_signal2_active = SENSOR_IC_SIGNAL_TRUE;
  } else {
    ph_sensor_signal2_active = SENSOR_IC_SIGNAL_FALSE;
  }
  if (prev_signal_ctrl_peripheral_turn_off_main_tank_water_output !=
      signal_ctrl_peripheral_turn_off_main_tank_water_output) {
    ph_sensor_signal3_active = SENSOR_IC_SIGNAL_TRUE;
  } else {
    ph_sensor_signal3_active = SENSOR_IC_SIGNAL_FALSE;
  }
  if (prev_signal_ctrl_led_turn_on_led != signal_ctrl_led_turn_on_led) {
    ph_sensor_signal4_active = SENSOR_IC_SIGNAL_TRUE;
  } else {
    ph_sensor_signal4_active = SENSOR_IC_SIGNAL_FALSE;
  }
  if (prev_signal_ctrl_peripheral_turn_on_main_pump !=
      signal_ctrl_peripheral_turn_on_main_pump) {
    ph_sensor_signal5_active = SENSOR_IC_SIGNAL_TRUE;
  } else {
    ph_sensor_signal5_active = SENSOR_IC_SIGNAL_FALSE;
  }
  if (prev_signal_ctrl_peripheral_turn_off_main_pump !=
      signal_ctrl_peripheral_turn_off_main_pump) {
    ph_sensor_signal6_active = SENSOR_IC_SIGNAL_TRUE;
  } else {
    ph_sensor_signal6_active = SENSOR_IC_SIGNAL_FALSE;
  }
  if (prev_signal_ctrl_mixer_motors_turn_on !=
      signal_ctrl_mixer_motors_turn_on) {
    ph_sensor_signal7_active = SENSOR_IC_SIGNAL_TRUE;
  } else {
    ph_sensor_signal7_active = SENSOR_IC_SIGNAL_FALSE;
  }
  if (prev_signal_ctrl_circulation_pumps_turn_on !=
      signal_ctrl_circulation_pumps_turn_on) {
    ph_sensor_signal8_active = SENSOR_IC_SIGNAL_TRUE;
  } else {
    ph_sensor_signal8_active = SENSOR_IC_SIGNAL_FALSE;
  }

  if (ph_sensor_deactive_time_counter < ph_sensor_deactive_time_tick) {
    ph_sensor_deactive_time_counter++;
  } else {
    ph_sensor_deactive_time_counter = 0;
    if (ph_sensor_signal0_active == SENSOR_IC_SIGNAL_TRUE ||
        ph_sensor_signal1_active == SENSOR_IC_SIGNAL_TRUE ||
        ph_sensor_signal2_active == SENSOR_IC_SIGNAL_TRUE ||
        ph_sensor_signal3_active == SENSOR_IC_SIGNAL_TRUE ||
        ph_sensor_signal4_active == SENSOR_IC_SIGNAL_TRUE ||
        ph_sensor_signal5_active == SENSOR_IC_SIGNAL_TRUE ||
        ph_sensor_signal6_active == SENSOR_IC_SIGNAL_TRUE ||
        ph_sensor_signal7_active == SENSOR_IC_SIGNAL_TRUE ||
        ph_sensor_signal8_active == SENSOR_IC_SIGNAL_TRUE) {
      ph_sensor_deactive_time_tick = 50;
    } else {
      ph_sensor_deactive_time_tick = 0;
      // read ph values only here
      if (ph_sensor_counter < OVERSAMPLE) {
        float safe_duty = isfinite(pump_io_get_ph_duty_V_float_data)
                              ? pump_io_get_ph_duty_V_float_data
                              : 0.0f;
        ph_sensor_voltage_value = (safe_duty * PH_DUTY_MULT_CONSTANT);
        ph_sensor_voltage_readings[ph_sensor_counter] = ph_sensor_voltage_value;
        if (ph_sensor_valid_samples < OVERSAMPLE) {
          ph_sensor_valid_samples++;
        }
        // 1. Read raw voltage (oversampled)
        rawV = readPHVoltage();

        // 2. Median filter
        medV = medianFilter(rawV);

        // 3. Kalman filter
        kalV = kalmanUpdate(medV);
        signal_data_set_ph_voltage = kalV;

        // 4. Convert to pH
        phValue = voltageToPH(kalV);
        ph_sensor_counter++;
      } else {
        ph_sensor_counter = 0;
      }
    }
  }

  ph_sensor_ph_value = phValue;

  prev_signal_ctrl_peripheral_turn_on_main_tank_water_input =
      signal_ctrl_peripheral_turn_on_main_tank_water_input;
  prev_signal_ctrl_peripheral_turn_off_main_tank_water_input =
      signal_ctrl_peripheral_turn_off_main_tank_water_input;
  prev_signal_ctrl_peripheral_turn_on_main_tank_water_output =
      signal_ctrl_peripheral_turn_on_main_tank_water_output;
  prev_signal_ctrl_peripheral_turn_off_main_tank_water_output =
      signal_ctrl_peripheral_turn_off_main_tank_water_output;
  prev_signal_ctrl_led_turn_on_led = signal_ctrl_led_turn_on_led;
  prev_signal_ctrl_peripheral_turn_on_main_pump =
      signal_ctrl_peripheral_turn_on_main_pump;
  prev_signal_ctrl_peripheral_turn_off_main_pump =
      signal_ctrl_peripheral_turn_off_main_pump;
  prev_signal_ctrl_mixer_motors_turn_on = signal_ctrl_mixer_motors_turn_on;
  prev_signal_ctrl_circulation_pumps_turn_on =
      signal_ctrl_circulation_pumps_turn_on;

#ifdef PH_SENSOR_LOGS
  ESP_LOGI(PH_SENSOR_TAG, "PH_VALUE : %f", ph_sensor_ph_value);
#endif
  return ph_sensor_ph_value;
}

// water flow rate to the small tank 1-5 L/min
void PH_SENSOR_CALCULATE_LIN_REGRESSUION(void) {
  ph_sensor_sumX = 0.0f;
  ph_sensor_sumX2 = 0.0f;
  ph_sensor_sumY = 0.0f;
  ph_sensor_sumXY = 0.0f;
  ph_sensor_a = 0.0f;
  ph_sensor_b = 0.0f;
  ph_sensor_n = PH_SENSOR_LR_SAMPLE_NR;

  float v_high = isfinite(signal_data_set_ph_cal_high_V)
                     ? signal_data_set_ph_cal_high_V
                     : 2.58f;
  float v_mid = isfinite(signal_data_set_ph_cal_mid_V)
                    ? signal_data_set_ph_cal_mid_V
                    : 3.01f;
  float v_low = isfinite(signal_data_set_ph_cal_low_V)
                    ? signal_data_set_ph_cal_low_V
                    : 3.16f;

  ph_sensor_x[1] = v_high; // 2.58
  ph_sensor_x[2] = v_mid;  // 3.01
  ph_sensor_x[3] = v_low;  // 3.16
  ph_sensor_y[1] = (float)PH_SENSOR_HIGH_SOLUTION;
  ph_sensor_y[2] = (float)PH_SENSOR_MID_SOLUTION;
  ph_sensor_y[3] = (float)PH_SENSOR_LOW_SOLUTION;

  for (ph_sensor_cnt = 1; ph_sensor_cnt <= ph_sensor_n; ph_sensor_cnt++) {
    ph_sensor_sumX = ph_sensor_sumX + ph_sensor_x[ph_sensor_cnt];
    ph_sensor_sumX2 = ph_sensor_sumX2 +
                      (ph_sensor_x[ph_sensor_cnt] * ph_sensor_x[ph_sensor_cnt]);
    ph_sensor_sumY = ph_sensor_sumY + ph_sensor_y[ph_sensor_cnt];
    ph_sensor_sumXY = ph_sensor_sumXY +
                      (ph_sensor_x[ph_sensor_cnt] * ph_sensor_y[ph_sensor_cnt]);
  }

  float denom = ((float)ph_sensor_n * ph_sensor_sumX2) -
                (ph_sensor_sumX * ph_sensor_sumX);
  if (fabsf(denom) < 1e-6f) {
    /* Degenerate/collinear calibration voltages -> fall back to nominal slope
     */
    ph_sensor_x[1] = 2.58f;
    ph_sensor_x[2] = 3.01f;
    ph_sensor_x[3] = 3.16f;
    ph_sensor_sumX = 0.0f;
    ph_sensor_sumX2 = 0.0f;
    ph_sensor_sumXY = 0.0f;
    for (ph_sensor_cnt = 1; ph_sensor_cnt <= ph_sensor_n; ph_sensor_cnt++) {
      ph_sensor_sumX += ph_sensor_x[ph_sensor_cnt];
      ph_sensor_sumX2 +=
          ph_sensor_x[ph_sensor_cnt] * ph_sensor_x[ph_sensor_cnt];
      ph_sensor_sumXY +=
          ph_sensor_x[ph_sensor_cnt] * ph_sensor_y[ph_sensor_cnt];
    }
    denom = ((float)ph_sensor_n * ph_sensor_sumX2) -
            (ph_sensor_sumX * ph_sensor_sumX);
  }

  ph_sensor_b = (((float)ph_sensor_n * ph_sensor_sumXY) -
                 (ph_sensor_sumX * ph_sensor_sumY)) /
                denom;
  ph_sensor_a =
      (ph_sensor_sumY - (ph_sensor_b * ph_sensor_sumX)) / (float)ph_sensor_n;

#ifdef PH_SENSOR_LOGS
  ESP_LOGI(PH_SENSOR_TAG, "Values are: a=%0.2f and b = %0.2f", ph_sensor_a,
           ph_sensor_b);
#endif
}
