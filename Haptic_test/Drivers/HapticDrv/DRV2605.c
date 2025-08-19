/*
 * DRV2605.c
 *
 *  Created on: Aug 18, 2025
 *      Author: jez
 */


#include "DRV2605.h"

#ifndef DRV2605_I2C_TIMEOUT_MS
#define DRV2605_I2C_TIMEOUT_MS  10   // ms
#endif

static HAL_StatusTypeDef _wr(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t val) {
    return HAL_I2C_Mem_Write(hi2c, DRV2605_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                             &val, 1, DRV2605_I2C_TIMEOUT_MS);
}
static HAL_StatusTypeDef _rd(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *val) {
    return HAL_I2C_Mem_Read(hi2c,  DRV2605_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                             val, 1, DRV2605_I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef _soft_reset(I2C_HandleTypeDef *hi2c) {
    HAL_StatusTypeDef st = _wr(hi2c, DRV2605_REG_MODE, 0x80); // reset
    HAL_Delay(2);
    return st;
}

// Init común para ERM (open-loop, seguro y simple)
static HAL_StatusTypeDef _init_erm_common(I2C_HandleTypeDef *hi2c, uint8_t library) {
    HAL_StatusTypeDef st;

    // Reset y salir a modo INTTRIG
    st = _soft_reset(hi2c);                                  if (st) return st;
    st = _wr(hi2c, DRV2605_REG_MODE, DRV2605_MODE_INTTRIG);  if (st) return st;

    // Selección de librería
    st = _wr(hi2c, DRV2605_REG_LIBRARY, library);            if (st) return st;

    // ERM: FEEDBACK bit7=0 (ERM). 0x36 es un baseline seguro (ganancia/BEMF)
    st = _wr(hi2c, DRV2605_REG_FEEDBACK, 0x36);              if (st) return st;

    // CONTROL3 = 0x20 → ERM open-loop (ideal para empezar)
    st = _wr(hi2c, DRV2605_REG_CONTROL3, 0x20);              if (st) return st;

    // (Opcional) Ajustar intensidad con RATEDV/CLAMPV si querés:
    if (library == 0x07) { // Coin (Library F)
		st = _wr(hi2c, DRV2605_REG_RATEDV, 0x70);            if (st) return st; // ~intensidad alta
		st = _wr(hi2c, DRV2605_REG_CLAMPV, 0x90);            if (st) return st; // límite superior
	} else {              // Bar/coreless (Library A u otras ERM)
		st = _wr(hi2c, DRV2605_REG_RATEDV, 0x60);            if (st) return st; // un poco más bajo
		st = _wr(hi2c, DRV2605_REG_CLAMPV, 0x88);            if (st) return st;
	}

    return HAL_OK;
}

// ERM tipo “coin/flat”: suele sentirse mejor con Library F (0x07)
HAL_StatusTypeDef drv2605_init_erm_coin(I2C_HandleTypeDef *hi2c) {
    return _init_erm_common(hi2c, DRV2605_LIB_ERM_F);
}

// ERM cilíndrico/coreless: Library A (0x01) anda muy bien
HAL_StatusTypeDef drv2605_init_erm_bar(I2C_HandleTypeDef *hi2c) {
    return _init_erm_common(hi2c, DRV2605_LIB_ERM_A);
}

HAL_StatusTypeDef drv2605_play_effect(I2C_HandleTypeDef *hi2c, uint8_t effect_id) {
    HAL_StatusTypeDef st;
    // Cargar efecto en slot1 y marcar fin con 0 en slot2
    st = _wr(hi2c, DRV2605_REG_WAVESEQ1 + 0, effect_id);     if (st) return st;
    st = _wr(hi2c, DRV2605_REG_WAVESEQ1 + 1, 0x00);          if (st) return st;
    // GO
    st = _wr(hi2c, DRV2605_REG_GO, 1);                       if (st) return st;

    // (Opcional) Esperar fin leyendo GO hasta 0 (timeout 100 ms)
//    uint8_t go = 1; uint32_t t0 = HAL_GetTick();
//    while (go && (HAL_GetTick() - t0) < 100) {
//        st = _rd(hi2c, DRV2605_REG_GO, &go);
//        if (st) return st;
//    }
    return HAL_OK;
}

HAL_StatusTypeDef drv2605_play_sequence(I2C_HandleTypeDef *hi2c, const uint8_t *effects, uint8_t n) {
    if (!effects || n == 0) return HAL_ERROR;
    if (n > 8) n = 8;

    for (uint8_t i = 0; i < n; ++i) {
        HAL_StatusTypeDef st = _wr(hi2c, DRV2605_REG_WAVESEQ1 + i, effects[i]);
        if (st) return st;
    }
    if (n < 8) {
        HAL_StatusTypeDef st = _wr(hi2c, DRV2605_REG_WAVESEQ1 + n, 0x00);
        if (st) return st;
    }
    return _wr(hi2c, DRV2605_REG_GO, 1);
}

HAL_StatusTypeDef drv2605_read_status(I2C_HandleTypeDef *hi2c, uint8_t *status) {
    if (!status) return HAL_ERROR;
    return _rd(hi2c, DRV2605_REG_STATUS, status);
}

// Repite un mismo efecto 'reps' veces. Si reps > 8, lo parte en bloques.
HAL_StatusTypeDef drv2605_play_repeat(I2C_HandleTypeDef *hi2c,
                                      uint8_t effect_id,
                                      uint8_t reps,
                                      uint16_t gap_ms)
{
    if (reps == 0) return HAL_OK;

    while (reps > 0) {
        uint8_t chunk = (reps > 8) ? 8 : reps;

        // Cargar hasta 8 slots con el mismo effect_id
        for (uint8_t i = 0; i < chunk; ++i) {
            HAL_StatusTypeDef st = _wr(hi2c, DRV2605_REG_WAVESEQ1 + i, effect_id);
            if (st) return st;
        }
        // Cerrar con 0 si queda espacio
        if (chunk < 8) {
            HAL_StatusTypeDef st = _wr(hi2c, DRV2605_REG_WAVESEQ1 + chunk, 0x00);
            if (st) return st;
        }

        // GO
        HAL_StatusTypeDef st = _wr(hi2c, DRV2605_REG_GO, 1);
        if (st) return st;

        // Esperar a que termine este bloque (simple: 120 ms tope)
        uint8_t go = 1; uint32_t t0 = HAL_GetTick();
        while (go && (HAL_GetTick() - t0) < 120) {
            st = _rd(hi2c, DRV2605_REG_GO, &go);
            if (st) return st;
        }

        if (gap_ms) HAL_Delay(gap_ms);
        reps -= chunk;
    }
    return HAL_OK;
}

// Reproduce una secuencia fija: effects[0..n-1], luego 0 como fin.
HAL_StatusTypeDef drv2605_play_sequence_fixed(I2C_HandleTypeDef *hi2c,
                                              const uint8_t *effects,
                                              uint8_t n,
                                              uint16_t gap_ms)
{
    if (!effects || n == 0) return HAL_ERROR;
    if (n > 8) n = 8;

    for (uint8_t i = 0; i < n; ++i) {
        HAL_StatusTypeDef st = _wr(hi2c, DRV2605_REG_WAVESEQ1 + i, effects[i]);
        if (st) return st;
    }
    if (n < 8) {
        HAL_StatusTypeDef st = _wr(hi2c, DRV2605_REG_WAVESEQ1 + n, 0x00);
        if (st) return st;
    }

    HAL_StatusTypeDef st = _wr(hi2c, DRV2605_REG_GO, 1);
    if (st) return st;

    // Espera corta al final (opcional)
    if (gap_ms) HAL_Delay(gap_ms);
    return HAL_OK;
}

