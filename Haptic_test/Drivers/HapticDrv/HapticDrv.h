/*
 * DRV2605.h
 *
 *  Created on: Aug 18, 2025
 *      Author: jez
 */
#ifndef HAPTICDRV_HAPTICDRV_H_
#define HAPTICDRV_HAPTICDRV_H_

#include "stm32f4xx_hal.h"

uint8_t hapticInit(I2C_HandleTypeDef *hi2c);

/**
 * Select library
 * @param lib 0 is empty, 1-5 ERM, 6 is LRA
 */
uint8_t hapticSelectLibrary(uint8_t lib);

/** 
 * Run sequence 
 */
uint8_t hapticGoSequence();

/** 
 * Stop sequence
 */
uint8_t hapticStopSequence();

/** 
 * Assign a waveform to a sequencer slot. 
 * See the datasheet, section 11.2 for a list of effects.
 * https://cdn-shop.adafruit.com/datasheets/DRV2605.pdf
 * @param slot Slot to set, 0-7
 * @param w Waveform sequence effect id (1-123).
*/
uint8_t hapticSetWaveform(uint8_t slot, uint8_t w);

/**
 * Set up to eight steps. Unused steps are emptied.
 * @param steps Pointer to an array of effect ids
 * @param n Number of steps in array (1-8)
 */
uint8_t hapticSetSequence(uint8_t *steps, uint8_t n);

/**
 * Assigns a waveform to the initial sequencer slot and plays it
 * @param id Number of effect (1 - 123)
 */
uint8_t hapticTrigger(uint8_t id);

#endif /* HAPTICDRV_HAPTICDRV_H_ */
