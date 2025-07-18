/**
 * @file API_spi.h
 * @brief Módulo para la utilización de la interfaz SPI1.
 *
 * @author Jez
 * @date Apr 28, 2025
 */

#ifndef API_INC_API_SPI_H_
#define API_INC_API_SPI_H_

#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"

extern SPI_HandleTypeDef hspi1;



HAL_StatusTypeDef spiInit(uint8_t mode, uint8_t speed, uint8_t cpol, uint8_t cpha);

/**
 * @brief Setea un pin a HIGH.
 *
 * @param[in] port Puerto del pin a setear.
 * @param pin Número de pin a setear.
 */
void setPin(GPIO_TypeDef * port, uint16_t pin);

/**
 * @brief Resetea un pin a LOW.
 *
 * @param[in] port Puerto del pin a resetear.
 * @param pin Número de pin a resetear.
 */
void clearPin(GPIO_TypeDef * port, uint16_t pin);

/**
 * @brief Envía un mensaje por SPI, habilitando el device mediante el pin designado como CS (chip select).
 *
 * @param[in] port Puerto del pin utilizado como CS.
 * @param pin Número de pin del CS.
 * @param message Mensaje a enviar (8 bits).
 * @return HAL_OK si la comunicación SPI fue exitosa, HAL_ERROR en caso contrario.
 *
HAL_StatusTypeDef sendTransmission(GPIO_TypeDef * csport, uint16_t cspin, const uint8_t * message);
*/
/**
 * @brief Recibe un mensaje por SPI, habilitando el device mediante el pin designado como CS (chip select).
 *
 * @param[in] port Puerto del pin utilizado como CS.
 * @param pin Número de pin del CS.
 * @param message Mensaje a recibir (máximo 16 bytes).
 * @return HAL_OK si la comunicación SPI fue exitosa, HAL_ERROR en caso contrario.
 *
HAL_StatusTypeDef receiveTransmission(GPIO_TypeDef * csport, uint16_t cspin, uint8_t * message);
*/

#endif /* API_INC_API_SPI_H_ */
