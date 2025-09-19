#pragma once
#include <stdint.h>
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Handle mínimo/opiaco para SPI */
typedef struct {
    void *Instance;
} SPI_HandleTypeDef;

/* Prototipos que el driver usa */
HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *hspi, const uint8_t *pData, uint16_t Size, uint32_t Timeout);

HAL_StatusTypeDef HAL_SPI_Receive(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);

HAL_StatusTypeDef HAL_SPI_Transmit_DMA(SPI_HandleTypeDef *hspi, const uint8_t *pData, uint16_t Size);

#ifdef __cplusplus
}
#endif
