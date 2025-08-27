/*
 * mario_sound.c
 *
 *  Created on: Aug 26, 2025
 *      Author: jez
 */
#include "AudioPlayer.h"
#include "AudioDrv.h"
#include <math.h>

/* Config */
#define AMP_MAX   1800
#define DAC_MID   2048
#define FADE_SMP  64

/* Estado */
static uint32_t       s_fs = 10000;
static const note_t*  s_song = 0;
static uint32_t       s_len = 0, s_idx = 0;
static uint8_t        s_loop = 0, s_music = 0;

static uint8_t        s_sfx_on = 0;
static note_t         s_sfx;

static float          s_gain = 1.0f;

static float          s_phase = 0.0f, s_step = 0.0f;
static wave_t         s_wave  = WF_SQUARE;
static uint16_t       s_amp   = 1200;
static uint32_t       s_left  = 0;
static uint32_t       s_fade  = 0;

/* Onda */
static inline float wave_sample(float a, wave_t wf) {
    switch (wf) {
    case WF_SINE:   return sinf(a);
    case WF_SQUARE: return (sinf(a) >= 0.0f) ? 1.0f : -1.0f;
    case WF_SAW: { float ph=a*(1.0f/(2.0f*M_PI)); ph-=(int)ph; return 2.0f*ph-1.0f; }
    case WF_TRI: { float ph=a*(1.0f/(2.0f*M_PI)); ph-=(int)ph; return (ph<0.5f)?(4.0f*ph-1.0f):(-4.0f*ph+3.0f); }
    default: return 0.0f;
    }
}
static inline uint16_t apply_fade(uint16_t v) {
    if (!s_fade) return v;
    int32_t d = (int32_t)v - DAC_MID;
    int32_t y = DAC_MID + (d * (int32_t)s_fade) / (int32_t)FADE_SMP;
    if (y < 0) y = 0; else if (y > 4095) y = 4095;
    return (uint16_t)y;
}

static void set_note(uint16_t f_hz, wave_t w, uint16_t amp12, uint16_t ms) {
    s_wave = w;
    uint16_t a = (amp12 > AMP_MAX) ? AMP_MAX : amp12;
    s_amp  = (uint16_t)((float)a * s_gain);
    s_step = (f_hz == 0) ? 0.0f : (2.0f*(float)M_PI * ((float)f_hz / (float)s_fs));
    s_left = (uint32_t)ms * s_fs / 1000u;
    if (s_left == 0) s_left = 1;
    s_fade = FADE_SMP;
}
static void next_music(void) {
    if (!s_music || !s_song) { set_note(0, WF_SINE, 0, 1); return; }
    if (s_idx >= s_len) {
        if (s_loop) s_idx = 0;
        else { s_music = 0; set_note(0, WF_SINE, 0, 1); return; }
    }
    const note_t* n = &s_song[s_idx++];
    set_note(n->freq_hz, n->wave, n->amp12, n->dur_ms);
}

/* ===== fill para el driver (entrega 12 bits) ===== */
static void fill12(uint16_t* dst, uint32_t n)
{
    for (uint32_t i=0;i<n;i++) {
        if (s_left == 0) {
            if (s_sfx_on) { s_sfx_on = 0; next_music(); }
            else if (s_music) { next_music(); }
            else { set_note(0, WF_SINE, 0, 1); }
        }

        int32_t y = DAC_MID;
        if (s_step != 0.0f && s_amp > 0) {
            float s = wave_sample(s_phase, s_wave);
            y = DAC_MID + (int32_t)((float)s_amp * s);
            if (y < 0) y = 0; else if (y > 4095) y = 4095;
        }
        dst[i] = apply_fade((uint16_t)y);

        s_phase += s_step; if (s_phase >= 2.0f*(float)M_PI) s_phase -= 2.0f*(float)M_PI;
        if (s_fade) s_fade--;
        if (s_left) s_left--;
    }
}

/* ===== API ===== */
void AudioPlayer_Init(uint32_t fs_hz)
{
    s_fs = fs_hz;
    AudioPwmDrv_Init(fs_hz);
    AudioPwmDrv_SetFill12Fn(fill12);
    AudioPwmDrv_Start();
}
void AudioPlayer_StartMusic(const note_t* song, uint32_t len, uint8_t loop)
{
    s_song = song; s_len = len; s_idx = 0; s_loop = loop ? 1:0; s_music = 1;
    s_phase = 0.0f; s_fade = 0; s_left = 0;
    next_music();
}
void AudioPlayer_StopAll(void)
{
    s_music = 0; s_sfx_on = 0; s_song = 0; s_len = s_idx = 0;
    set_note(0, WF_SINE, 0, 1);
}
void AudioPlayer_PlaySfx(note_t sfx)
{
    s_sfx = sfx; s_sfx_on = 1;
    set_note(sfx.freq_hz, sfx.wave, sfx.amp12, sfx.dur_ms);
}
void AudioPlayer_SetVolume(float g)
{
    if (g < 0.0f) g = 0.0f;
    if (g > 1.0f) g = 1.0f;
    s_gain = g;
}
