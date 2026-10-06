/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< IR_COMMUNICATION_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : IR_COMMUNICATION_INTERFACE
 * Description : This file contains the NEC infrared communication decoding functionality.
 */

#ifndef IR_COMMUNICATION_INTERFACE_H
#define IR_COMMUNICATION_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

typedef struct
{
    u8 address;
    u8 command;
    u8 isRepeat;
} IR_Communication_Frame_t;

/* Caller captures both edges of an active-low receiver and supplies elapsed us.
 * levelAfterEdge is 0 (LOW) or 1 (HIGH); UINT32_MAX means unknown/saturated.
 * Caller owns GPIO, routing, interrupts, timestamps, and serialization of all
 * calls, including ISR/main-loop access. No internal synchronization is used.
 * Malformed input invalidates repeat association but preserves pending output.
 * An occupied slot drops the new event with ERROR; valid reception still
 * refreshes repeat tracking. Silence is handled when the next edge arrives.
 */

/* Validates timing configuration and resets reception and result state. */
ErrorStatus_t IR_Communication_Init(void);

/* Processes one transition without waiting; requires initialization. */
ErrorStatus_t IR_Communication_ProcessEdge(u32 elapsedMicroseconds, u8 levelAfterEdge);

/* Consumes a pending result; no result leaves frame unchanged and sets available=0. */
ErrorStatus_t IR_Communication_GetFrame(IR_Communication_Frame_t *frame, u8 *available);

#endif
