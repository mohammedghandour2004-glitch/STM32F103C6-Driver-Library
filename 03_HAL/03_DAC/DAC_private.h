/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< DAC_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : DAC_PRIVATE
 * Description : This file contains the external 8-bit R-2R DAC control functionality.
 */

#ifndef DAC_PRIVATE_H
#define DAC_PRIVATE_H

#include "../../02_MCAL/02_DIO/DIO_interface.h"

typedef struct
{
    DIO_Port_t port;
    DIO_Pin_t pin;
} DAC_Pin_t;

#endif
