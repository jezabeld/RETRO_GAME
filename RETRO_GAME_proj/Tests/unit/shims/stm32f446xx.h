#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __NOP()  void nop(void){}
#define HAL_MAX_DELAY 0xFFFFFFFFU

/* Estructura mínima/opiaca: suficiente para punteros */
typedef struct {
    int _unused;
} GPIO_TypeDef;
/* Tipos mínimos para GPIO */
typedef enum
{
  GPIO_PIN_RESET = 0,
  GPIO_PIN_SET
}GPIO_PinState;

/* Prototipos que el driver usa */
void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif
