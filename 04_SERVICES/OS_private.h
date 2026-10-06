/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< OS_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 04_SERVICES
 * Component   : OS_PRIVATE
 * Description : This file contains the cooperative task scheduler functionality.
 */

#ifndef OS_PRIVATE_H
#define OS_PRIVATE_H

#include "OS_interface.h"

typedef enum
{
    OS_STATE_UNINITIALIZED,
    OS_STATE_INITIALIZED,
    OS_STATE_STARTED
} OS_State_t;

typedef struct
{
    OS_TaskFunction_t function;
    u32 periodTicks;
    u32 remainingTicks;
    volatile u8 request;
    volatile u8 acknowledgment;
} OS_TaskControl_t;

#endif
