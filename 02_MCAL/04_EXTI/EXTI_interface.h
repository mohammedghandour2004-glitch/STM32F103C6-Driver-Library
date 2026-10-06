/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< EXTI_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : EXTI_INTERFACE
 * Description : This file contains the external interrupt controller functionality.
 */

#ifndef EXTI_INTERFACE_H
#define EXTI_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

/* Lines 0..15 are GPIO lines; 16=PVD, 17=RTC alarm, 18=USB wakeup. */
typedef enum
{
    EXTI_LINE_0 = 0,
    EXTI_LINE_1 = 1,
    EXTI_LINE_2 = 2,
    EXTI_LINE_3 = 3,
    EXTI_LINE_4 = 4,
    EXTI_LINE_5 = 5,
    EXTI_LINE_6 = 6,
    EXTI_LINE_7 = 7,
    EXTI_LINE_8 = 8,
    EXTI_LINE_9 = 9,
    EXTI_LINE_10 = 10,
    EXTI_LINE_11 = 11,
    EXTI_LINE_12 = 12,
    EXTI_LINE_13 = 13,
    EXTI_LINE_14 = 14,
    EXTI_LINE_15 = 15,
    EXTI_LINE_16 = 16,
    EXTI_LINE_17 = 17,
    EXTI_LINE_18 = 18
} EXTI_Line_t;

typedef enum
{
    EXTI_TRIGGER_RISING,
    EXTI_TRIGGER_FALLING,
    EXTI_TRIGGER_BOTH
} EXTI_Trigger_t;

/* Callers serialize configuration RMW operations and own GPIO routing/source setup. */

/* Selects edge detection; edges during trigger-register writes may be lost. */
ErrorStatus_t EXTI_SetTrigger(EXTI_Line_t line, EXTI_Trigger_t trigger);

/* Unmasks one EXTI interrupt request without changing event mode. */
ErrorStatus_t EXTI_EnableInterrupt(EXTI_Line_t line);

/* Masks one EXTI interrupt request without clearing pending state. */
ErrorStatus_t EXTI_DisableInterrupt(EXTI_Line_t line);

/* Unmasks one EXTI event request without changing interrupt mode. */
ErrorStatus_t EXTI_EnableEvent(EXTI_Line_t line);

/* Masks one EXTI event request without changing interrupt mode. */
ErrorStatus_t EXTI_DisableEvent(EXTI_Line_t line);

/* Requests software triggering under existing masks; clear pending before retriggering. */
ErrorStatus_t EXTI_GenerateSoftwareTrigger(EXTI_Line_t line);

/* Reads pending state as 0 or 1, preserving output on failure. */
ErrorStatus_t EXTI_GetPending(EXTI_Line_t line, u8 *isPending);

/* Clears only the selected pending flag and its software request latch. */
ErrorStatus_t EXTI_ClearPending(EXTI_Line_t line);

#endif /* EXTI_INTERFACE_H */
