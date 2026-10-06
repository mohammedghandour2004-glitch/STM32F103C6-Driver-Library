/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NVIC_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : NVIC_PRIVATE
 * Description : Description : This file contains the Cortex-M3 interrupt controller functionality.
 */

#ifndef NVIC_PRIVATE_H
#define NVIC_PRIVATE_H

#include "../../01_LIB/STD_TYPES.h"

#define NVIC_BASE_ADDRESS              UINT32_C(0xE000E100)
#define NVIC_ISER_OFFSET               UINT32_C(0x000)
#define NVIC_ICER_OFFSET               UINT32_C(0x080)
#define NVIC_ISPR_OFFSET               UINT32_C(0x100)
#define NVIC_ICPR_OFFSET               UINT32_C(0x180)
#define NVIC_IABR_OFFSET               UINT32_C(0x200)
#define NVIC_IPR_OFFSET                UINT32_C(0x300)
#define NVIC_SCB_BASE_ADDRESS          UINT32_C(0xE000ED00)
#define NVIC_AIRCR_OFFSET              UINT32_C(0x00C)

#define NVIC_REGISTER32(ADDRESS)       (*(volatile u32 *)(uintptr_t)(ADDRESS))
#define NVIC_REGISTER8(ADDRESS)        (*(volatile u8 *)(uintptr_t)(ADDRESS))
#define NVIC_IRQ_BANK_BITS             32U
#define NVIC_REGISTER_BYTES            4U
#define NVIC_PRIORITY_SHIFT            4U
#define NVIC_PRIORITY_MAX              15U

#define NVIC_AIRCR_KEY_MASK            UINT32_C(0xFFFF0000)
#define NVIC_AIRCR_WRITE_KEY           UINT32_C(0x05FA0000)
#define NVIC_AIRCR_GROUP_MASK          UINT32_C(0x00000700)
#define NVIC_AIRCR_GROUP_SHIFT         8U
#define NVIC_AIRCR_COMMAND_MASK        UINT32_C(0x00000007)
#define NVIC_GROUP_ENCODING_4_0        3U
#define NVIC_GROUP_ENCODING_3_1        4U
#define NVIC_GROUP_ENCODING_2_2        5U
#define NVIC_GROUP_ENCODING_1_3        6U
#define NVIC_GROUP_ENCODING_0_4        7U

#endif /* NVIC_PRIVATE_H */
