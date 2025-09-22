#ifndef STM32F4XX_HAL_DMA_H
#define STM32F4XX_HAL_DMA_H

#include <stdint.h>

typedef struct {
    void* Instance;
    void* Init;
    void* Lock;
    uint32_t State;
    void* Parent;
    void* XferCpltCallback;
    void* XferHalfCpltCallback;
    void* XferM1CpltCallback;
    void* XferM1HalfCpltCallback;
    void* XferErrorCallback;
    void* XferAbortCallback;
    uint32_t ErrorCode;
    uint32_t StreamBaseAddress;
    uint32_t StreamIndex;
} DMA_HandleTypeDef;

#endif /* STM32F4XX_HAL_DMA_H */