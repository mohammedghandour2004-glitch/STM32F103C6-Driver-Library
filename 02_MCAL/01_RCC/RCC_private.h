/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< RCC_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : RCC_PRIVATE
 * Description : This file contains the private registers and internal definitions for the RCC driver.
 */

#ifndef RCC_PRIVATE_H
#define RCC_PRIVATE_H

#include <stdint.h>

#include "../../01_LIB/STD_TYPES.h"

#define RCC_BASE_ADDRESS                 0x40021000UL

#define RCC_CR_OFFSET                    0x00UL
#define RCC_CFGR_OFFSET                  0x04UL
#define RCC_APB2RSTR_OFFSET              0x0CUL
#define RCC_APB1RSTR_OFFSET              0x10UL
#define RCC_AHBENR_OFFSET                0x14UL
#define RCC_APB2ENR_OFFSET               0x18UL
#define RCC_APB1ENR_OFFSET               0x1CUL
#define RCC_CSR_OFFSET                   0x24UL

#define RCC_REGISTER(OFFSET)             (*(volatile u32 *)(uintptr_t)(RCC_BASE_ADDRESS + (OFFSET)))

#define RCC_CR                            RCC_REGISTER(RCC_CR_OFFSET)
#define RCC_CFGR                          RCC_REGISTER(RCC_CFGR_OFFSET)
#define RCC_APB2RSTR                     RCC_REGISTER(RCC_APB2RSTR_OFFSET)
#define RCC_APB1RSTR                     RCC_REGISTER(RCC_APB1RSTR_OFFSET)
#define RCC_AHBENR                       RCC_REGISTER(RCC_AHBENR_OFFSET)
#define RCC_APB2ENR                      RCC_REGISTER(RCC_APB2ENR_OFFSET)
#define RCC_APB1ENR                      RCC_REGISTER(RCC_APB1ENR_OFFSET)
#define RCC_CSR                           RCC_REGISTER(RCC_CSR_OFFSET)

#define RCC_CR_HSION_MASK                (1UL << 0)
#define RCC_CR_HSIRDY_MASK               (1UL << 1)
#define RCC_CR_HSEON_MASK                (1UL << 16)
#define RCC_CR_HSERDY_MASK               (1UL << 17)
#define RCC_CR_HSEBYP_MASK               (1UL << 18)
#define RCC_CR_PLLON_MASK                (1UL << 24)
#define RCC_CR_PLLRDY_MASK               (1UL << 25)

#define RCC_CFGR_SW_MASK                 (3UL << 0)
#define RCC_CFGR_SWS_MASK                (3UL << 2)
#define RCC_CFGR_HPRE_MASK               (15UL << 4)
#define RCC_CFGR_PPRE1_MASK              (7UL << 8)
#define RCC_CFGR_PPRE2_MASK              (7UL << 11)
#define RCC_CFGR_ADCPRE_MASK             (3UL << 14)
#define RCC_CFGR_PLLSRC_MASK             (1UL << 16)
#define RCC_CFGR_PLLXTPRE_MASK           (1UL << 17)
#define RCC_CFGR_PLLMUL_MASK             (15UL << 18)

#define RCC_CFGR_HPRE_SHIFT              4U
#define RCC_CFGR_PPRE1_SHIFT             8U
#define RCC_CFGR_PPRE2_SHIFT             11U
#define RCC_CFGR_ADCPRE_SHIFT            14U
#define RCC_CFGR_PLLMUL_SHIFT            18U

#define RCC_CFGR_CLOCK_SOURCE_HSI        0UL
#define RCC_CFGR_CLOCK_SOURCE_HSE        1UL
#define RCC_CFGR_CLOCK_SOURCE_PLL        2UL

#define RCC_CSR_RMVF_MASK                (1UL << 24)
#define RCC_CSR_RESET_FLAGS_MASK         (63UL << 26)

#define RCC_HSI_FREQUENCY_HZ             8000000UL
#define RCC_MAX_SYSCLK_HZ                72000000UL
#define RCC_MAX_HSI_PLL_SYSCLK_HZ        64000000UL
#define RCC_MAX_HCLK_HZ                  72000000UL
#define RCC_MAX_PCLK1_HZ                 36000000UL
#define RCC_MAX_PCLK2_HZ                 72000000UL
#define RCC_MAX_ADC_HZ                   14000000UL
#define RCC_MIN_PLL_INPUT_HZ             1000000UL
#define RCC_MAX_PLL_INPUT_HZ             25000000UL
#define RCC_MIN_PLL_OUTPUT_HZ            16000000UL
#define RCC_MAX_PLL_OUTPUT_HZ            72000000UL

#endif /* RCC_PRIVATE_H */
