#ifndef STM32F4XX_HAL_ADC_H
#define STM32F4XX_HAL_ADC_H

#include <stdint.h>

/* Forward declaration for HAL_StatusTypeDef */
#ifndef HAL_STATUS_TYPEDEF_DEFINED
    #define HAL_STATUS_TYPEDEF_DEFINED
typedef enum { HAL_OK = 0x00U, HAL_ERROR = 0x01U, HAL_BUSY = 0x02U, HAL_TIMEOUT = 0x03U } HAL_StatusTypeDef;
#endif

typedef struct {
    void *Instance;
    void *Init;
    void *NbrOfCurrentConversionRank;
    void *DMA_Handle;
    void *Lock;
    uint32_t State;
    uint32_t ErrorCode;
} ADC_HandleTypeDef;

HAL_StatusTypeDef HAL_ADC_Start_DMA(ADC_HandleTypeDef *hadc, uint32_t *pData, uint32_t Length);

#endif /* STM32F4XX_HAL_ADC_H */