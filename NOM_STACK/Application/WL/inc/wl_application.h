#ifndef WL_APPLICATION_H
#define WL_APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define         WL_APPLICATION_TAG         "WL_APPLICATION"

#define         WL_APPLICATION_LOGS
#undef          WL_APPLICATION_LOGS

#define         WL_LOW_THRESHOLD_CM         105 
#define         WL_CRITICAL_THRESHOLD_CM    23 
#define         WL_HIGH_THRESHOLD_CM        33

typedef enum 
{
    WL_IDLE_STATE,
    WL_TANK_IS_FULL,
    WL_TANK_IS_READY_FOR_DOSING,
    WL_FILL_THE_TANK,
    WL_DISCHARGE_THE_TANK,
    WL_CONTROL_DUTY_TO_ANOTHER_UNIT
}wl_state_machine;

extern wl_state_machine wl_sm_current_state;

float calculate_percentage(float length, float width, float height, float filled_height);
float WL_GET_PERCENTAGE(void);
void WL_RESET_STATE(void);

void WL_EXE_APPLICATION(void);

#ifdef __cplusplus
}
#endif

#endif //WL_APPLICATION_H
