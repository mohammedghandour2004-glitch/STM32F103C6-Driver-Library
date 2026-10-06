/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< AFIO_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : AFIO_PRIVATE
 * Description : This file contains alternate-function routing and remapping functionality.
 */

#ifndef AFIO_PRIVATE_H
#define AFIO_PRIVATE_H

#include "../../01_LIB/STD_TYPES.h"

#define AFIO_BASE_ADDRESS         UINT32_C(0x40010000)
#define AFIO_EVCR_OFFSET          UINT32_C(0x00)
#define AFIO_MAPR_OFFSET          UINT32_C(0x04)
#define AFIO_EXTICR1_OFFSET       UINT32_C(0x08)
#define AFIO_EXTICR2_OFFSET       UINT32_C(0x0C)
#define AFIO_EXTICR3_OFFSET       UINT32_C(0x10)
#define AFIO_EXTICR4_OFFSET       UINT32_C(0x14)
#define AFIO_REGISTER(OFFSET)     (*(volatile u32 *)(uintptr_t)(AFIO_BASE_ADDRESS + (OFFSET)))

#define AFIO_PORT_A_ENCODING      0U
#define AFIO_PORT_B_ENCODING      1U
#define AFIO_PORT_C_ENCODING      2U
#define AFIO_PORT_D_ENCODING      3U
#define AFIO_EXTI_LAST_LINE       15U
#define AFIO_EXTICR_FIELDS        4U
#define AFIO_EXTICR_FIELD_BITS    4U
#define AFIO_EXTICR_FIELD_MASK    UINT32_C(0xF)
#define AFIO_REGISTER_BYTES       4U

#define AFIO_MAPR_SPI1_MASK        (UINT32_C(1) << 0U)
#define AFIO_MAPR_USART1_MASK      (UINT32_C(1) << 2U)
#define AFIO_MAPR_USART2_MASK      (UINT32_C(1) << 3U)
#define AFIO_MAPR_SWJ_SHIFT        24U
#define AFIO_MAPR_SWJ_MASK         (UINT32_C(7) << AFIO_MAPR_SWJ_SHIFT)
#define AFIO_SWJ_FULL_ENCODING     0U
#define AFIO_SWJ_NO_NJTRST_ENCODING 1U
#define AFIO_SWJ_SWD_ENCODING      2U
#define AFIO_SWJ_DISABLED_ENCODING 4U

#define AFIO_EVCR_PIN_MASK         UINT32_C(0x0F)
#define AFIO_EVCR_PORT_MASK        UINT32_C(0x70)
#define AFIO_EVCR_PORT_SHIFT       4U
#define AFIO_EVCR_EVOE_MASK        (UINT32_C(1) << 7U)
#define AFIO_EVENT_LAST_PIN        15U

#endif /* AFIO_PRIVATE_H */
