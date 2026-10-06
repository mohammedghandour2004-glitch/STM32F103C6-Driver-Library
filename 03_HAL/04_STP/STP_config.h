/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< STP_CONFIG.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : STP_CONFIG
 * Description : This file contains the 74HC595 serial-to-parallel interface functionality.
 */

#ifndef STP_CONFIG_H
#define STP_CONFIG_H

#include "../../02_MCAL/02_DIO/DIO_interface.h"

/* COUNT sentinels leave wiring unconfigured until the board mapping is supplied. */

#ifndef STP_DATA_PORT
#define STP_DATA_PORT DIO_PORT_COUNT
#endif

#ifndef STP_DATA_PIN
#define STP_DATA_PIN DIO_PIN_COUNT
#endif

#ifndef STP_SHIFT_CLOCK_PORT
#define STP_SHIFT_CLOCK_PORT DIO_PORT_COUNT
#endif

#ifndef STP_SHIFT_CLOCK_PIN
#define STP_SHIFT_CLOCK_PIN DIO_PIN_COUNT
#endif

#ifndef STP_LATCH_CLOCK_PORT
#define STP_LATCH_CLOCK_PORT DIO_PORT_COUNT
#endif

#ifndef STP_LATCH_CLOCK_PIN
#define STP_LATCH_CLOCK_PIN DIO_PIN_COUNT
#endif

/* GPIO output speed, not the serial bit rate. */
#ifndef STP_OUTPUT_SPEED
#define STP_OUTPUT_SPEED DIO_SPEED_2MHZ
#endif

#endif
