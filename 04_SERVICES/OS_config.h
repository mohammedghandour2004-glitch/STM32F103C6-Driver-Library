/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< OS_CONFIG.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 04_SERVICES
 * Component   : OS_CONFIG
 * Description : This file contains the cooperative task scheduler functionality.
 */

#ifndef OS_CONFIG_H
#define OS_CONFIG_H

#include "../02_MCAL/06_STK/STK_interface.h"

#ifndef OS_MAX_TASKS
#define OS_MAX_TASKS 8U
#endif

/* Application must select source/period for its actual clock; these are not milliseconds. */
#ifndef OS_STK_CLOCK_SOURCE
#define OS_STK_CLOCK_SOURCE STK_CLOCK_HCLK_DIV_8
#endif
#ifndef OS_STK_PERIOD_TICKS
#define OS_STK_PERIOD_TICKS 1000U
#endif

#if (OS_MAX_TASKS < 1) || (OS_MAX_TASKS > 255)
#error OS_MAX_TASKS_must_be_1_to_255
#endif
#if (OS_STK_PERIOD_TICKS < 2) || (OS_STK_PERIOD_TICKS > 16777216)
#error OS_STK_PERIOD_TICKS_must_be_2_to_16777216
#endif

#endif
