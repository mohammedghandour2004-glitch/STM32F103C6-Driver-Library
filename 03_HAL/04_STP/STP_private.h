/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< STP_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : STP_PRIVATE
 * Description : This file contains the 74HC595 serial-to-parallel interface functionality.
 */

#ifndef STP_PRIVATE_H
#define STP_PRIVATE_H

#include "../../02_MCAL/02_DIO/DIO_interface.h"

typedef struct
{
    DIO_Port_t port;
    DIO_Pin_t pin;
} STP_Pin_t;

typedef enum
{
    STP_SIGNAL_LATCH_CLOCK,
    STP_SIGNAL_SHIFT_CLOCK,
    STP_SIGNAL_DATA,
    STP_SIGNAL_COUNT
} STP_Signal_t;

#endif
