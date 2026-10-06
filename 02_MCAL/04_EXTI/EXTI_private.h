/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< EXTI_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : EXTI_PRIVATE
 * Description : This file contains the external interrupt controller functionality.
 */

#ifndef EXTI_PRIVATE_H
#define EXTI_PRIVATE_H

#include "../../01_LIB/STD_TYPES.h"

#define EXTI_BASE_ADDRESS       UINT32_C(0x40010400)
#define EXTI_IMR_OFFSET         UINT32_C(0x00)
#define EXTI_EMR_OFFSET         UINT32_C(0x04)
#define EXTI_RTSR_OFFSET        UINT32_C(0x08)
#define EXTI_FTSR_OFFSET        UINT32_C(0x0C)
#define EXTI_SWIER_OFFSET       UINT32_C(0x10)
#define EXTI_PR_OFFSET          UINT32_C(0x14)
#define EXTI_LAST_LINE          18U

#define EXTI_REGISTER(OFFSET)   (*(volatile u32 *)(uintptr_t)(EXTI_BASE_ADDRESS + (OFFSET)))

#endif /* EXTI_PRIVATE_H */
