/*
 * AudioDrv.h
 *
 *  Created on: Aug 25, 2025
 *      Author: jez
 */

#ifndef AUDIODRV_AUDIODRV_H_
#define AUDIODRV_AUDIODRV_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifndef AUDIO_DMA_BLOCK
#define AUDIO_DMA_BLOCK  64u
#endif

/* Callback: llená 'dst' con N muestras 12-bit (0..4095, centro=2048) */
typedef void (*AudioPwmFill12Fn)(uint16_t* dst, uint32_t n);

/* Init del driver. fs_hz = TIM6 real (ej: 10000). TIM3 debe estar ya en PWM. */
void AudioPwmDrv_Init(uint32_t fs_hz);

/* Arranca PWM + DMA (circular) usando TIM6_UP -> TIM3->CCR3 */
void AudioPwmDrv_Start(void);

/* Detiene limpio (CCR a 50%) */
void AudioPwmDrv_Stop(void);

/* Registra la función de llenado (se llama en half/full) */
void AudioPwmDrv_SetFill12Fn(AudioPwmFill12Fn fn);

/* Actualiza ARR de TIM3 si lo cambiás en runtime (carrier) */
void AudioPwmDrv_SetPwmARR(uint32_t arr);

/* Lee fs actual (para el player) */
uint32_t AudioPwmDrv_GetFs(void);

#endif /* AUDIODRV_AUDIODRV_H_ */
