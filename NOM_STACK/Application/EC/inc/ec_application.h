#ifndef EC_APPLICATION_H
#define EC_APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "../../NOM_STACK/Sensors/RealTimeClock/inc/real_time_clock.h"

#define         EC_APPLICATION_TAG         "EC_APPLICATION"

#define         EC_APPLICATION_LOGS
#undef          EC_APPLICATION_LOGS

#define         EC_DIFF_TOLERANCE_POS       100.0f //uS/cm
#define         EC_DIFF_TOLERANCE_NEG       -100.0f //uS/cm

typedef enum 
{
    EC_CALCULATE_EC_ADJ_VOLUME,
    EC_EXE_PUMPS,
    EC_CALCULATE_ADDED_EC_VOL_DIFF_ON_TANK
}ec_state_machine;

typedef enum 
{
    EC_POS_IDLE,
    EC_POS_NEGATIVE,
    EC_POS_POSITIVE
}ec_diff_pos;


extern DS1302_DateTime *ec_current_time;
extern int required_time_for_ec_dosing;
extern int ec_calculated_time_difference;

extern bool ec_first_shot_in_out_relays;
extern float ec_adjustment;
extern float ec_volumeToAdd;
extern float ec_first_shot_wl_data;

extern uint16_t ec_check_the_dosing_process_counter;
extern uint8_t ec_place_to_buffer;
extern uint8_t ec_for_loop_counter;
extern float ec_volume_buffer_sum, ec_diff_buffer_sum;
extern float ec_volume_buffer_mean_value, ec_diff_buffer_mean_value;
extern float ec_calculated_volume_buffer[32];
extern float ec_calculated_ec_diff_buffer[32];

int ec_timeToSeconds(DS1302_DateTime t);
DS1302_DateTime ec_secondsToTime(int totalSeconds);
DS1302_DateTime ec_calculateTimeDifference(DS1302_DateTime *start, DS1302_DateTime *end);
int ec_calculate_active_time_of_pump(float volume);

float calculate_solution_volume(float targetEC, float currentEC, float solutionEC, float tankVolume);
float calculate_ec_adjustment(float targetEC, float currentEC, float concentrationFactor, float tankVolume);

void calculate_added_vol_diff_ec(void);
void exe_ec_pumps(void);

void EC_EXE_APPLICATION(void);

#endif //EC_APPLICATION_H