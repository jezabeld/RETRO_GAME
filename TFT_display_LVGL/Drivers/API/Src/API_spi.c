/**
 * @file API_spi.c
 * @brief Módulo para la utilización de la interfaz SPI1.
 *
 * @author Jez
 * @date Apr 28, 2025
 */

#include <API_spi.h>
#include "assert.h"

#define MAX16B (1<<16);

//SPI_HandleTypeDef hspi1;
// la definición de la variable siempre va en un archivo fuente ya que los headers se pueden importar múltiples veces
// la declaración con `extern` en el header hace que la variable sea visible desde main.c


// inicializacion en modo default (0,0,0,0)
HAL_StatusTypeDef spiInit(uint8_t mode, uint8_t speed, uint8_t cpol, uint8_t cpha){
	assert_param((!mode) || (mode==1));
	assert_param((!cpol) || (cpol==1));
	assert_param((!cpha) || (cpha==1));
	assert_param((speed >=0) || (speed<=2));

	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = (!mode)? SPI_DIRECTION_2LINES : SPI_DIRECTION_1LINE;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = (!cpol)? SPI_POLARITY_LOW : SPI_POLARITY_HIGH;
	hspi1.Init.CLKPhase = (!cpha)? SPI_PHASE_1EDGE : SPI_PHASE_2EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = (!speed)? SPI_BAUDRATEPRESCALER_32 : ((speed == 1)? SPI_BAUDRATEPRESCALER_16 : SPI_BAUDRATEPRESCALER_8);
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
		return HAL_ERROR;
	}
	return HAL_OK;
}

/*
HAL_StatusTypeDef sendTransmission(GPIO_TypeDef * csport, uint16_t cspin, const uint8_t *message){
	assert(csport);
	assert(message);
	assert_param(cspin >= 0 && cspin <16);

	HAL_StatusTypeDef status;

    clearPin(csport, cspin); // CS en bajo
    status = HAL_SPI_Transmit(&hspi1, message, 1, HAL_MAX_DELAY);
    setPin(csport, cspin);   // CS en alto

    if(status != HAL_OK){
		return HAL_ERROR;
	}
	return HAL_OK;
}

HAL_StatusTypeDef receiveTransmission(GPIO_TypeDef * csport, uint16_t cspin, uint8_t * message){
	assert(csport);
	assert(message);
	assert_param(sizeof(message)<=MAX16B);
	assert_param(cspin >= 0 && cspin <16);

	clearPin(csport, cspin); // CS en bajo
	HAL_StatusTypeDef status = HAL_SPI_Receive(&hspi1, message, (uint8_t)sizeof(*message), HAL_MAX_DELAY);
	setPin(csport, cspin);   // CS en alto

    if(status != HAL_OK){
		return HAL_ERROR;
	}
	return HAL_OK;
}*/


void setPin(GPIO_TypeDef * port, uint16_t pin){
	assert(port);
	assert_param(pin >= 0 && pin <16);

	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

void clearPin(GPIO_TypeDef * port, uint16_t pin){
	assert(port);
	assert_param(pin >= 0 && pin <16);

	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}


/**
 * @brief Envía un comando por SPI con su respectivo valor, habilitando el device mediante el
 * 	pin designado como CS (chip select).
 *
 * @param[in] hspi Puntero al handler de SPI a utilizar.
 * @param[in] port Puerto del pin utilizado como CS.
 * @param pin Número de pin del CS.
 * @param command Conamdo a enviar (8 bits iniciales).
 * @param value Valor del comando a enviar (siguientes 8 bits).
 * @return `false` si la comunicación SPI fue exitosa, `true` en caso contrario.
 *
static HAL_StatusTypeDef sendDoubleCommand(GPIO_TypeDef * csport, uint16_t cspin,  uint8_t command, uint8_t value) {
	assert(csport);
	assert_param(cspin >= 0 && cspin <16);

	HAL_StatusTypeDef status;
	uint8_t buffer[2];
    buffer[0] = command;
    buffer[1] = value;

    clearPin(csport, cspin); // CS en bajo
    status = HAL_SPI_Transmit(hspi1, buffer, 2, HAL_MAX_DELAY);
    setPin(csport, cspin);   // CS en alto

    if(status != HAL_OK){
		return HAL_ERROR;
	}
	return HAL_OK;
}
*/
