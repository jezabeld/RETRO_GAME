#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Estado genérico de HAL (mínimo necesario) */
typedef enum {
    HAL_OK      = 0x00U,
    HAL_ERROR   = 0x01U,
    HAL_BUSY    = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;

#ifdef __cplusplus
}
#endif
