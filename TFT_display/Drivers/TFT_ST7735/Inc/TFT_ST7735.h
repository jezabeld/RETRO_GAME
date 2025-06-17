/*
 * TFT_ST7735.h
 *
 *  Created on: May 5, 2025
 *      Author: jez
 */

#ifndef TFT_ST7735_INC_TFT_ST7735_H_
#define TFT_ST7735_INC_TFT_ST7735_H_

#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"

#define TFT_WIDTH 128  //
#define TFT_HEIGHT 160 // LINES for 1.8"

typedef struct{
	SPI_HandleTypeDef * hSpi;
	GPIO_TypeDef * csPort;
	uint16_t csPin;
	GPIO_TypeDef * dcPort;
	uint16_t dcPin;
	GPIO_TypeDef * resPort;
	uint16_t resPin;
} tft_t;

void tftInit(tft_t * tft, SPI_HandleTypeDef * hSpi,
		GPIO_TypeDef * csPort, uint16_t csPin,
		GPIO_TypeDef * dcPort, uint16_t dcPin,
		GPIO_TypeDef * resPort, uint16_t resPin);

// solo para pruebas
void sendCommand(tft_t * tft, uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes);
void receiveParams(tft_t * tft, uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes);

#endif /* TFT_ST7735_INC_TFT_ST7735_H_ */
