#ifndef PH_APPLICATION_H
#define PH_APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "../../NOM_STACK/Sensors/RealTimeClock/inc/real_time_clock.h"


#define         PH_APPLICATION_TAG         "PH_APPLICATION"

#define         PH_APPLICATION_LOGS
#undef          PH_APPLICATION_LOGS

#define         PH_DIFF_TOLERANCE_POS       0.25f //pH
#define         PH_DIFF_TOLERANCE_NEG       -0.25f //pH

typedef enum 
{
    PH_CALCULATE_PH_ADJ_VOLUME,
    PH_EXE_PUMPS,
    PH_CALCULATE_ADDED_PH_VOL_DIFF_ON_TANK
}ph_state_machine;

typedef enum 
{
    PH_POS_IDLE,
    PH_POS_NEGATIVE,
    PH_POS_POSITIVE
}ph_diff_pos;

DS1302_DateTime *current_time;
int required_time_for_ph_dosing;
int calculated_time_difference;

bool ph_first_shot_in_out_relays;
float adjustment;
float volumeToAdd;
float first_shot_wl_data;

uint16_t check_the_dosing_process_counter;
uint8_t place_to_buffer;
uint8_t for_loop_counter;
float volume_buffer_sum, ph_diff_buffer_sum;
float volume_buffer_mean_value, ph_diff_buffer_mean_value;
float calculated_volume_buffer[32];
float calculated_ph_diff_buffer[32];

int ph_timeToSeconds(DS1302_DateTime t);
DS1302_DateTime ph_secondsToTime(int totalSeconds);
DS1302_DateTime ph_calculateTimeDifference(DS1302_DateTime *start, DS1302_DateTime *end);
int ph_calculate_active_time_of_pump(float volume);

double calculate_adjustment_volume(double tank_volume, double current_pH, double target_pH, double solution_conc);
float calculate_ph_adjustment(float targetPH, float currentPH, float concentrationFactor, float tankVolume);

void PH_EXE_APPLICATION(void);

#endif //PH_APPLICATION_H