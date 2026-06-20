#ifndef LED_CTRL_APPLICATION_H
#define LED_CTRL_APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "../../NOM_STACK/Sensors/RealTimeClock/inc/real_time_clock.h"


#define         LED_CTRL_APPLICATION_TAG         "LED_CTRL_APPLICATION"

#define         LED_CTRL_APPLICATION_LOGS
#undef          LED_CTRL_APPLICATION_LOGS

DS1302_DateTime end;

DS1302_DateTime calculateLEDTurnOffTime(DS1302_DateTime start, int durationHours);

void LED_CTRL_EXE_APPLICATION(void);

#endif //LED_CTRL_APPLICATION_H