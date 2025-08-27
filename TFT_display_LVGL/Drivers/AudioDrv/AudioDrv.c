/*
 * AudioDrv.c
 *
 *  Created on: Aug 25, 2025
 *      Author: jez
 */

#include "AudioDrv.h"
#include "main.h"

extern TIM_HandleTypeDef htim3;        // TIM3_CH3 PB0 PWM
extern TIM_HandleTypeDef htim6;        // sample rate
extern DMA_HandleTypeDef hdma_tim6_up; // DMA request TIM6_UP

static volatile uint16_t s_pwm[2 * AUDIO_DMA_BLOCK];
static AudioPwmFill12Fn  s_fill12 = 0;
static uint32_t          s_fs = 10000;
static uint32_t          s_arr = 839;   // ≈100 kHz (PSC=0, clk=84MHz)

static void audio_dma_half_callback(DMA_HandleTypeDef *hdma);
static void audio_dma_full_callback(DMA_HandleTypeDef *hdma);

static inline uint16_t map12_to_duty(uint16_t s12, uint32_t arr) {
    return (uint16_t)(( (uint32_t)s12 * arr ) / 4095u);
}

void AudioPwmDrv_Init(uint32_t fs_hz)
{
    s_fs = fs_hz;
    s_arr = __HAL_TIM_GET_AUTORELOAD(&htim3);
    uint16_t mid = map12_to_duty(2048, s_arr);
    for (uint32_t i=0;i<2*AUDIO_DMA_BLOCK;i++) s_pwm[i] = mid;
}

void AudioPwmDrv_SetPwmARR(uint32_t arr) { s_arr = arr; }
uint32_t AudioPwmDrv_GetFs(void) { return s_fs; }
void AudioPwmDrv_SetFill12Fn(AudioPwmFill12Fn fn) { s_fill12 = fn; }

void AudioPwmDrv_Start(void)
{
    // carrier a 50%
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, s_arr/2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

    // Registrar callbacks específicos para este DMA
    hdma_tim6_up.XferHalfCpltCallback = audio_dma_half_callback;
    hdma_tim6_up.XferCpltCallback = audio_dma_full_callback;

    // DMA circular con IT: TIM6_UP → CCR3
    HAL_DMA_Start_IT(&hdma_tim6_up,
                     (uint32_t)s_pwm,
                     (uint32_t)&TIM3->CCR3,
                     2*AUDIO_DMA_BLOCK);

    __HAL_TIM_ENABLE_DMA(&htim6, TIM_DMA_UPDATE);
    HAL_TIM_Base_Start(&htim6);
}

void AudioPwmDrv_Stop(void)
{
    HAL_TIM_Base_Stop(&htim6);
    __HAL_TIM_DISABLE_DMA(&htim6, TIM_DMA_UPDATE);
    HAL_DMA_Abort(&hdma_tim6_up);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, s_arr/2);
    // opcional: HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
}

/* ==== Callbacks DMA específicos para TIM6 ==== */
static void audio_dma_half_callback(DMA_HandleTypeDef *hdma)
{
    if (s_fill12) {
        uint16_t tmp[AUDIO_DMA_BLOCK];
        s_fill12(tmp, AUDIO_DMA_BLOCK);
        for (uint32_t i=0;i<AUDIO_DMA_BLOCK;i++)
            ((uint16_t*)s_pwm)[i] = map12_to_duty(tmp[i], s_arr);
    } else {
        uint16_t mid = map12_to_duty(2048, s_arr);
        for (uint32_t i=0;i<AUDIO_DMA_BLOCK;i++)
            ((uint16_t*)s_pwm)[i] = mid;
    }
}

static void audio_dma_full_callback(DMA_HandleTypeDef *hdma)
{
    if (s_fill12) {
        uint16_t tmp[AUDIO_DMA_BLOCK];
        s_fill12(tmp, AUDIO_DMA_BLOCK);
        for (uint32_t i=0;i<AUDIO_DMA_BLOCK;i++)
            ((uint16_t*)s_pwm)[AUDIO_DMA_BLOCK+i] = map12_to_duty(tmp[i], s_arr);
    } else {
        uint16_t mid = map12_to_duty(2048, s_arr);
        for (uint32_t i=0;i<AUDIO_DMA_BLOCK;i++)
            ((uint16_t*)s_pwm)[AUDIO_DMA_BLOCK+i] = mid;
    }
}
