/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< STK_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : STK_INTERFACE
 * Description : This file contains the Cortex-M3 SysTick timer functionality.
 */

#ifndef STK_INTERFACE_H
#define STK_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

typedef enum
{
    STK_CLOCK_HCLK,
    STK_CLOCK_HCLK_DIV_8
} STK_ClockSource_t;

/* Caller supplies the clock frequency and serializes privileged register access. */
/* All control-changing APIs read CTRL and consume COUNTFLAG; GetCurrentValue does not. */

/* Configures a 2..16777216-tick period, preserves TICKINT, clears VAL, and leaves stopped. */
ErrorStatus_t STK_Configure(STK_ClockSource_t clockSource, u32 periodTicks);

/* Enables counting without changing LOAD, VAL, clock source, or TICKINT. */
ErrorStatus_t STK_Start(void);

/* Disables counting without changing the configured period or TICKINT. */
ErrorStatus_t STK_Stop(void);

/* Enables SysTick exception requests without changing counter enable or source. */
ErrorStatus_t STK_EnableInterrupt(void);

/* Disables new SysTick requests; an already pending exception is not cleared. */
ErrorStatus_t STK_DisableInterrupt(void);

/* Reads the 24-bit counter value, preserving output on failure. */
ErrorStatus_t STK_GetCurrentValue(u32 *currentValue);

/* Reads COUNTFLAG as 0 or 1; the CTRL read clears the flag. */
ErrorStatus_t STK_GetCountFlag(u8 *countFlag);

#endif /* STK_INTERFACE_H */
