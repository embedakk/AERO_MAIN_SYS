#ifndef SCHEDULER_H
#define SCHEDULER_H

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


#define         SCHEDULER_TAG         "SCHEDULER"

#define         SCHEDULER_LOGS
#undef          SCHEDULER_LOGS

int scheduler_mechanism_counter;
TickType_t scheduler_xLastWakeTime;

void task_1ms(void *arg);
void task_3ms(void *arg); 
void task_5ms(void *arg); 
void task_10ms(void *arg); 
void task_15ms(void *arg);
void task_20ms(void *arg);
void task_25ms(void *arg); 

//signals
void os_signal_task_1ms(void);
void os_signal_task_3ms(void);
void os_signal_task_5ms(void);
void os_signal_task_10ms(void);
void os_signal_task_15ms(void);
void os_signal_task_20ms(void);
void os_signal_task_25ms(void);

void SCHEDULER_INIT(void); //ALL INITIALIZTION WILL BE DONE HERE
void SCHEDULER_MECHANISM(void *pvParameters); // TASK AND SIGNAL TIMING STUFFS
void SCHEDULER_MAIN(void); //

#endif // SCHEDULER_H