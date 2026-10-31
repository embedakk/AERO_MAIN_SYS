#ifndef FREERTOS_H
#define FREERTOS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t TickType_t;
typedef int BaseType_t;
typedef unsigned int UBaseType_t;
typedef int portMUX_TYPE;

#define portMUX_INITIALIZER_UNLOCKED 0
#define pdTRUE                       1
#define pdFALSE                      0
#define pdPASS                       1
#define pdFAIL                       0
#define portTICK_PERIOD_MS           1
#define portTICK_RATE_MS             1
#define portMAX_DELAY                ((TickType_t)0xFFFFFFFFUL)
#define configMAX_PRIORITIES         25
#define IRAM_ATTR

#define pdMS_TO_TICKS(ms)            ((TickType_t)(ms))

#define portENTER_CRITICAL(mux)      do { (void)(mux); } while (0)
#define portEXIT_CRITICAL(mux)       do { (void)(mux); } while (0)
#define taskENTER_CRITICAL(mux)      do { (void)(mux); } while (0)
#define taskEXIT_CRITICAL(mux)       do { (void)(mux); } while (0)

void vPortYield(void);

#ifdef __cplusplus
}
#endif

#endif // FREERTOS_H

