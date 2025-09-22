#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Estado genérico de HAL (mínimo necesario) */
#ifndef HAL_STATUS_TYPEDEF_DEFINED
#define HAL_STATUS_TYPEDEF_DEFINED
typedef enum { HAL_OK = 0x00U, HAL_ERROR = 0x01U, HAL_BUSY = 0x02U, HAL_TIMEOUT = 0x03U } HAL_StatusTypeDef;
#endif

/* Include basic GPIO types from stm32f446xx.h */
#include "stm32f446xx.h"

/* Include peripheral handle types */
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_tim.h"

/* IRQ definitions */
typedef enum { BTN_A_EXTI_IRQn = 0, BTN_B_EXTI_IRQn = 1, BTN_C_EXTI_IRQn = 2, BTN_D_EXTI_IRQn = 3 } IRQn_Type;

/* Function prototypes used in InputDrv */
void HAL_NVIC_DisableIRQ(IRQn_Type IRQn);
void HAL_NVIC_EnableIRQ(IRQn_Type IRQn);
uint32_t HAL_GetTick(void);

/* Macro definitions */
#define __HAL_GPIO_EXTI_CLEAR_IT(PIN)                                                                                  \
    do {                                                                                                               \
    } while (0)

#ifdef __cplusplus
}
#endif
