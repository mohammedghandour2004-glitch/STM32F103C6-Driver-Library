/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NVIC_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : NVIC_INTERFACE
 * Description : Description : This file contains the Cortex-M3 interrupt controller functionality.
 */

#ifndef NVIC_INTERFACE_H
#define NVIC_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

typedef enum
{
    NVIC_IRQ_WWDG = 0,
    NVIC_IRQ_PVD = 1,
    NVIC_IRQ_TAMPER = 2,
    NVIC_IRQ_RTC = 3,
    NVIC_IRQ_FLASH = 4,
    NVIC_IRQ_RCC = 5,
    NVIC_IRQ_EXTI0 = 6,
    NVIC_IRQ_EXTI1 = 7,
    NVIC_IRQ_EXTI2 = 8,
    NVIC_IRQ_EXTI3 = 9,
    NVIC_IRQ_EXTI4 = 10,
    NVIC_IRQ_DMA1_CHANNEL1 = 11,
    NVIC_IRQ_DMA1_CHANNEL2 = 12,
    NVIC_IRQ_DMA1_CHANNEL3 = 13,
    NVIC_IRQ_DMA1_CHANNEL4 = 14,
    NVIC_IRQ_DMA1_CHANNEL5 = 15,
    NVIC_IRQ_DMA1_CHANNEL6 = 16,
    NVIC_IRQ_DMA1_CHANNEL7 = 17,
    NVIC_IRQ_ADC1_2 = 18,
    NVIC_IRQ_USB_HP_CAN_TX = 19,
    NVIC_IRQ_USB_LP_CAN_RX0 = 20,
    NVIC_IRQ_CAN_RX1 = 21,
    NVIC_IRQ_CAN_SCE = 22,
    NVIC_IRQ_EXTI9_5 = 23,
    NVIC_IRQ_TIM1_BRK = 24,
    NVIC_IRQ_TIM1_UP = 25,
    NVIC_IRQ_TIM1_TRG_COM = 26,
    NVIC_IRQ_TIM1_CC = 27,
    NVIC_IRQ_TIM2 = 28,
    NVIC_IRQ_TIM3 = 29,
    NVIC_IRQ_I2C1_EV = 31,
    NVIC_IRQ_I2C1_ER = 32,
    NVIC_IRQ_SPI1 = 35,
    NVIC_IRQ_USART1 = 37,
    NVIC_IRQ_USART2 = 38,
    NVIC_IRQ_EXTI15_10 = 40,
    NVIC_IRQ_RTC_ALARM = 41,
    NVIC_IRQ_USB_WAKEUP = 42
} NVIC_IRQ_t;

typedef enum
{
    NVIC_GROUP_4_PREEMPT_0_SUB,
    NVIC_GROUP_3_PREEMPT_1_SUB,
    NVIC_GROUP_2_PREEMPT_2_SUB,
    NVIC_GROUP_1_PREEMPT_3_SUB,
    NVIC_GROUP_0_PREEMPT_4_SUB
} NVIC_PriorityGrouping_t;

/* NVIC control requires privileged execution. */

/* Enables one valid external interrupt. */
ErrorStatus_t NVIC_EnableIRQ(NVIC_IRQ_t irq);

/* Disables one valid external interrupt. */
ErrorStatus_t NVIC_DisableIRQ(NVIC_IRQ_t irq);

/* Sets the pending state of one external interrupt. */
ErrorStatus_t NVIC_SetPendingIRQ(NVIC_IRQ_t irq);

/* Clears NVIC pending state; the peripheral source may assert it again. */
ErrorStatus_t NVIC_ClearPendingIRQ(NVIC_IRQ_t irq);

/* Reads pending state as 0 or 1, preserving output on failure. */
ErrorStatus_t NVIC_GetPendingIRQ(NVIC_IRQ_t irq, u8 *isPending);

/* Reads active state as 0 or 1, preserving output on failure. */
ErrorStatus_t NVIC_GetActiveIRQ(NVIC_IRQ_t irq, u8 *isActive);

/* Sets logical priority 0..15; smaller values have higher priority. */
ErrorStatus_t NVIC_SetPriority(NVIC_IRQ_t irq, u8 priority);

/* Reads logical priority 0..15, preserving output on failure. */
ErrorStatus_t NVIC_GetPriority(NVIC_IRQ_t irq, u8 *priority);

/* Sets the global priority split, also affecting configurable system exceptions. */
ErrorStatus_t NVIC_SetPriorityGrouping(NVIC_PriorityGrouping_t grouping);

/* Reads the effective priority split, preserving output on failure. */
ErrorStatus_t NVIC_GetPriorityGrouping(NVIC_PriorityGrouping_t *grouping);

#endif /* NVIC_INTERFACE_H */
