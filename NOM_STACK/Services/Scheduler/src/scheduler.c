#include "../inc/scheduler.h"
#include "../../../Manager/INIT/inc/initialization.h"

#undef SCHEDULER_LOGS

int scheduler_mechanism_counter = 0;
TickType_t scheduler_xLastWakeTime = 0;
static bool s_scheduler_tasks_started = false;

void os_signal_task_1ms(void) {
  xTaskCreatePinnedToCore(task_1ms, "task_1ms", 4096, NULL, 9, NULL, 0);
#ifdef SCHEDULER_LOGS
  ESP_LOGI(SCHEDULER_TAG, "os_signal_task_1ms...");
#endif
}

void os_signal_task_3ms(void) {
  xTaskCreatePinnedToCore(task_3ms, "task_3ms", 4096, NULL, 9, NULL, 1);
#ifdef SCHEDULER_LOGS
  ESP_LOGI(SCHEDULER_TAG, "os_signal_task_3ms...");
#endif
}

void os_signal_task_5ms(void) {
  xTaskCreatePinnedToCore(task_5ms, "task_5ms", 4096, NULL, 8, NULL, 0);
#ifdef SCHEDULER_LOGS
  ESP_LOGI(SCHEDULER_TAG, "os_signal_task_5ms...");
#endif
}

void os_signal_task_10ms(void) {
  xTaskCreatePinnedToCore(task_10ms, "task_10ms", 4096, NULL, 8, NULL, 1);
#ifdef SCHEDULER_LOGS
  ESP_LOGI(SCHEDULER_TAG, "os_signal_task_10ms...");
#endif
}

void os_signal_task_15ms(void) {
  xTaskCreatePinnedToCore(task_15ms, "task_15ms", 4096, NULL, 7, NULL, 0);
#ifdef SCHEDULER_LOGS
  ESP_LOGI(SCHEDULER_TAG, "os_signal_task_15ms...");
#endif
}

void os_signal_task_20ms(void) {
  xTaskCreatePinnedToCore(task_20ms, "task_20ms", 4096, NULL, 7, NULL, 1);
#ifdef SCHEDULER_LOGS
  ESP_LOGI(SCHEDULER_TAG, "os_signal_task_20ms...");
#endif
}

void os_signal_task_25ms(void) {
  xTaskCreatePinnedToCore(task_25ms, "task_25ms", 4096, NULL, 6, NULL, 0);
#ifdef SCHEDULER_LOGS
  ESP_LOGI(SCHEDULER_TAG, "os_signal_task_25ms...");
#endif
}

void SCHEDULER_INIT(void) {
  s_scheduler_tasks_started = false;
  scheduler_mechanism_counter = 0;
  INIT_PERIPHERALS();
}

void SCHEDULER_MECHANISM(void *pvParameters) {
  (void)pvParameters;
  scheduler_xLastWakeTime = xTaskGetTickCount();

  while (1) {
    scheduler_mechanism_counter++;
    vTaskDelayUntil(&scheduler_xLastWakeTime, pdMS_TO_TICKS(100));
  }
}

void SCHEDULER_MAIN(void) {
  if (s_scheduler_tasks_started) {
    return;
  }
  s_scheduler_tasks_started = true;

  // Create each periodic task ONCE; each task loops internally with
  // vTaskDelayUntil
  os_signal_task_1ms();  // 10 ms period
  os_signal_task_3ms();  // 25 ms period
  os_signal_task_5ms();  // 45 ms period
  os_signal_task_10ms(); // 65 ms period
  os_signal_task_15ms(); // 85 ms period
  os_signal_task_20ms(); // 110 ms period
  os_signal_task_25ms(); // 135 ms period
}
