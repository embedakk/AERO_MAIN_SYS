#ifndef FREERTOS_QUEUE_H
#define FREERTOS_QUEUE_H

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *QueueHandle_t;

QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize);
BaseType_t xQueueReceive(QueueHandle_t xQueue, void * const pvBuffer, TickType_t xTicksToWait);
BaseType_t xQueueSend(QueueHandle_t xQueue, const void * const pvItemToQueue, TickType_t xTicksToWait);
BaseType_t xQueueReset(QueueHandle_t xQueue);

#ifdef __cplusplus
}
#endif

#endif // FREERTOS_QUEUE_H

