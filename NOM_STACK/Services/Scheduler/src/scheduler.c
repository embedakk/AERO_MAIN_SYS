#include "../../../Manager/INIT/inc/initialization.h"
#include "../inc/scheduler.h"

#undef          SCHEDULER_LOGS

void os_signal_task_1ms() 
{ 
    xTaskCreatePinnedToCore(task_1ms, "task_1ms", 4096, NULL, 9, NULL, 0);
#ifdef SCHEDULER_LOGS
    ESP_LOGI(SCHEDULER_TAG, "os_signal_task_1ms...");
#endif
    return;
}

void os_signal_task_3ms() 
{ 
    xTaskCreatePinnedToCore(task_3ms, "task_3ms", 4096, NULL, 9, NULL, 1);
#ifdef SCHEDULER_LOGS
    ESP_LOGI(SCHEDULER_TAG, "os_signal_task_3ms...");
#endif
    return;
}

void os_signal_task_5ms() 
{ 
    xTaskCreatePinnedToCore(task_5ms, "task_5ms", 4096, NULL, 8, NULL, 0);

#ifdef SCHEDULER_LOGS
    ESP_LOGI(SCHEDULER_TAG, "os_signal_task_5ms...");
#endif
    return;
}

void os_signal_task_10ms() 
{
    xTaskCreatePinnedToCore(task_10ms, "task_10ms", 4096, NULL, 8, NULL, 1);
#ifdef SCHEDULER_LOGS
    ESP_LOGI(SCHEDULER_TAG, "os_signal_task_10ms...");
#endif
    return;
}

void os_signal_task_15ms() 
{ 
    xTaskCreatePinnedToCore(task_15ms, "task_15ms", 4096, NULL, 7, NULL, 0);
#ifdef SCHEDULER_LOGS
ESP_LOGI(SCHEDULER_TAG, "os_signal_task_15ms...");
#endif
    return;
}

void os_signal_task_20ms()
{ 
    xTaskCreatePinnedToCore(task_20ms, "task_20ms", 4096, NULL, 7, NULL, 1);
#ifdef SCHEDULER_LOGS
    ESP_LOGI(SCHEDULER_TAG, "os_signal_task_20ms...");
#endif
    return;
}

void os_signal_task_25ms() 
{ 
    xTaskCreatePinnedToCore(task_25ms, "task_25ms", 4096, NULL, 6, NULL, 0);
#ifdef SCHEDULER_LOGS
    ESP_LOGI(SCHEDULER_TAG, "os_signal_task_25ms...");
#endif
    return;
}

void SCHEDULER_INIT(void) //ALL INITIALIZTION WILL BE DONE HERE
{
    // vTaskDelayUntil(&scheduler_xLastWakeTime, pdMS_TO_TICKS(100));
    INIT_PERIPHERALS();
    // printf("INIT PERIPHERALS DONE...\r\n");
    return;
}

// Scheduler MECHANISM
void SCHEDULER_MECHANISM(void *pvParameters) // TASK AND SIGNAL TIMING STUFFS 
{
    scheduler_xLastWakeTime = xTaskGetTickCount();

    while (1) {
        if (scheduler_mechanism_counter % 2 == 0)  os_signal_task_1ms();//2
        if (scheduler_mechanism_counter % 5 == 0)  os_signal_task_3ms();//5
        if (scheduler_mechanism_counter % 9 == 0)  os_signal_task_5ms();//9
        if (scheduler_mechanism_counter % 13 == 0) os_signal_task_10ms();//13
        if (scheduler_mechanism_counter % 17 == 0) os_signal_task_15ms();//17
        if (scheduler_mechanism_counter % 22 == 0) os_signal_task_20ms();//22
        if (scheduler_mechanism_counter % 27 == 0) os_signal_task_25ms();//26

        scheduler_mechanism_counter++;  // Increment scheduler counter

        vTaskDelayUntil(&scheduler_xLastWakeTime, pdMS_TO_TICKS(5));
    }
}

void SCHEDULER_MAIN(void) //
{
    xTaskCreate(SCHEDULER_MECHANISM, "SCHEDULER_MECHANISM", 4096, NULL, 10, NULL);
    return;
}