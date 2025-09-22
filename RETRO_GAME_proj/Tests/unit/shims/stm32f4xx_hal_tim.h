#ifndef STM32F4XX_HAL_TIM_H
#define STM32F4XX_HAL_TIM_H

#include <stdint.h>

/* Forward declaration for HAL_StatusTypeDef */
#ifndef HAL_STATUS_TYPEDEF_DEFINED
#define HAL_STATUS_TYPEDEF_DEFINED
typedef enum {
    HAL_OK      = 0x00U,
    HAL_ERROR   = 0x01U,
    HAL_BUSY    = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;
#endif

typedef struct {
    void* Instance;
    void* Init;
    uint32_t ActiveChannel;
    void* DMA_Handle;
    void* Lock;
    uint32_t State;
    uint32_t ChannelState[4];
    uint32_t ChannelNState[4];
    void* DMABurstState;
} TIM_HandleTypeDef;

HAL_StatusTypeDef HAL_TIM_Base_Start(TIM_HandleTypeDef *htim);

#endif /* STM32F4XX_HAL_TIM_H */