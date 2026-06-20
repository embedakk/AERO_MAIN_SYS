#ifndef INITIALIZATION_H
#define INITIALIZATION_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"


#define         INIT_TAG         "INIT"

#define         INIT_LOGS
#undef          INIT_LOGS

/*
    Here put the pin numbers
*/


void INIT_PH_EC_SENSORS(void);//put I2C COM init
void INIT_RTC(void);
void INIT_WATER_TEMPERATURE(void);
void INIT_WATER_LEVEL(void);
void INIT_PH_EC_PUMPS(void);
void INIT_MAIN_PUMP(void);
void INIT_CIRCULATION_PUMPS(void);
void INIT_TANK_WATER_INPUT(void);
void INIT_TANK_WATER_OUTPUT(void);
void INIT_ROOM_TEMPERATURE(void);
void INIT_ROOM_HUMIDITY(void);
void INIT_ROOM_AIR_CONDITIONER(void);
void INIT_ROOM_LEDS(void);
void INIT_PIPE_VENTILATION(void);
void INIT_MIXER_MOTORS(void);
void INIT_WIFI(void);
void INIT_BLE(void);
void INIT_UI(void);

void INIT_PERIPHERALS(void);



#endif // INITIALIZATION_H