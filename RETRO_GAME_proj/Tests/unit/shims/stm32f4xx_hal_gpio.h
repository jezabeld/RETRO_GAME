#ifndef STM32F4XX_HAL_GPIO_H
#define STM32F4XX_HAL_GPIO_H

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

/* GPIO types are defined in stm32f446xx.h */
#include "stm32f446xx.h"

/* GPIO function prototypes */
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);

#endif /* STM32F4XX_HAL_GPIO_H */