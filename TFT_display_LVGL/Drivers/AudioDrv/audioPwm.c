/*
 * audioPwm.c
 *
 *  Created on: Aug 27, 2025
 *      Author: jez
 */
// ========= audio_pwm.c =========
#include "audioPwm.h"
#include "main.h"

extern TIM_HandleTypeDef htim3;  // PWM carrier (CH3 en PB0)
extern TIM_HandleTypeDef htim6;  // sample rate timer (fs)
extern DMA_HandleTypeDef hdma_tim6_up; // generado por CubeMX al añadir TIM6_UP DMA

// Configura acá tu portadora y fs en CubeMX:
// TIM3: PSC=0, ARR=839 -> 100 kHz  
// TIM6: PSC=84-1, ARR=100-1 -> 10 kHz

// Tamaño de buffer (ping-pong si querés rellenar por mitades)
#define N_SAMPLES   256
static uint16_t pwm_duty[N_SAMPLES];

// ARR de TIM3 (lo leemos en runtime)
static inline uint32_t PWM_ARR(void) { return __HAL_TIM_GET_AUTORELOAD(&htim3); }

// Convierte muestra 12-bit (0..4095) a duty 0..ARR
static inline uint16_t dac12_to_pwm(uint16_t s12, uint32_t arr)
{
    // duty = round( s12 * (ARR) / 4095 )
    return (uint16_t)(( (uint32_t)s12 * arr ) / 4095u);
}

// Rellena el buffer con una senoide centrada (ejemplo)
static void build_sine_block(uint16_t *dst, uint32_t n, float freq_hz, uint32_t fs_hz, uint32_t arr)
{
    static float phase = 0.0f;
    float step = 2.0f * 3.14159265f * (freq_hz / (float)fs_hz);
    for (uint32_t i=0; i<n; i++) {
        float s = sinf(phase);             // -1..+1
        int32_t dac = 2048 + (int32_t)(1500.0f * s); // 12-bit nominal
        if (dac < 0) dac = 0; else if (dac > 4095) dac = 4095;
        dst[i] = dac12_to_pwm((uint16_t)dac, arr);
        phase += step; if (phase >= 2.0f*3.14159265f) phase -= 2.0f*3.14159265f;
    }
}

// Inicia PWM + DMA: TIM3 PWM ON, DMA (trigger TIM6_UP) -> TIM3->CCR3, TIM6 fs ON
void AudioPWM_Start(float tone_hz, uint32_t fs_hz)
{
    uint32_t arr = PWM_ARR();

    // Preload CCR3 con duty medio
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, arr/2);

    // Arranca PWM portadora
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

    // Prepara el buffer de duties a la fs deseada
    build_sine_block(pwm_duty, N_SAMPLES, tone_hz, fs_hz, arr);

    // Arranca DMA: origen = pwm_duty, destino = TIM3->CCR3
    // OJO: el request del DMA viene de TIM6_UP (no de TIM3). CubeMX crea hdma_tim6_up.
    HAL_DMA_Start(&hdma_tim6_up,
                  (uint32_t)pwm_duty,
                  (uint32_t)&TIM3->CCR3,   // destino: duty
                  N_SAMPLES);

    // Habilitá la petición DMA por Update de TIM6
    __HAL_TIM_ENABLE_DMA(&htim6, TIM_DMA_UPDATE);

    // Arrancá TIM6 (marca la fs → una transferencia por update)
    HAL_TIM_Base_Start(&htim6);
}

// Para limpio: frenar fs → deshabilitar DMA → duty medio → apagar PWM si querés
void AudioPWM_Stop(void)
{
    HAL_TIM_Base_Stop(&htim6);
    __HAL_TIM_DISABLE_DMA(&htim6, TIM_DMA_UPDATE);
    HAL_DMA_Abort(&hdma_tim6_up);

    uint32_t arr = PWM_ARR();
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, arr/2);
    // opcional: HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
}


