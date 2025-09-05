/*
 * AudioDrv.c
 *
 *  Created on: Aug 25, 2025
 *      Author: jez
 */

#include "AudioDrv.h"

/* Instancia estática para callbacks DMA (driver de una sola instancia) */
static audioDrv_t *g_audio_instance = 0;

/* Callbacks DMA privados */
static void audioDmaHalfCallback(DMA_HandleTypeDef *hdma);
static void audioDmaFullCallback(DMA_HandleTypeDef *hdma);

/* Mapea sample de 12-bit (0..4095) a duty cycle PWM */
static inline uint16_t audioMap12ToDuty(uint16_t s12, uint32_t pwm_arr) {
    return (uint16_t)(( (uint32_t)s12 * pwm_arr ) / AUDIO_SAMPLE_MAX);
}

uint8_t audioInit(audioDrv_t *audio, TIM_HandleTypeDef *pwm_tim, uint32_t pwm_ch, 
                  TIM_HandleTypeDef *fs_tim, DMA_HandleTypeDef *dma, 
                  uint32_t fs_hz)
{
    if (!audio || !pwm_tim || !fs_tim || !dma) return 1;
    
    /* Configurar el objeto */
    audio->pwm_timer = pwm_tim;
    audio->pwm_channel = pwm_ch;
    audio->pwm_arr = __HAL_TIM_GET_AUTORELOAD(pwm_tim);
    audio->fs_timer = fs_tim;
    audio->dma_handle = dma;
    audio->fs_hz = fs_hz;
    audio->fill_callback = 0;
    
    /* Inicializar buffer con silencio (50% duty) */
    uint16_t mid = audioMap12ToDuty(AUDIO_SAMPLE_CENTER, audio->pwm_arr);
    for (uint32_t i = 0; i < 2 * AUDIO_DMA_BLOCK; i++) {
        audio->pwm_buffer[i] = mid;
    }
    
    /* Registrar instancia para callbacks DMA */
    g_audio_instance = audio;
    
    return 0;
}

uint32_t audioGetFs(audioDrv_t *audio) { 
    return audio ? audio->fs_hz : 0; 
}

void audioSetFillFn(audioDrv_t *audio, audioPwmFill12Fn fn) { 
    if (audio) audio->fill_callback = fn; 
}

void audioStart(audioDrv_t *audio)
{
    if (!audio) return;
    
    // Configurar PWM al 50% duty inicialmente
    __HAL_TIM_SET_COMPARE(audio->pwm_timer, audio->pwm_channel, audio->pwm_arr/2);
    HAL_TIM_PWM_Start(audio->pwm_timer, audio->pwm_channel);

    // Registrar callbacks específicos para este DMA
    audio->dma_handle->XferHalfCpltCallback = audioDmaHalfCallback;
    audio->dma_handle->XferCpltCallback = audioDmaFullCallback;

    // Calcular dirección del registro CCR según el canal
    uint32_t ccr_address;
    switch (audio->pwm_channel) {
        case TIM_CHANNEL_1: ccr_address = (uint32_t)&audio->pwm_timer->Instance->CCR1; break;
        case TIM_CHANNEL_2: ccr_address = (uint32_t)&audio->pwm_timer->Instance->CCR2; break;
        case TIM_CHANNEL_3: ccr_address = (uint32_t)&audio->pwm_timer->Instance->CCR3; break;
        case TIM_CHANNEL_4: ccr_address = (uint32_t)&audio->pwm_timer->Instance->CCR4; break;
        default: return;
    }

    // DMA circular con IT: fs_timer_UP → PWM_CCRx
    HAL_DMA_Start_IT(audio->dma_handle,
                     (uint32_t)audio->pwm_buffer,
                     ccr_address,
                     2*AUDIO_DMA_BLOCK);

    __HAL_TIM_ENABLE_DMA(audio->fs_timer, TIM_DMA_UPDATE);
    HAL_TIM_Base_Start(audio->fs_timer);
}

void audioStop(audioDrv_t *audio)
{
    if (!audio) return;
    
    HAL_TIM_Base_Stop(audio->fs_timer);
    __HAL_TIM_DISABLE_DMA(audio->fs_timer, TIM_DMA_UPDATE);
    HAL_DMA_Abort(audio->dma_handle);
    __HAL_TIM_SET_COMPARE(audio->pwm_timer, audio->pwm_channel, audio->pwm_arr/2);
    // opcional: HAL_TIM_PWM_Stop(audio->pwm_timer, audio->pwm_channel);
}

/* ==== Callbacks DMA específicos ==== */
static void audioDmaHalfCallback(DMA_HandleTypeDef *hdma)
{
    if (!g_audio_instance) return;
    
    if (g_audio_instance->fill_callback) {
        uint16_t tmp[AUDIO_DMA_BLOCK];
        g_audio_instance->fill_callback(tmp, AUDIO_DMA_BLOCK);
        for (uint32_t i = 0; i < AUDIO_DMA_BLOCK; i++) {
            g_audio_instance->pwm_buffer[i] = audioMap12ToDuty(tmp[i], g_audio_instance->pwm_arr);
        }
    } else {
        uint16_t mid = audioMap12ToDuty(AUDIO_SAMPLE_CENTER, g_audio_instance->pwm_arr);
        for (uint32_t i = 0; i < AUDIO_DMA_BLOCK; i++) {
            g_audio_instance->pwm_buffer[i] = mid;
        }
    }
}

static void audioDmaFullCallback(DMA_HandleTypeDef *hdma)
{
    if (!g_audio_instance) return;
    
    if (g_audio_instance->fill_callback) {
        uint16_t tmp[AUDIO_DMA_BLOCK];
        g_audio_instance->fill_callback(tmp, AUDIO_DMA_BLOCK);
        for (uint32_t i = 0; i < AUDIO_DMA_BLOCK; i++) {
            g_audio_instance->pwm_buffer[AUDIO_DMA_BLOCK + i] = audioMap12ToDuty(tmp[i], g_audio_instance->pwm_arr);
        }
    } else {
        uint16_t mid = audioMap12ToDuty(AUDIO_SAMPLE_CENTER, g_audio_instance->pwm_arr);
        for (uint32_t i = 0; i < AUDIO_DMA_BLOCK; i++) {
            g_audio_instance->pwm_buffer[AUDIO_DMA_BLOCK + i] = mid;
        }
    }
}
