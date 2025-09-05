/*
 * AudioPlayerTask.c
 *
 * Tarea FreeRTOS para manejar eventos de audio del sistema
 *
 *  Created on: Sep 3, 2025
 *      Author: jez
 */

#include "AudioPlayer.h"
#include "synchronization.h"

extern audioDrv_t audioDriver;

static const note_t beep_table[] = {
    {2048, 200, WF_SINE, 1500},     // 0: buen sonido para solo un beep de boton (se cumplio tiempo, aviso de la nave)
    {1000,60,  WF_SQUARE, 1000},    // 1: muy corto, “blip” (un buen pip agudo, radar)
    {523, 120, WF_SINE,   1400},    // 2: DO5 (aggarrar algo, item, seleccion UI, suave)
    {440, 300, WF_NOISE,  900},     // 3: burst de ruido (explosión pequeña)
    {660, 100, WF_SQUARE, 1200},    // 4: warning
    {300, 200, WF_SQUARE, 1500},    // 5: NOP
};

/* ===== TAREA FREERTOS ===== */
void AudioPlayerTask(void *pvParameters)
{
    
    /* Inicializar AudioPlayer */
    playerInit(&audioDriver);
    
    event_id_t receivedEvent;
    
    /* Loop principal de la tarea */
    for (;;) {
        /* Esperar eventos de audio */
        if (xQueueReceive(qAudio, &receivedEvent, portMAX_DELAY) == pdTRUE) {
            
            /* Procesar eventos de audio */
            switch (receivedEvent) {
                case AUP_BEEP_1:
                    /* Usar primer sonido de la tabla */
                    playerPlaySfx(beep_table[0]);
                    break;
                    
                case AUP_BEEP_2:
                    /* Usar segundo sonido de la tabla */
                    playerPlaySfx(beep_table[1]);
                    break;
                    
                case AUP_BEEP_3:
                    /* Usar tercer sonido de la tabla */
                    playerPlaySfx(beep_table[2]);
                    break;
                    
                case AUP_BEEP_4:
                    /* Usar cuarto sonido de la tabla */
                    playerPlaySfx(beep_table[3]);
                    break;
                
                default:
                    /* Evento no manejado */
                    break;
            }
        }
    }
}