/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ADC_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : ADC_PRIVATE
 * Description : This file contains ADC1 and ADC2 analog-to-digital conversion functionality.
 */

#ifndef ADC_PRIVATE_H
#define ADC_PRIVATE_H

#include "../../01_LIB/STD_TYPES.h"

#define ADC1_BASE UINT32_C(0x40012400)
#define ADC2_BASE UINT32_C(0x40012800)
#define ADC_REG(BASE, OFFSET) (*(volatile u32 *)(uintptr_t)((BASE) + (OFFSET)))
#define ADC_SR_OFFSET 0x00U
#define ADC_CR1_OFFSET 0x04U
#define ADC_CR2_OFFSET 0x08U
#define ADC_SMPR1_OFFSET 0x0CU
#define ADC_SMPR2_OFFSET 0x10U
#define ADC_SQR1_OFFSET 0x2CU
#define ADC_SQR2_OFFSET 0x30U
#define ADC_SQR3_OFFSET 0x34U
#define ADC_DR_OFFSET 0x4CU
#define ADC_MIN_CLOCK UINT32_C(600000)
#define ADC_MAX_CLOCK UINT32_C(14000000)
#define ADC_MAX_RANKS 16U
#define ADC_LENGTH_SHIFT 20U
#define ADC_LENGTH_MASK UINT32_C(0x00F00000)
#define ADC_SQR1_MASK UINT32_C(0x00FFFFFF)
#define ADC_SQR23_MASK UINT32_C(0x3FFFFFFF)
#define ADC_CHANNEL_MASK UINT32_C(0x1F)
#define ADC_RESULT_MASK UINT32_C(0xFFF)
#define ADC_SR_VALID UINT32_C(0x1F)
#define ADC_DUAL_MASK UINT32_C(0x000F0000)
#define ADC_EXTSEL_MASK UINT32_C(0x000E0000)
#define ADC_EOC (UINT32_C(1) << 1U)
#define ADC_STRT (UINT32_C(1) << 4U)
#define ADC_EOCIE (UINT32_C(1) << 5U)
#define ADC_AWDIE (UINT32_C(1) << 6U)
#define ADC_JEOCIE (UINT32_C(1) << 7U)
#define ADC_SCAN (UINT32_C(1) << 8U)
#define ADC_JAUTO (UINT32_C(1) << 10U)
#define ADC_DISCEN (UINT32_C(1) << 11U)
#define ADC_JDISCEN (UINT32_C(1) << 12U)
#define ADC_JAWDEN (UINT32_C(1) << 22U)
#define ADC_AWDEN (UINT32_C(1) << 23U)
#define ADC_ADON (UINT32_C(1) << 0U)
#define ADC_CONT (UINT32_C(1) << 1U)
#define ADC_CAL (UINT32_C(1) << 2U)
#define ADC_RSTCAL (UINT32_C(1) << 3U)
#define ADC_DMA (UINT32_C(1) << 8U)
#define ADC_ALIGN (UINT32_C(1) << 11U)
#define ADC_JEXTTRIG (UINT32_C(1) << 15U)
#define ADC_EXTTRIG (UINT32_C(1) << 20U)
#define ADC_JSWSTART (UINT32_C(1) << 21U)
#define ADC_SWSTART (UINT32_C(1) << 22U)
#define ADC_TSVREFE (UINT32_C(1) << 23U)
#define ADC_CALIBRATION (ADC_CAL | ADC_RSTCAL)
#define ADC_ADVANCED (ADC_AWDIE | ADC_JEOCIE | ADC_JAUTO | ADC_DISCEN | ADC_JDISCEN | ADC_JAWDEN | ADC_AWDEN)
#define ADC_CR2_OWNED (ADC_ADON | ADC_CONT | ADC_CALIBRATION | ADC_DMA | ADC_ALIGN | ADC_JEXTTRIG | ADC_EXTTRIG | ADC_EXTSEL_MASK | ADC_JSWSTART | ADC_SWSTART)

#endif
