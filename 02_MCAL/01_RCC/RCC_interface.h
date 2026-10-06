/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< RCC_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : RCC_INTERFACE
 * Description : This file contains the public API and data types for the RCC driver.
 */

#ifndef RCC_INTERFACE_H
#define RCC_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

typedef enum
{
    RCC_PERIPHERAL_DMA1,
    RCC_PERIPHERAL_AFIO,
    RCC_PERIPHERAL_GPIOA,
    RCC_PERIPHERAL_GPIOB,
    RCC_PERIPHERAL_GPIOC,
    RCC_PERIPHERAL_GPIOD,
    RCC_PERIPHERAL_ADC1,
    RCC_PERIPHERAL_ADC2,
    RCC_PERIPHERAL_SPI1,
    RCC_PERIPHERAL_USART1,
    RCC_PERIPHERAL_USART2,
    RCC_PERIPHERAL_COUNT
} RCC_Peripheral_t;

typedef enum
{
    RCC_SYSTEM_CLOCK_HSI,
    RCC_SYSTEM_CLOCK_HSE,
    RCC_SYSTEM_CLOCK_PLL
} RCC_SystemClockSource_t;

typedef enum
{
    RCC_HSE_MODE_CRYSTAL,
    RCC_HSE_MODE_BYPASS
} RCC_HseMode_t;

typedef enum
{
    RCC_PLL_SOURCE_HSI_DIV2,
    RCC_PLL_SOURCE_HSE
} RCC_PllSource_t;

typedef enum
{
    RCC_PLL_HSE_DIVIDER_1 = 1,
    RCC_PLL_HSE_DIVIDER_2 = 2
} RCC_PllHseDivider_t;

typedef enum
{
    RCC_PLL_MULTIPLIER_2 = 2,
    RCC_PLL_MULTIPLIER_3 = 3,
    RCC_PLL_MULTIPLIER_4 = 4,
    RCC_PLL_MULTIPLIER_5 = 5,
    RCC_PLL_MULTIPLIER_6 = 6,
    RCC_PLL_MULTIPLIER_7 = 7,
    RCC_PLL_MULTIPLIER_8 = 8,
    RCC_PLL_MULTIPLIER_9 = 9,
    RCC_PLL_MULTIPLIER_10 = 10,
    RCC_PLL_MULTIPLIER_11 = 11,
    RCC_PLL_MULTIPLIER_12 = 12,
    RCC_PLL_MULTIPLIER_13 = 13,
    RCC_PLL_MULTIPLIER_14 = 14,
    RCC_PLL_MULTIPLIER_15 = 15,
    RCC_PLL_MULTIPLIER_16 = 16
} RCC_PllMultiplier_t;

typedef enum
{
    RCC_AHB_PRESCALER_1 = 1,
    RCC_AHB_PRESCALER_2 = 2,
    RCC_AHB_PRESCALER_4 = 4,
    RCC_AHB_PRESCALER_8 = 8,
    RCC_AHB_PRESCALER_16 = 16,
    RCC_AHB_PRESCALER_64 = 64,
    RCC_AHB_PRESCALER_128 = 128,
    RCC_AHB_PRESCALER_256 = 256,
    RCC_AHB_PRESCALER_512 = 512
} RCC_AhbPrescaler_t;

typedef enum
{
    RCC_APB_PRESCALER_1 = 1,
    RCC_APB_PRESCALER_2 = 2,
    RCC_APB_PRESCALER_4 = 4,
    RCC_APB_PRESCALER_8 = 8,
    RCC_APB_PRESCALER_16 = 16
} RCC_ApbPrescaler_t;

typedef enum
{
    RCC_ADC_PRESCALER_2 = 2,
    RCC_ADC_PRESCALER_4 = 4,
    RCC_ADC_PRESCALER_6 = 6,
    RCC_ADC_PRESCALER_8 = 8
} RCC_AdcPrescaler_t;

typedef enum
{
    RCC_CLOCK_SYSCLK,
    RCC_CLOCK_HCLK,
    RCC_CLOCK_PCLK1,
    RCC_CLOCK_PCLK2,
    RCC_CLOCK_ADC
} RCC_Clock_t;

typedef enum
{
    RCC_RESET_FLAG_LOW_POWER = (1U << 0),
    RCC_RESET_FLAG_WINDOW_WATCHDOG = (1U << 1),
    RCC_RESET_FLAG_INDEPENDENT_WATCHDOG = (1U << 2),
    RCC_RESET_FLAG_SOFTWARE = (1U << 3),
    RCC_RESET_FLAG_POWER_ON = (1U << 4),
    RCC_RESET_FLAG_PIN = (1U << 5)
} RCC_ResetFlags_t;

typedef struct
{
    RCC_SystemClockSource_t system_clock_source;
    u8 hsi_ready;
    u8 hse_ready;
    u8 pll_ready;
    u32 reset_flags;
} RCC_Status_t;

/* Applies configuration; set FPEC latency first before increasing SYSCLK above 24 MHz. */
ErrorStatus_t RCC_InitSystemClock(void);

/* Enables the clock for a supported project peripheral. */
ErrorStatus_t RCC_EnablePeripheralClock(RCC_Peripheral_t peripheral);

/* Disables the clock for a supported project peripheral. */
ErrorStatus_t RCC_DisablePeripheralClock(RCC_Peripheral_t peripheral);

/* Pulses reset for a supported resettable project peripheral. */
ErrorStatus_t RCC_ResetPeripheral(RCC_Peripheral_t peripheral);

/* Returns a frequency derived from the live RCC configuration. */
ErrorStatus_t RCC_GetClockFrequency(RCC_Clock_t clock, u32 *frequency_hz);

/* Returns selected clock, readiness, and reset-cause information. */
ErrorStatus_t RCC_GetStatus(RCC_Status_t *status);

/* Clears the RCC reset-cause flags. */
void RCC_ClearResetFlags(void);

#endif /* RCC_INTERFACE_H */
