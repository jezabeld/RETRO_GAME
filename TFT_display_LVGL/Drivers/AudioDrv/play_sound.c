/*
 * mario_sound.c
 *
 *  Created on: Aug 26, 2025
 *      Author: jez
 */
#ifdef AUD_DAC
#include <play_sound.h>

static const beep_entry_t *g_song = NULL;
static uint32_t g_song_len = 0;
static uint32_t g_song_idx = 0;
static uint32_t g_note_end_tick = 0;
static uint8_t  g_playing = 0;

void Play_Song_Start(const beep_entry_t *song, uint32_t length) {
    g_song = song;
    g_song_len = length;
    g_song_idx = 0;
    g_playing = 1;
    g_note_end_tick = 0;   // fuerza a lanzar la primera nota en update
}

void Play_Song_Update(void) {
    if (!g_playing || g_song == NULL) return;

    uint32_t now = HAL_GetTick();

    if (now >= g_note_end_tick) {
        if (g_song_idx >= g_song_len) {
            // Terminé la canción
            g_playing = 0;
            return;
        }

        const beep_entry_t *n = &g_song[g_song_idx++];

        if (n->freq > 0) {
            Audio_BeepEx(n->freq, n->dur_ms, n->wf, n->amp);
        }

        // fijar tick de final con un “+30%” de pausa
        g_note_end_tick = now + (uint32_t)(n->dur_ms * 1.3f);
    }
}
#endif
