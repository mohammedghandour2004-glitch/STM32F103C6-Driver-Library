/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< STP_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : STP_INTERFACE
 * Description : This file contains the 74HC595 serial-to-parallel interface functionality.
 */

#ifndef STP_INTERFACE_H
#define STP_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

/* Caller enables GPIO clocks, owns the pins, and serializes all calls.
 * Only logical sequencing is provided; board/component timing must be verified.
 * Interrupts may lengthen pulses; GPIO call overhead is not a timing guarantee.
 * Init sets GPIO idle levels only, not the external parallel-output byte.
 * A failed transfer requires reinitialization. Before latching, only partial
 * shift data may have changed; a latch failure leaves output storage uncertain.
 */

/* Configures latch clock, shift clock, and data as LOW push-pull outputs. */
ErrorStatus_t STP_Init(void);

/* Shifts eight bits MSB first, then pulses the latch without timing delays. */
ErrorStatus_t STP_SendByte(u8 data);

#endif
