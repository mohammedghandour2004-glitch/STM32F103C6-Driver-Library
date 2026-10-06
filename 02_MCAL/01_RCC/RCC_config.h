/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< RCC_CONFIG.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : RCC_CONFIG
 * Description : This file contains the compile-time configuration parameters for the RCC driver.
 */

#ifndef RCC_CONFIG_H
#define RCC_CONFIG_H

#include "RCC_interface.h"

#define RCC_CFG_SYSTEM_CLOCK_SOURCE    RCC_SYSTEM_CLOCK_HSI
#define RCC_CFG_HSE_FREQUENCY_HZ        8000000UL
#define RCC_CFG_HSE_MODE                RCC_HSE_MODE_CRYSTAL
#define RCC_CFG_PLL_SOURCE              RCC_PLL_SOURCE_HSI_DIV2
#define RCC_CFG_PLL_HSE_DIVIDER         RCC_PLL_HSE_DIVIDER_1
#define RCC_CFG_PLL_MULTIPLIER          RCC_PLL_MULTIPLIER_2
#define RCC_CFG_AHB_PRESCALER           RCC_AHB_PRESCALER_1
#define RCC_CFG_APB1_PRESCALER          RCC_APB_PRESCALER_1
#define RCC_CFG_APB2_PRESCALER          RCC_APB_PRESCALER_1
#define RCC_CFG_ADC_PRESCALER           RCC_ADC_PRESCALER_2
#define RCC_CFG_STARTUP_TIMEOUT          100000UL

#endif /* RCC_CONFIG_H */
