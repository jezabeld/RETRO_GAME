/*
 * AudioPlayer.c
 *
 *  Created on: Aug 26, 2025
 *      Author: jez
 */
#include "AudioPlayer.h"
#include <math.h>
#include <stdlib.h>

/**********************
 *      DEFINES
 **********************/
#define AMP_MAX   1800
#define SAMPLE_MID   2048
#define FADE_SMP  64
#define SFX_GAP_MS  100  /* Silencio antes/después de SFX en ms */

/**********************
 *      TYPEDEFS
 **********************/
/* Estado de reproducción de música */
typedef struct {
    const note_t* song;
    uint32_t len;
    uint32_t idx;
    uint8_t loop;
    uint8_t playing;
} musicState_t;

/* Estado de efectos de sonido */
typedef struct {
    note_t sfx;
    uint8_t active;
    uint8_t preGap;   /* Silencio antes del SFX */
    uint8_t postGap;  /* Silencio después del SFX */
} sfxState_t;

/* Estado del generador de ondas */
typedef struct {
    float phase;
    float step;
    wave_t wave;
    uint16_t amp;
    uint32_t samplesLeft;
    uint32_t fadeSamples;
} waveState_t;

/**********************
 *  STATIC VARIABLES
 **********************/
/* Referencia al driver de audio (inicializado externamente) */
static audioDrv_t *s_audioDrv = 0;

/* Estados del reproductor organizados */
static musicState_t s_music = {0};
static sfxState_t s_sfx = {0};
static waveState_t s_wave = {0};
static float s_globalGain = 1.0f;

/* Onda */
static inline float waveSample(float a, wave_t wf) {
    switch (wf) {
    case WF_SINE:   return sinf(a);
    case WF_SQUARE: return (sinf(a) >= 0.0f) ? 1.0f : -1.0f;
    case WF_SAW: { float ph=a*(1.0f/(2.0f*M_PI)); ph-=(int)ph; return 2.0f*ph-1.0f; }
    case WF_TRI: { float ph=a*(1.0f/(2.0f*M_PI)); ph-=(int)ph; return (ph<0.5f)?(4.0f*ph-1.0f):(-4.0f*ph+3.0f); }
    case WF_NOISE: {float r = (float)rand() / (float)RAND_MAX; return (2.0f * r) - 1.0f;}
    default: return 0.0f;
    }
}
static inline uint16_t applyFade(uint16_t v) {
    if (!s_wave.fadeSamples) return v;
    int32_t d = (int32_t)v - SAMPLE_MID;
    int32_t y = SAMPLE_MID + (d * (int32_t)s_wave.fadeSamples) / (int32_t)FADE_SMP;
    if (y < 0) y = 0; else if (y > 4095) y = 4095;
    return (uint16_t)y;
}

static void setNote(uint16_t f_hz, wave_t w, uint16_t amp12, uint16_t ms) {
    s_wave.wave = w;
    uint16_t a = (amp12 > AMP_MAX) ? AMP_MAX : amp12;
    s_wave.amp = (uint16_t)((float)a * s_globalGain);
    uint32_t fs = audioGetFs(s_audioDrv);
    s_wave.step = (f_hz == 0) ? 0.0f : (2.0f*(float)M_PI * ((float)f_hz / (float)fs));
    s_wave.samplesLeft = (uint32_t)ms * fs / 1000u;
    if (s_wave.samplesLeft == 0) s_wave.samplesLeft = 1;
    s_wave.fadeSamples = FADE_SMP;
}
static void nextMusic(void) {
    if (!s_music.playing || !s_music.song) { setNote(0, WF_SINE, 0, 1); return; }
    if (s_music.idx >= s_music.len) {
        if (s_music.loop) s_music.idx = 0;
        else { s_music.playing = 0; setNote(0, WF_SINE, 0, 1); return; }
    }
    const note_t* n = &s_music.song[s_music.idx++];
    setNote(n->freq_hz, n->wave, n->amp12, n->dur_ms);
}

/* ===== fill para el driver (entrega 12 bits) ===== */
static void audioFillCallback(uint16_t* dst, uint32_t n)
{
    for (uint32_t i=0;i<n;i++) {
        if (s_wave.samplesLeft == 0) {
            if (s_sfx.active) {
                if (s_sfx.preGap) {
                    /* Reproducir silencio antes del SFX */
                    s_sfx.preGap = 0;
                    setNote(0, WF_SINE, 0, SFX_GAP_MS);
                } else if (s_sfx.postGap) {
                    /* Reproducir el SFX */
                    s_sfx.postGap = 0;
                    setNote(s_sfx.sfx.freq_hz, s_sfx.sfx.wave, s_sfx.sfx.amp12, s_sfx.sfx.dur_ms);
                } else {
                    /* Reproducir silencio después del SFX y terminar */
                    s_sfx.active = 0;
                    setNote(0, WF_SINE, 0, SFX_GAP_MS);
                }
            }
            else if (s_music.playing) { 
                nextMusic(); 
            }
            else { 
                setNote(0, WF_SINE, 0, 1); 
            }
        }

        int32_t y = SAMPLE_MID;
        if (s_wave.step != 0.0f && s_wave.amp > 0) {
            float sample = waveSample(s_wave.phase, s_wave.wave);
            y = SAMPLE_MID + (int32_t)((float)s_wave.amp * sample);
            if (y < 0) y = 0; 
            else if (y > 4095) y = 4095;
        }
        dst[i] = applyFade((uint16_t)y);

        s_wave.phase += s_wave.step;
        if (s_wave.phase >= 2.0f * (float)M_PI) {
            s_wave.phase -= 2.0f * (float)M_PI;
        }
        if (s_wave.fadeSamples) s_wave.fadeSamples--;
        if (s_wave.samplesLeft) s_wave.samplesLeft--;
    }
}

/* ===== API ===== */
void playerInit(audioDrv_t *audio)
{
    /* Guardar referencia al driver */
    s_audioDrv = audio;
    
    /* Registrar callback y arrancar (el driver ya está inicializado) */
    audioSetFillFn(audio, audioFillCallback);
    audioStart(audio);
    
    /* Inicializar estados del reproductor */
    s_music.playing = 0;
    s_sfx.active = 0;
    s_wave.phase = 0.0f;
    s_globalGain = 1.0f;
}
void playerStartMusic(const note_t* song, uint32_t len, uint8_t loop)
{
    s_music.song = song;
    s_music.len = len;
    s_music.idx = 0;
    s_music.loop = loop ? 1 : 0;
    s_music.playing = 1;
    s_wave.phase = 0.0f;
    s_wave.fadeSamples = 0;
    s_wave.samplesLeft = 0;
    nextMusic();
}
void playerStopAll(void)
{
    s_music.playing = 0;
    s_music.song = 0;
    s_music.len = s_music.idx = 0;
    s_sfx.active = 0;
    setNote(0, WF_SINE, 0, 1);
}
void playerPlaySfx(note_t sfx)
{
    s_sfx.sfx = sfx;
    s_sfx.active = 1;
    
    /* Si hay música sonando, agregar gaps para separar el SFX */
    if (s_music.playing) {
        s_sfx.preGap = 1;   /* Iniciar con gap de silencio */
        s_sfx.postGap = 1;  /* Después del silencio, reproducir SFX */
        setNote(0, WF_SINE, 0, SFX_GAP_MS);  /* Comenzar con silencio */
    } else {
        /* Si no hay música, reproducir SFX directamente */
        s_sfx.preGap = 0;
        s_sfx.postGap = 0;
        setNote(sfx.freq_hz, sfx.wave, sfx.amp12, sfx.dur_ms);
    }
}
void playerSetVolume(float g)
{
    if (g < 0.0f) g = 0.0f;
    if (g > 1.0f) g = 1.0f;
    s_globalGain = g;
}
