/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ERROR_STATUS.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 01_LIB
 * Component   : ERROR_STATUS
 * Description : This file contains the error status definitions for the STM32 target.
 */

#ifndef ERROR_STATUS_H
#define ERROR_STATUS_H

#include "STD_TYPES.h"

/* Common project operation status. */
typedef enum
{
    ERROR_STATUS_OK = 0,

    ERROR_STATUS_ERROR = 1
} ErrorStatus_t;

#endif /* ERROR_STATUS_H */
