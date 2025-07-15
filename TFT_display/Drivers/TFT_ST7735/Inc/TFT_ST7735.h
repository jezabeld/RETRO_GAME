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
#include "fonts.h"

#define TFT_WIDTH 128  //
#define TFT_HEIGHT 160 // LINES for 1.8"

// Color definitions
#define	TFT_COLOR_BLACK   0x0000
#define	TFT_COLOR_BLUE    0x001F
#define	TFT_COLOR_RED     0xF800
#define	TFT_COLOR_GREEN   0x07E0
#define TFT_COLOR_CYAN    0x07FF
#define TFT_COLOR_MAGENTA 0xF81F
#define TFT_COLOR_YELLOW  0xFFE0
#define TFT_COLOR_WHITE   0xFFFF

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

void tftSetAddressWindow(tft_t * tft, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void tftDrawPixel(tft_t * tft, uint8_t x, uint8_t y, uint16_t color);
void tftWriteChar(tft_t * tft, uint8_t x, uint8_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
void tftWriteString(tft_t * tft, uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void tftFillRectangle(tft_t * tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void tftFillScreen(tft_t * tft, uint16_t color);
void tftDrawImage(tft_t * tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
// solo para pruebas
//void tftSendCommand(tft_t * tft, uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes);
void receiveParams(tft_t * tft, uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes);

#endif /* TFT_ST7735_INC_TFT_ST7735_H_ */
