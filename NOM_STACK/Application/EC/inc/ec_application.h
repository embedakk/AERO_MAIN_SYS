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


DS1302_DateTime *current_time;
int required_time_for_ec_dosing;
int calculated_time_difference;

bool ec_first_shot_in_out_relays;
float adjustment;
float volumeToAdd;
float first_shot_wl_data;

uint16_t check_the_dosing_process_counter;
uint8_t place_to_buffer;
uint8_t for_loop_counter;
float volume_buffer_sum, ec_diff_buffer_sum;
float volume_buffer_mean_value, ec_diff_buffer_mean_value;
float calculated_volume_buffer[32];
float calculated_ec_diff_buffer[32];

int timeToSeconds(DS1302_DateTime t);
DS1302_DateTime secondsToTime(int totalSeconds);
DS1302_DateTime calculateTimeDifference(DS1302_DateTime *start, DS1302_DateTime *end);
int calculate_active_time_of_pump(float volume);

float calculate_solution_volume(float targetEC, float currentEC, float solutionEC, float tankVolume);
float calculate_ec_adjustment(float targetEC, float currentEC, float concentrationFactor, float tankVolume);

void EC_EXE_APPLICATION(void);

#endif //EC_APPLICATION_H