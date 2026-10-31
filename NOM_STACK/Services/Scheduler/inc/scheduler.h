#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCHEDULER_TAG "SCHEDULER"

#define SCHEDULER_LOGS
#undef SCHEDULER_LOGS

extern int scheduler_mechanism_counter;
extern TickType_t scheduler_xLastWakeTime;

// Deterministic single-step handlers (used by persistent FreeRTOS tasks and
// GoogleTest suites)
void scheduler_step_1ms(void);
void scheduler_step_3ms(void);
void scheduler_step_5ms(void);
void scheduler_step_10ms(void);
void scheduler_step_15ms(void);
void scheduler_step_20ms(void);
void scheduler_step_25ms(void);

// Persistent FreeRTOS task entry points
void task_1ms(void *arg);
void task_3ms(void *arg);
void task_5ms(void *arg);
void task_10ms(void *arg);
void task_15ms(void *arg);
void task_20ms(void *arg);
void task_25ms(void *arg);

// Task creation helpers (spawned once in SCHEDULER_MAIN)
void os_signal_task_1ms(void);
void os_signal_task_3ms(void);
void os_signal_task_5ms(void);
void os_signal_task_10ms(void);
void os_signal_task_15ms(void);
void os_signal_task_20ms(void);
void os_signal_task_25ms(void);

void SCHEDULER_INIT(void); // ALL INITIALIZATION WILL BE DONE HERE
void SCHEDULER_MECHANISM(void *pvParameters); // LEGACY / SUPERVISOR HOOK
void SCHEDULER_MAIN(void); // SPAWNS PERSISTENT PERIODIC TASKS ONCE

#ifdef __cplusplus
}
#endif

#endif // SCHEDULER_H
