#ifndef CMSIS_OS_H
#define CMSIS_OS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FreeRTOS basic types */
typedef int32_t BaseType_t;
typedef uint32_t TickType_t;

/* FreeRTOS constants */
#define pdFALSE         ((BaseType_t) 0)
#define pdTRUE          ((BaseType_t) 1)
#define pdPASS          (pdTRUE)
#define pdFAIL          (pdFALSE)

/* Timer handle type */
typedef void* TimerHandle_t;

/* Queue handle type */
typedef void* QueueHandle_t;

/* Semaphore handle type */
typedef void* SemaphoreHandle_t;

/* FreeRTOS Timer functions */
BaseType_t xTimerStartFromISR(TimerHandle_t xTimer, BaseType_t *pxHigherPriorityTaskWoken);

/* FreeRTOS Queue functions */
BaseType_t xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait);
BaseType_t xQueueSendFromISR(QueueHandle_t xQueue, const void *pvItemToQueue, BaseType_t *pxHigherPriorityTaskWoken);

/* FreeRTOS macros */
#define portYIELD_FROM_ISR(x) do { (void)(x); } while(0)

#ifdef __cplusplus
}
#endif

#endif /* CMSIS_OS_H */