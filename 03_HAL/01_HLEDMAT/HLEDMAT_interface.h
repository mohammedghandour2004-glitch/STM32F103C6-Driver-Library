/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< HLEDMAT_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : HLEDMAT_INTERFACE
 * Description : This file contains the 8x8 monochrome LED matrix control functionality.
 */

#ifndef HLEDMAT_INTERFACE_H
#define HLEDMAT_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

/* Caller enables GPIO clocks, provides usable pins, and serializes all calls.
 * Board circuitry owns current limits and safe startup/stalled-scan behavior.
 * frame[r] bit c controls row r, column c: 1 is ON, independent of polarity.
 * Single-buffer updates during a scan may mix old and new visual frames.
 * Refresh has no timing: full-frame rate = refresh-call frequency / 8.
 * A selected row remains active until refreshed; no flicker guarantee is made.
 */

/* Configures the matrix outputs inactive and clears the private frame. */
ErrorStatus_t HLEDMAT_Init(void);

/* Copies eight readable bytes; requires initialization and does not write GPIO. */
ErrorStatus_t HLEDMAT_SetFrame(const u8 frame[8]);

/* Scans one row without waiting; requires successful initialization. */
ErrorStatus_t HLEDMAT_Refresh(void);

#endif
