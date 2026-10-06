/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< HLEDMAT_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : HLEDMAT_PRIVATE
 * Description : This file contains the 8x8 monochrome LED matrix control functionality.
 */

#ifndef HLEDMAT_PRIVATE_H
#define HLEDMAT_PRIVATE_H

#include "../../02_MCAL/02_DIO/DIO_interface.h"

typedef struct
{
    DIO_Port_t port;
    DIO_Pin_t pin;
} HLEDMAT_Pin_t;

#endif
