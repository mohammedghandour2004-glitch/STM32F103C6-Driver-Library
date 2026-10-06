/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< DIO_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : DIO_PRIVATE
 * Description : This file contains the GPIO digital input/output control functionality.
 */

#ifndef DIO_PRIVATE_H
#define DIO_PRIVATE_H

#include <stdint.h>

#include "../../01_LIB/STD_TYPES.h"

#define DIO_GPIOA_BASE_ADDRESS          0x40010800UL
#define DIO_GPIOB_BASE_ADDRESS          0x40010C00UL
#define DIO_GPIOC_BASE_ADDRESS          0x40011000UL
#define DIO_GPIOD_BASE_ADDRESS          0x40011400UL

#define DIO_CRL_OFFSET                  0x00UL
#define DIO_CRH_OFFSET                  0x04UL
#define DIO_IDR_OFFSET                  0x08UL
#define DIO_ODR_OFFSET                  0x0CUL
#define DIO_BSRR_OFFSET                 0x10UL
#define DIO_BRR_OFFSET                  0x14UL
#define DIO_LCKR_OFFSET                 0x18UL

#define DIO_REGISTER(BASE, OFFSET)      (*(volatile u32 *)(uintptr_t)((BASE) + (OFFSET)))

#define DIO_CONFIGURATION_FIELD_MASK    0x0FUL
#define DIO_CNF_SHIFT                   2U
#define DIO_INPUT_MODE_BITS             0U
#define DIO_INPUT_ANALOG_CNF            0U
#define DIO_INPUT_FLOATING_CNF          1U
#define DIO_INPUT_PULL_CNF              2U
#define DIO_OUTPUT_PUSH_PULL_CNF        0U
#define DIO_OUTPUT_OPEN_DRAIN_CNF       1U
#define DIO_ALTERNATE_PUSH_PULL_CNF     2U
#define DIO_ALTERNATE_OPEN_DRAIN_CNF    3U
#define DIO_SPEED_10MHZ_BITS            1U
#define DIO_SPEED_2MHZ_BITS             2U
#define DIO_SPEED_50MHZ_BITS            3U

#define DIO_LCKR_KEY_MASK               (1UL << 16)
#define DIO_LCKR_PIN_MASK               0xFFFFUL

#endif /* DIO_PRIVATE_H */
