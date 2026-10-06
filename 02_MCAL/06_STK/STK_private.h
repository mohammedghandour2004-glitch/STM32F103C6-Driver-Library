/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< STK_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : STK_PRIVATE
 * Description : This file contains the Cortex-M3 SysTick timer functionality.
 */

#ifndef STK_PRIVATE_H
#define STK_PRIVATE_H

#include "../../01_LIB/STD_TYPES.h"

#define STK_BASE_ADDRESS          UINT32_C(0xE000E010)
#define STK_CTRL_OFFSET           UINT32_C(0x00)
#define STK_LOAD_OFFSET           UINT32_C(0x04)
#define STK_VAL_OFFSET            UINT32_C(0x08)
#define STK_CALIB_OFFSET          UINT32_C(0x0C)
#define STK_REGISTER(OFFSET)      (*(volatile u32 *)(uintptr_t)(STK_BASE_ADDRESS + (OFFSET)))

#define STK_CTRL_ENABLE_MASK      (UINT32_C(1) << 0U)
#define STK_CTRL_TICKINT_MASK     (UINT32_C(1) << 1U)
#define STK_CTRL_CLKSOURCE_MASK   (UINT32_C(1) << 2U)
#define STK_CTRL_COUNTFLAG_MASK   (UINT32_C(1) << 16U)
#define STK_CTRL_WRITABLE_MASK    UINT32_C(0x00000007)
#define STK_COUNTER_MASK          UINT32_C(0x00FFFFFF)
#define STK_PERIOD_MIN            UINT32_C(2)
#define STK_PERIOD_MAX            UINT32_C(0x01000000)

#endif /* STK_PRIVATE_H */
