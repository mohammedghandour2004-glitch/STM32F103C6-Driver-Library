/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< DAC_CONFIG.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : DAC_CONFIG
 * Description : This file contains the external 8-bit R-2R DAC control functionality.
 */

#ifndef DAC_CONFIG_H
#define DAC_CONFIG_H

#include "../../02_MCAL/02_DIO/DIO_interface.h"

/* COUNT sentinels keep board wiring explicitly unconfigured. */

#ifndef DAC_D0_PORT
#define DAC_D0_PORT DIO_PORT_COUNT
#endif

#ifndef DAC_D0_PIN
#define DAC_D0_PIN DIO_PIN_COUNT
#endif

#ifndef DAC_D1_PORT
#define DAC_D1_PORT DIO_PORT_COUNT
#endif

#ifndef DAC_D1_PIN
#define DAC_D1_PIN DIO_PIN_COUNT
#endif

#ifndef DAC_D2_PORT
#define DAC_D2_PORT DIO_PORT_COUNT
#endif

#ifndef DAC_D2_PIN
#define DAC_D2_PIN DIO_PIN_COUNT
#endif

#ifndef DAC_D3_PORT
#define DAC_D3_PORT DIO_PORT_COUNT
#endif

#ifndef DAC_D3_PIN
#define DAC_D3_PIN DIO_PIN_COUNT
#endif

#ifndef DAC_D4_PORT
#define DAC_D4_PORT DIO_PORT_COUNT
#endif

#ifndef DAC_D4_PIN
#define DAC_D4_PIN DIO_PIN_COUNT
#endif

#ifndef DAC_D5_PORT
#define DAC_D5_PORT DIO_PORT_COUNT
#endif

#ifndef DAC_D5_PIN
#define DAC_D5_PIN DIO_PIN_COUNT
#endif

#ifndef DAC_D6_PORT
#define DAC_D6_PORT DIO_PORT_COUNT
#endif

#ifndef DAC_D6_PIN
#define DAC_D6_PIN DIO_PIN_COUNT
#endif

#ifndef DAC_D7_PORT
#define DAC_D7_PORT DIO_PORT_COUNT
#endif

#ifndef DAC_D7_PIN
#define DAC_D7_PIN DIO_PIN_COUNT
#endif

/* GPIO output speed, not the DAC sample rate. */
#ifndef DAC_OUTPUT_SPEED
#define DAC_OUTPUT_SPEED DIO_SPEED_2MHZ
#endif

#endif
