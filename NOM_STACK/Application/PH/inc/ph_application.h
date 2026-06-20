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

extern DS1302_DateTime *ph_current_time;
extern int required_time_for_ph_dosing;
extern int ph_calculated_time_difference;

extern bool ph_first_shot_in_out_relays;
extern float ph_adjustment;
extern float ph_volumeToAdd;
extern float ph_first_shot_wl_data;

extern uint16_t ph_check_the_dosing_process_counter;
extern uint8_t ph_place_to_buffer;
extern uint8_t ph_for_loop_counter;
extern float ph_volume_buffer_sum, ph_ph_diff_buffer_sum;
extern float ph_volume_buffer_mean_value, ph_ph_diff_buffer_mean_value;
extern float ph_calculated_volume_buffer[32];
extern float ph_calculated_ph_diff_buffer[32];

int ph_timeToSeconds(DS1302_DateTime t);
DS1302_DateTime ph_secondsToTime(int totalSeconds);
DS1302_DateTime ph_calculateTimeDifference(DS1302_DateTime *start, DS1302_DateTime *end);
int ph_calculate_active_time_of_pump(float volume);

double calculate_adjustment_volume(double tank_volume, double current_pH, double target_pH, double solution_conc);
float calculate_ph_adjustment(float targetPH, float currentPH, float concentrationFactor, float tankVolume);

void CALCULATE_ADDED_PH_VOL_DIFF_ON_TANK(void);
void EXE_PUMPS(void);
void PH_EXE_APPLICATION(void);

#endif //PH_APPLICATION_H