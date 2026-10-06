/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< TFT_CONFIG.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : TFT_CONFIG
 * Description : This file contains the ST7735S TFT display control functionality.
 */

#ifndef TFT_CONFIG_H
#define TFT_CONFIG_H

#include "../../02_MCAL/02_DIO/DIO_interface.h"
#include "../../02_MCAL/08_SPI/SPI_interface.h"

#ifndef TFT_SPI_INSTANCE
#define TFT_SPI_INSTANCE SPI_INSTANCE_1
#endif

/* COUNT sentinels require an application-supplied board mapping. */
#ifndef TFT_CS_PORT
#define TFT_CS_PORT DIO_PORT_COUNT
#endif
#ifndef TFT_CS_PIN
#define TFT_CS_PIN DIO_PIN_COUNT
#endif
#ifndef TFT_DC_PORT
#define TFT_DC_PORT DIO_PORT_COUNT
#endif
#ifndef TFT_DC_PIN
#define TFT_DC_PIN DIO_PIN_COUNT
#endif
#ifndef TFT_RST_PORT
#define TFT_RST_PORT DIO_PORT_COUNT
#endif
#ifndef TFT_RST_PIN
#define TFT_RST_PIN DIO_PIN_COUNT
#endif

/* GPIO output speed, not the SPI clock rate. */
#ifndef TFT_OUTPUT_SPEED
#define TFT_OUTPUT_SPEED DIO_SPEED_2MHZ
#endif

/* Nonzero u32 iteration budgets, not milliseconds. Transmit uses SPI's retry
 * budget; completion allows this many checks EACH for TXE and BSY per byte.
 */
#ifndef TFT_SPI_TRANSMIT_TIMEOUT
#define TFT_SPI_TRANSMIT_TIMEOUT 100000UL
#endif
#ifndef TFT_SPI_COMPLETION_TIMEOUT
#define TFT_SPI_COMPLETION_TIMEOUT 100000UL
#endif

#endif
