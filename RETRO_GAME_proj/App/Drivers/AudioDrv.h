/*
 * AudioDrv.h
 *
 *  Created on: Aug 25, 2025
 *      Author: jez
 */


#ifndef DRIVERS_AUDIODRV_H_
#define DRIVERS_AUDIODRV_H_

// #include <stdint.h>
// #include <stdbool.h>

// typedef struct {
//     const int16_t *pcm;
//     uint32_t durationMs;
// } audioPcmCue_t;

// int audioInit(void);
// int audioPlayFx(const audioPcmCue_t *cue);


#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifndef AUDIO_DMA_BLOCK
#define AUDIO_DMA_BLOCK  64u
#endif

/* Callback: llená 'dst' con N muestras 12-bit (0..4095, centro=2048) */
typedef void (*audioPwmFill12Fn)(uint16_t* dst, uint32_t n);

/* Init del driver. fs_hz = TIM6 real (ej: 10000). TIM3 debe estar ya en PWM. */
void audioInit(uint32_t fs_hz);

/* Arranca PWM + DMA (circular) usando TIM6_UP -> TIM3->CCR3 */
void audioStart(void);

/* Detiene limpio (CCR a 50%) */
void audioStop(void);

/* Registra la función de llenado (se llama en half/full) */
void audioSetFill12Fn(audioPwmFill12Fn fn);

/* Actualiza ARR de TIM3 si lo cambiás en runtime (carrier) */
void audioSetPwmARR(uint32_t arr);

/* Lee fs actual (para el player) */
uint32_t audioGetFs(void);

#endif /* DRIVERS_AUDIODRV_H_ */