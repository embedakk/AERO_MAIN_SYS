#ifndef PIPE_WATER_CTRL_APPLICATION_H
#define PIPE_WATER_CTRL_APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "../../NOM_STACK/Sensors/RealTimeClock/inc/real_time_clock.h"


#define         PIPE_WATER_CTRL_APPLICATION_TAG         "PIPE_WATER_CTRL_APPLICATION"

#define         PIPE_WATER_CTRL_APPLICATION_LOGS
#undef          PIPE_WATER_CTRL_APPLICATION_LOGS

#define         PIPE_WATER_PUMP_ON_TIME                 3   
#define         PIPE_WATER_PUMP_OFF_TIME                7   
#define         PIPE_WATER_RETURN_DELAY_TIME            2   // Minutes to wait for water to drain back

void PIPE_WATER_CTRL_EXE_APPLICATION(void);

#endif // PIPE_WATER_CTRL_APPLICATION_H