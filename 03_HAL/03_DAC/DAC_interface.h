/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< DAC_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : DAC_INTERFACE
 * Description : This file contains the external 8-bit R-2R DAC control functionality.
 */

#ifndef DAC_INTERFACE_H
#define DAC_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

/* External 8-bit R-2R DAC: D0 is LSB and D7 is MSB.
 * Caller enables GPIO clocks, owns usable pins, and serializes all calls.
 * Other code must not manipulate the assigned pins.
 * Sequential writes expose intermediate codes; no glitch-free output,
 * analog settling time, or sample rate is guaranteed.
 * A failed write leaves an uncertain code and requires successful reinitialization.
 */

/* Configures all eight outputs LOW, establishing digital code zero. */
ErrorStatus_t DAC_Init(void);

/* Writes one sample in D0-to-D7 order after successful initialization. */
ErrorStatus_t DAC_Write(u8 value);

#endif
