/*
 * TFT_CMD.c
 *
 *  Created on: May 5, 2025
 *      Author: jez
 */

#include "TFT_ST7735.h"
#include "stdint.h"

extern SPI_HandleTypeDef hspi1;

#define ST_CMD_DELAY 0x80 // special signifier for command lists
#define MAX_DELAY_MS 500
// Commands
#define CMD_NOP 0x00
#define CMD_SWRESET 0x01
#define CMD_RDDID 0x04
#define CMD_RDDST 0x09

#define CMD_SLPIN 0x10
#define CMD_SLPOUT 0x11
#define CMD_PTLON 0x12
#define CMD_NORON 0x13

#define CMD_INVOFF 0x20
#define CMD_INVON 0x21
#define CMD_DISPOFF 0x28
#define CMD_DISPON 0x29
#define CMD_CASET 0x2A
#define CMD_RASET 0x2B
#define CMD_RAMWR 0x2C
#define CMD_RAMRD 0x2E

#define CMD_PTLAR 0x30
#define CMD_TEOFF 0x34
#define CMD_TEON 0x35
#define CMD_MADCTL 0x36
#define CMD_COLMOD 0x3A

#define CMD_FRMCTR1 0xB1
#define CMD_FRMCTR2 0xB2
#define CMD_FRMCTR3 0xB3
#define CMD_INVCTR 0xB4
#define CMD_DISSET5 0xB6

#define CMD_PWCTR1 0xC0
#define CMD_PWCTR2 0xC1
#define CMD_PWCTR3 0xC2
#define CMD_PWCTR4 0xC3
#define CMD_PWCTR5 0xC4
#define CMD_VMCTR1 0xC5

#define CMD_RDID1 0xDA
#define CMD_RDID2 0xDB
#define CMD_RDID3 0xDC
#define CMD_RDID4 0xDD

#define CMD_GMCTRP1 0xE0
#define CMD_GMCTRN1 0xE1

#define CMD_PWCTR6 0xFC

// Parameters
#define FRMCTR_RTNA 0x01 // número de clocks por línea
#define FRMCTR_FPA 0x2C  // front porch (espera antes de cada línea)
#define FRMCTR_BPA 0x2D  // back porch (espera después de cada línea)
#define INVCTR_LINE 0x07

#define PWCTR_AVDD (5<<5)	// 5V
#define PWCTR_VRH 0x02 		//+- 4.6 V
#define PWCTR_MODEAUTO 0x84

#define MADCTL_MY 0x80
#define MADCTL_MX 0x40
#define MADCTL_MV 0x20
#define MADCTL_ML 0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH 0x04

#define COLMOD_16BIT 0x05

static const uint8_t initCmds[] = {    // ST7735R init
	21,                             // 21 commands in list:
	CMD_SWRESET,   ST_CMD_DELAY, 	//  1: Software reset, 0 args, w/delay
	  150,                          //     150 ms delay
	CMD_SLPOUT,    ST_CMD_DELAY, 	//  2: Out of sleep mode, 0 args, w/delay
	  255,                          //     500 ms delay (255 means max value)
	CMD_FRMCTR1, 3,              	//  3: Framerate ctrl - normal mode, 3 arg: RTNA, FPA, BPA
	  FRMCTR_RTNA, FRMCTR_FPA, 		//     Rate = fosc/((RTNA x 2 + 40) x (LINE + FPA + BPA))
	  FRMCTR_BPA,					//	   fosc = 625kHz
	CMD_FRMCTR2, 3,              	//  4: Framerate ctrl - idle mode, 3 args:
	  FRMCTR_RTNA, FRMCTR_FPA, 		//     Rate = fosc/((RTNA x 2 + 40) x (LINE + FPA + BPA))
	  FRMCTR_BPA,
	CMD_FRMCTR3, 6,              	//  5: Framerate - partial mode, 6 args:
	  FRMCTR_RTNA, FRMCTR_FPA, 		//     RTNA, FPA, BPA for Dot inversion mode
	  FRMCTR_BPA, FRMCTR_RTNA,  	//     RTNA, FPA, BPA for Line inversion mode
	  FRMCTR_FPA, FRMCTR_BPA,
	CMD_INVCTR,  1,              	//  6: Display inversion ctrl, 1 arg:
	  INVCTR_LINE,                  //     Line inversion for all modes (Normal mode, Idle mode, partial mode)
	CMD_PWCTR1,  3,              	//  7: Power control, 3 args, no delay:
	  PWCTR_AVDD|PWCTR_VRH,			//	   AVDD: 5V, VRHP: +4.6V
	  PWCTR_VRH,                    //     VRHN: -4.6V
	  PWCTR_MODEAUTO,               //     AUTO mode
	CMD_PWCTR2,  1,              	//  8: Power control, 1 arg, no delay:
	  0xC5,                        	//     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
	CMD_PWCTR3,  2,              	//  9: Power control, 2 args, no delay:
	  0x0A,                        	//     Opamp current small
	  0x00,                         //     Boost frequency
	CMD_PWCTR4,  2,              	// 10: Power control, 2 args, no delay:
	  0x8A,                         //     BCLK/2,
	  0x2A,                         //     opamp current small & medium low
	CMD_PWCTR5,  2,              	// 11: Power control, 2 args, no delay:
	  0x8A, 0xEE,
	CMD_VMCTR1,  1,              	// 12: Power control, 1 arg, no delay:
	  0x0E,
	CMD_INVOFF,  0,              	// 13: Don't invert display, no args
	CMD_MADCTL,  1,              	// 14: Mem access ctl (directions), 1 arg:
	  MADCTL_MY|MADCTL_MX|MADCTL_RGB,//     row/col addr, top-to-bottom refresh, RGB mode
	CMD_COLMOD,  1,              	// 15: set color mode, 1 arg, no delay:
	  COLMOD_16BIT,                 //     16-bit color
	CMD_CASET,   4,              	// 16: Column addr set, 4 args, no delay:
	  0x00, 0x00,                   //     XSTART = 0 (2 bytes data)
	  0x00, 0x7F,                   //     XEND = 127 (2 bytes data)
	CMD_RASET,   4,              	// 17: Row addr set, 4 args, no delay:
	  0x00, 0x00,                   //     XSTART = 0
	  0x00, 0x9F,                 	//     XEND = 159
	CMD_GMCTRP1, 16      ,       	// 18: Gamma Adjustments (pos. polarity), 16 args + delay:
	  0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
	  0x37, 0x32, 0x29, 0x2d,       //     accurate colors)
	  0x29, 0x25, 0x2B, 0x39,
	  0x00, 0x01, 0x03, 0x10,
	CMD_GMCTRN1, 16      ,       	// 19: Gamma Adjustments (neg. polarity), 16 args + delay:
	  0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
	  0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
	  0x2E, 0x2E, 0x37, 0x3F,
	  0x00, 0x00, 0x02, 0x10,
	CMD_NORON,     ST_CMD_DELAY, 	// 20: Normal display on, no args, w/delay
	  10,                           //     10 ms delay
	CMD_DISPON,    ST_CMD_DELAY, 	// 21: Display on, no args w/delay
	  100                         	//     100 ms delay
};

void sendCommand(tft_t * tft, uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes)
{
  HAL_GPIO_WritePin(tft->csPort, tft->csPin, GPIO_PIN_RESET); // CS low
  HAL_GPIO_WritePin(tft->dcPort, tft->dcPin, GPIO_PIN_RESET); // command mode

  HAL_SPI_Transmit(tft->hSpi, &commandByte, 1, HAL_MAX_DELAY); // send command

  HAL_GPIO_WritePin(tft->dcPort, tft->dcPin, GPIO_PIN_SET); // param/data mode

  for (int i = 0; i < numDataBytes; i++) {					// send params
	  HAL_SPI_Transmit(tft->hSpi, dataBytes, 1, HAL_MAX_DELAY);
      dataBytes++; // avanza el puntero
  }
  HAL_GPIO_WritePin(tft->csPort,tft->csPin, GPIO_PIN_SET);	// CS high
}

void setRotation(tft_t * tft){
	uint8_t madctl = MADCTL_MX | MADCTL_MY | MADCTL_RGB;
	sendCommand(tft, CMD_MADCTL, &madctl, 1);
}

void setUpDisplay(tft_t * tft, const uint8_t * addr)
{
	uint8_t numCommands, cmd, numArgs;
	uint16_t ms;

	numCommands = *(addr++); // primer dato es cantidad de comandos a leer
	while (numCommands--) {              // For each command...
		cmd = *(addr++);       // Load command
		numArgs = *(addr++);   // Number of args to follow
		ms = numArgs & ST_CMD_DELAY;       // If hi-bit set, delay follows args
		numArgs &= ~ST_CMD_DELAY;          // Mask out delay bit

		sendCommand(tft, cmd, addr, numArgs);
		addr += numArgs;

		if (ms) {
		  ms = *(addr++); // Read post-command delay time (ms)
		  if (ms == 255){
			ms = MAX_DELAY_MS; // If 255, delay for 500 ms
		  }
		  HAL_Delay(ms);
		}
	}
}

void tftInit(tft_t * tft, SPI_HandleTypeDef * hSpi,
		GPIO_TypeDef * csPort, uint16_t csPin,
		GPIO_TypeDef * dcPort, uint16_t dcPin,
		GPIO_TypeDef * resPort, uint16_t resPin)
{

	tft->hSpi = hSpi;
	tft->csPort = csPort;
	tft->csPin = csPin;
	tft->dcPort = dcPort;
	tft->dcPin = dcPin;
	tft->resPort = resPort;
	tft->resPin = resPin;

	setUpDisplay(tft, initCmds);
}

void receiveParams(tft_t * tft, uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes)
{
  HAL_GPIO_WritePin(tft->csPort, tft->csPin, GPIO_PIN_RESET); // CS low
  HAL_GPIO_WritePin(tft->dcPort, tft->dcPin, GPIO_PIN_RESET); // command mode

  HAL_SPI_Transmit(tft->hSpi, &commandByte, 1, HAL_MAX_DELAY); // send command

  HAL_GPIO_WritePin(tft->dcPort, tft->dcPin, GPIO_PIN_SET); // param/data mode

  for (int i = 0; i < numDataBytes; i++) {					// receive params
	  HAL_SPI_Receive(tft->hSpi, dataBytes, 1, HAL_MAX_DELAY);
      dataBytes++; // avanza el puntero
  }
  HAL_GPIO_WritePin(tft->csPort,tft->csPin, GPIO_PIN_SET);	// CS high
}
