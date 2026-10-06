/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< RCC_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : RCC_PROGRAM
 * Description : This file contains the implementation of the RCC driver.
 */

#include "RCC_interface.h"
#include "RCC_config.h"
#include "RCC_private.h"

typedef struct
{
    u32 sysclk_hz;
    u32 hclk_hz;
    u32 pclk1_hz;
    u32 pclk2_hz;
    u32 adc_hz;
} RCC_ClockTree_t;

/* Waits for a register flag to reach the requested state. */
static ErrorStatus_t RCC_WaitForFlag(volatile u32 *register_address, u32 mask, u8 set)
{
    u32 counter;

    for (counter = 0U; counter < RCC_CFG_STARTUP_TIMEOUT; ++counter)
    {
        if (((*register_address & mask) != 0U) == (set != 0U))
        {
            return ERROR_STATUS_OK;
        }
    }

    return ERROR_STATUS_ERROR;
}

/* Waits until hardware confirms the requested system clock source. */
static ErrorStatus_t RCC_WaitForSystemClockSource(RCC_SystemClockSource_t source)
{
    u32 counter;
    u32 expected_status;

    switch (source)
    {
        case RCC_SYSTEM_CLOCK_HSI:
            expected_status = RCC_CFGR_CLOCK_SOURCE_HSI;
            break;

        case RCC_SYSTEM_CLOCK_HSE:
            expected_status = RCC_CFGR_CLOCK_SOURCE_HSE;
            break;

        case RCC_SYSTEM_CLOCK_PLL:
            expected_status = RCC_CFGR_CLOCK_SOURCE_PLL;
            break;

        default:
            return ERROR_STATUS_ERROR;
    }

    for (counter = 0U; counter < RCC_CFG_STARTUP_TIMEOUT; ++counter)
    {
        if (((RCC_CFGR & RCC_CFGR_SWS_MASK) >> 2U) == expected_status)
        {
            return ERROR_STATUS_OK;
        }
    }

    return ERROR_STATUS_ERROR;
}

/* Translates a semantic AHB divider into its private register encoding. */
static ErrorStatus_t RCC_EncodeAhbPrescaler(RCC_AhbPrescaler_t prescaler, u32 *encoding)
{
    u32 value;

    if (encoding == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    switch (prescaler)
    {
        case RCC_AHB_PRESCALER_1: value = 0U; break;
        case RCC_AHB_PRESCALER_2: value = 8U; break;
        case RCC_AHB_PRESCALER_4: value = 9U; break;
        case RCC_AHB_PRESCALER_8: value = 10U; break;
        case RCC_AHB_PRESCALER_16: value = 11U; break;
        case RCC_AHB_PRESCALER_64: value = 12U; break;
        case RCC_AHB_PRESCALER_128: value = 13U; break;
        case RCC_AHB_PRESCALER_256: value = 14U; break;
        case RCC_AHB_PRESCALER_512: value = 15U; break;
        default: return ERROR_STATUS_ERROR;
    }

    *encoding = value;
    return ERROR_STATUS_OK;
}

/* Translates a semantic APB divider into its private register encoding. */
static ErrorStatus_t RCC_EncodeApbPrescaler(RCC_ApbPrescaler_t prescaler, u32 *encoding)
{
    u32 value;

    if (encoding == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    switch (prescaler)
    {
        case RCC_APB_PRESCALER_1: value = 0U; break;
        case RCC_APB_PRESCALER_2: value = 4U; break;
        case RCC_APB_PRESCALER_4: value = 5U; break;
        case RCC_APB_PRESCALER_8: value = 6U; break;
        case RCC_APB_PRESCALER_16: value = 7U; break;
        default: return ERROR_STATUS_ERROR;
    }

    *encoding = value;
    return ERROR_STATUS_OK;
}

/* Translates a semantic ADC divider into its private register encoding. */
static ErrorStatus_t RCC_EncodeAdcPrescaler(RCC_AdcPrescaler_t prescaler, u32 *encoding)
{
    u32 value;

    if (encoding == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    switch (prescaler)
    {
        case RCC_ADC_PRESCALER_2: value = 0U; break;
        case RCC_ADC_PRESCALER_4: value = 1U; break;
        case RCC_ADC_PRESCALER_6: value = 2U; break;
        case RCC_ADC_PRESCALER_8: value = 3U; break;
        default: return ERROR_STATUS_ERROR;
    }

    *encoding = value;
    return ERROR_STATUS_OK;
}

/* Decodes a private AHB prescaler encoding. */
static ErrorStatus_t RCC_DecodeAhbPrescaler(u32 encoding, u32 *divider)
{
    if (divider == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    if (encoding < 8U)
    {
        *divider = 1U;
        return ERROR_STATUS_OK;
    }

    switch (encoding)
    {
        case 8U: *divider = 2U; break;
        case 9U: *divider = 4U; break;
        case 10U: *divider = 8U; break;
        case 11U: *divider = 16U; break;
        case 12U: *divider = 64U; break;
        case 13U: *divider = 128U; break;
        case 14U: *divider = 256U; break;
        case 15U: *divider = 512U; break;
        default: return ERROR_STATUS_ERROR;
    }

    return ERROR_STATUS_OK;
}

/* Decodes a private APB prescaler encoding. */
static ErrorStatus_t RCC_DecodeApbPrescaler(u32 encoding, u32 *divider)
{
    if (divider == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    if (encoding < 4U)
    {
        *divider = 1U;
        return ERROR_STATUS_OK;
    }

    switch (encoding)
    {
        case 4U: *divider = 2U; break;
        case 5U: *divider = 4U; break;
        case 6U: *divider = 8U; break;
        case 7U: *divider = 16U; break;
        default: return ERROR_STATUS_ERROR;
    }

    return ERROR_STATUS_OK;
}

/* Decodes a private ADC prescaler encoding. */
static ErrorStatus_t RCC_DecodeAdcPrescaler(u32 encoding, u32 *divider)
{
    if (divider == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    switch (encoding)
    {
        case 0U: *divider = 2U; break;
        case 1U: *divider = 4U; break;
        case 2U: *divider = 6U; break;
        case 3U: *divider = 8U; break;
        default: return ERROR_STATUS_ERROR;
    }

    return ERROR_STATUS_OK;
}

/* Validates the configured HSE mode and frequency. */
static ErrorStatus_t RCC_ValidateHseConfiguration(void)
{
    if (RCC_CFG_HSE_MODE == RCC_HSE_MODE_CRYSTAL)
    {
        return ((RCC_CFG_HSE_FREQUENCY_HZ >= 4000000UL) &&
                (RCC_CFG_HSE_FREQUENCY_HZ <= 16000000UL)) ?
                ERROR_STATUS_OK : ERROR_STATUS_ERROR;
    }

    if (RCC_CFG_HSE_MODE == RCC_HSE_MODE_BYPASS)
    {
        return ((RCC_CFG_HSE_FREQUENCY_HZ >= 1000000UL) &&
                (RCC_CFG_HSE_FREQUENCY_HZ <= 25000000UL)) ?
                ERROR_STATUS_OK : ERROR_STATUS_ERROR;
    }

    return ERROR_STATUS_ERROR;
}

/* Calculates the configured PLL frequency without touching hardware. */
static ErrorStatus_t RCC_GetConfiguredPllFrequency(u32 *pll_frequency_hz)
{
    u32 pll_input_hz;
    u64 pll_output_hz;

    if (pll_frequency_hz == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    if ((RCC_CFG_PLL_MULTIPLIER < RCC_PLL_MULTIPLIER_2) ||
        (RCC_CFG_PLL_MULTIPLIER > RCC_PLL_MULTIPLIER_16))
    {
        return ERROR_STATUS_ERROR;
    }

    if (RCC_CFG_PLL_SOURCE == RCC_PLL_SOURCE_HSI_DIV2)
    {
        pll_input_hz = RCC_HSI_FREQUENCY_HZ / 2U;
    }
    else if (RCC_CFG_PLL_SOURCE == RCC_PLL_SOURCE_HSE)
    {
        if ((RCC_CFG_PLL_HSE_DIVIDER != RCC_PLL_HSE_DIVIDER_1) &&
            (RCC_CFG_PLL_HSE_DIVIDER != RCC_PLL_HSE_DIVIDER_2))
        {
            return ERROR_STATUS_ERROR;
        }

        pll_input_hz = RCC_CFG_HSE_FREQUENCY_HZ / (u32)RCC_CFG_PLL_HSE_DIVIDER;
    }
    else
    {
        return ERROR_STATUS_ERROR;
    }

    if ((pll_input_hz < RCC_MIN_PLL_INPUT_HZ) || (pll_input_hz > RCC_MAX_PLL_INPUT_HZ))
    {
        return ERROR_STATUS_ERROR;
    }

    pll_output_hz = (u64)pll_input_hz * (u64)RCC_CFG_PLL_MULTIPLIER;
    if ((pll_output_hz < RCC_MIN_PLL_OUTPUT_HZ) || (pll_output_hz > RCC_MAX_PLL_OUTPUT_HZ))
    {
        return ERROR_STATUS_ERROR;
    }

    *pll_frequency_hz = (u32)pll_output_hz;
    return ERROR_STATUS_OK;
}

/* Builds and validates the complete compile-time clock tree. */
static ErrorStatus_t RCC_BuildConfiguredClockTree(RCC_ClockTree_t *clock_tree)
{
    RCC_ClockTree_t calculated_tree;
    u32 ahb_divider;
    u32 apb1_divider;
    u32 apb2_divider;
    u32 adc_divider;

    if ((clock_tree == NULL) || (RCC_CFG_STARTUP_TIMEOUT == 0U) ||
        (RCC_ValidateHseConfiguration() != ERROR_STATUS_OK) ||
        (RCC_EncodeAhbPrescaler(RCC_CFG_AHB_PRESCALER, &ahb_divider) != ERROR_STATUS_OK) ||
        (RCC_EncodeApbPrescaler(RCC_CFG_APB1_PRESCALER, &apb1_divider) != ERROR_STATUS_OK) ||
        (RCC_EncodeApbPrescaler(RCC_CFG_APB2_PRESCALER, &apb2_divider) != ERROR_STATUS_OK) ||
        (RCC_EncodeAdcPrescaler(RCC_CFG_ADC_PRESCALER, &adc_divider) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    switch (RCC_CFG_SYSTEM_CLOCK_SOURCE)
    {
        case RCC_SYSTEM_CLOCK_HSI:
            calculated_tree.sysclk_hz = RCC_HSI_FREQUENCY_HZ;
            break;

        case RCC_SYSTEM_CLOCK_HSE:
            calculated_tree.sysclk_hz = RCC_CFG_HSE_FREQUENCY_HZ;
            break;

        case RCC_SYSTEM_CLOCK_PLL:
            if (RCC_GetConfiguredPllFrequency(&calculated_tree.sysclk_hz) != ERROR_STATUS_OK)
            {
                return ERROR_STATUS_ERROR;
            }
            if ((RCC_CFG_PLL_SOURCE == RCC_PLL_SOURCE_HSI_DIV2) &&
                (calculated_tree.sysclk_hz > RCC_MAX_HSI_PLL_SYSCLK_HZ))
            {
                return ERROR_STATUS_ERROR;
            }
            break;

        default:
            return ERROR_STATUS_ERROR;
    }

    calculated_tree.hclk_hz = calculated_tree.sysclk_hz / (u32)RCC_CFG_AHB_PRESCALER;
    calculated_tree.pclk1_hz = calculated_tree.hclk_hz / (u32)RCC_CFG_APB1_PRESCALER;
    calculated_tree.pclk2_hz = calculated_tree.hclk_hz / (u32)RCC_CFG_APB2_PRESCALER;
    calculated_tree.adc_hz = calculated_tree.pclk2_hz / (u32)RCC_CFG_ADC_PRESCALER;

    if ((calculated_tree.sysclk_hz > RCC_MAX_SYSCLK_HZ) ||
        (calculated_tree.hclk_hz > RCC_MAX_HCLK_HZ) ||
        (calculated_tree.pclk1_hz > RCC_MAX_PCLK1_HZ) ||
        (calculated_tree.pclk2_hz > RCC_MAX_PCLK2_HZ) ||
        (calculated_tree.adc_hz > RCC_MAX_ADC_HZ))
    {
        return ERROR_STATUS_ERROR;
    }

    *clock_tree = calculated_tree;
    return ERROR_STATUS_OK;
}

/* Programs all bus prescalers while preserving unrelated CFGR fields. */
static ErrorStatus_t RCC_ConfigurePrescalers(RCC_AhbPrescaler_t ahb, RCC_ApbPrescaler_t apb1,
                                               RCC_ApbPrescaler_t apb2, RCC_AdcPrescaler_t adc)
{
    u32 ahb_encoding;
    u32 apb1_encoding;
    u32 apb2_encoding;
    u32 adc_encoding;
    u32 configuration;

    if ((RCC_EncodeAhbPrescaler(ahb, &ahb_encoding) != ERROR_STATUS_OK) ||
        (RCC_EncodeApbPrescaler(apb1, &apb1_encoding) != ERROR_STATUS_OK) ||
        (RCC_EncodeApbPrescaler(apb2, &apb2_encoding) != ERROR_STATUS_OK) ||
        (RCC_EncodeAdcPrescaler(adc, &adc_encoding) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    configuration = RCC_CFGR;
    configuration &= ~(RCC_CFGR_HPRE_MASK | RCC_CFGR_PPRE1_MASK |
                       RCC_CFGR_PPRE2_MASK | RCC_CFGR_ADCPRE_MASK);
    configuration |= (ahb_encoding << RCC_CFGR_HPRE_SHIFT);
    configuration |= (apb1_encoding << RCC_CFGR_PPRE1_SHIFT);
    configuration |= (apb2_encoding << RCC_CFGR_PPRE2_SHIFT);
    configuration |= (adc_encoding << RCC_CFGR_ADCPRE_SHIFT);
    RCC_CFGR = configuration;

    return ERROR_STATUS_OK;
}

/* Selects a system clock source and waits for the SWS confirmation. */
static ErrorStatus_t RCC_SelectSystemClockSource(RCC_SystemClockSource_t source)
{
    u32 configuration;
    u32 encoding;

    switch (source)
    {
        case RCC_SYSTEM_CLOCK_HSI: encoding = RCC_CFGR_CLOCK_SOURCE_HSI; break;
        case RCC_SYSTEM_CLOCK_HSE: encoding = RCC_CFGR_CLOCK_SOURCE_HSE; break;
        case RCC_SYSTEM_CLOCK_PLL: encoding = RCC_CFGR_CLOCK_SOURCE_PLL; break;
        default: return ERROR_STATUS_ERROR;
    }

    configuration = RCC_CFGR;
    configuration &= ~RCC_CFGR_SW_MASK;
    configuration |= encoding;
    RCC_CFGR = configuration;

    return RCC_WaitForSystemClockSource(source);
}

/* Configures and starts HSE after it is no longer the active source. */
static ErrorStatus_t RCC_StartHse(void)
{
    RCC_CR &= ~RCC_CR_HSEON_MASK;
    if (RCC_WaitForFlag(&RCC_CR, RCC_CR_HSERDY_MASK, 0U) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    if (RCC_CFG_HSE_MODE == RCC_HSE_MODE_BYPASS)
    {
        RCC_CR |= RCC_CR_HSEBYP_MASK;
    }
    else
    {
        RCC_CR &= ~RCC_CR_HSEBYP_MASK;
    }

    RCC_CR |= RCC_CR_HSEON_MASK;
    return RCC_WaitForFlag(&RCC_CR, RCC_CR_HSERDY_MASK, 1U);
}

/* Configures and starts PLL only while it is disabled. */
static ErrorStatus_t RCC_StartPll(void)
{
    u32 configuration;
    u32 multiplier_encoding;

    if ((RCC_CFG_PLL_MULTIPLIER < RCC_PLL_MULTIPLIER_2) ||
        (RCC_CFG_PLL_MULTIPLIER > RCC_PLL_MULTIPLIER_16))
    {
        return ERROR_STATUS_ERROR;
    }

    multiplier_encoding = (u32)RCC_CFG_PLL_MULTIPLIER - 2U;
    configuration = RCC_CFGR;
    configuration &= ~(RCC_CFGR_PLLSRC_MASK | RCC_CFGR_PLLXTPRE_MASK | RCC_CFGR_PLLMUL_MASK);

    if (RCC_CFG_PLL_SOURCE == RCC_PLL_SOURCE_HSE)
    {
        configuration |= RCC_CFGR_PLLSRC_MASK;
        if (RCC_CFG_PLL_HSE_DIVIDER == RCC_PLL_HSE_DIVIDER_2)
        {
            configuration |= RCC_CFGR_PLLXTPRE_MASK;
        }
    }
    else if (RCC_CFG_PLL_SOURCE != RCC_PLL_SOURCE_HSI_DIV2)
    {
        return ERROR_STATUS_ERROR;
    }

    configuration |= (multiplier_encoding << RCC_CFGR_PLLMUL_SHIFT);
    RCC_CFGR = configuration;
    RCC_CR |= RCC_CR_PLLON_MASK;

    return RCC_WaitForFlag(&RCC_CR, RCC_CR_PLLRDY_MASK, 1U);
}

/* Maps a supported peripheral to its clock-enable register and mask. */
static ErrorStatus_t RCC_GetClockControl(RCC_Peripheral_t peripheral, volatile u32 **register_address, u32 *mask)
{
    if ((register_address == NULL) || (mask == NULL))
    {
        return ERROR_STATUS_ERROR;
    }

    switch (peripheral)
    {
        case RCC_PERIPHERAL_DMA1: *register_address = &RCC_AHBENR; *mask = (1UL << 0); break;
        case RCC_PERIPHERAL_AFIO: *register_address = &RCC_APB2ENR; *mask = (1UL << 0); break;
        case RCC_PERIPHERAL_GPIOA: *register_address = &RCC_APB2ENR; *mask = (1UL << 2); break;
        case RCC_PERIPHERAL_GPIOB: *register_address = &RCC_APB2ENR; *mask = (1UL << 3); break;
        case RCC_PERIPHERAL_GPIOC: *register_address = &RCC_APB2ENR; *mask = (1UL << 4); break;
        case RCC_PERIPHERAL_GPIOD: *register_address = &RCC_APB2ENR; *mask = (1UL << 5); break;
        case RCC_PERIPHERAL_ADC1: *register_address = &RCC_APB2ENR; *mask = (1UL << 9); break;
        case RCC_PERIPHERAL_ADC2: *register_address = &RCC_APB2ENR; *mask = (1UL << 10); break;
        case RCC_PERIPHERAL_SPI1: *register_address = &RCC_APB2ENR; *mask = (1UL << 12); break;
        case RCC_PERIPHERAL_USART1: *register_address = &RCC_APB2ENR; *mask = (1UL << 14); break;
        case RCC_PERIPHERAL_USART2: *register_address = &RCC_APB1ENR; *mask = (1UL << 17); break;
        default: return ERROR_STATUS_ERROR;
    }

    return ERROR_STATUS_OK;
}

/* Maps a resettable peripheral to its reset register and mask. */
static ErrorStatus_t RCC_GetResetControl(RCC_Peripheral_t peripheral, volatile u32 **register_address, u32 *mask)
{
    if ((register_address == NULL) || (mask == NULL))
    {
        return ERROR_STATUS_ERROR;
    }

    switch (peripheral)
    {
        case RCC_PERIPHERAL_AFIO: *register_address = &RCC_APB2RSTR; *mask = (1UL << 0); break;
        case RCC_PERIPHERAL_GPIOA: *register_address = &RCC_APB2RSTR; *mask = (1UL << 2); break;
        case RCC_PERIPHERAL_GPIOB: *register_address = &RCC_APB2RSTR; *mask = (1UL << 3); break;
        case RCC_PERIPHERAL_GPIOC: *register_address = &RCC_APB2RSTR; *mask = (1UL << 4); break;
        case RCC_PERIPHERAL_GPIOD: *register_address = &RCC_APB2RSTR; *mask = (1UL << 5); break;
        case RCC_PERIPHERAL_ADC1: *register_address = &RCC_APB2RSTR; *mask = (1UL << 9); break;
        case RCC_PERIPHERAL_ADC2: *register_address = &RCC_APB2RSTR; *mask = (1UL << 10); break;
        case RCC_PERIPHERAL_SPI1: *register_address = &RCC_APB2RSTR; *mask = (1UL << 12); break;
        case RCC_PERIPHERAL_USART1: *register_address = &RCC_APB2RSTR; *mask = (1UL << 14); break;
        case RCC_PERIPHERAL_USART2: *register_address = &RCC_APB1RSTR; *mask = (1UL << 17); break;
        default: return ERROR_STATUS_ERROR;
    }

    return ERROR_STATUS_OK;
}

/* Decodes the live PLL output frequency. */
static ErrorStatus_t RCC_GetLivePllFrequency(u32 configuration, u32 *pll_frequency_hz)
{
    u32 pll_input_hz;
    u32 multiplier_encoding;
    u32 multiplier;
    u64 pll_output_hz;

    if (pll_frequency_hz == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    if ((configuration & RCC_CFGR_PLLSRC_MASK) == 0U)
    {
        pll_input_hz = RCC_HSI_FREQUENCY_HZ / 2U;
    }
    else
    {
        if (RCC_ValidateHseConfiguration() != ERROR_STATUS_OK)
        {
            return ERROR_STATUS_ERROR;
        }
        pll_input_hz = RCC_CFG_HSE_FREQUENCY_HZ;
        if ((configuration & RCC_CFGR_PLLXTPRE_MASK) != 0U)
        {
            pll_input_hz /= 2U;
        }
    }

    multiplier_encoding = (configuration & RCC_CFGR_PLLMUL_MASK) >> RCC_CFGR_PLLMUL_SHIFT;
    multiplier = (multiplier_encoding == 15U) ? 16U : (multiplier_encoding + 2U);
    pll_output_hz = (u64)pll_input_hz * (u64)multiplier;

    if ((pll_input_hz < RCC_MIN_PLL_INPUT_HZ) || (pll_input_hz > RCC_MAX_PLL_INPUT_HZ) ||
        (pll_output_hz < RCC_MIN_PLL_OUTPUT_HZ) || (pll_output_hz > RCC_MAX_PLL_OUTPUT_HZ) ||
        (pll_output_hz > RCC_MAX_SYSCLK_HZ) ||
        (((configuration & RCC_CFGR_PLLSRC_MASK) == 0U) &&
         (pll_output_hz > RCC_MAX_HSI_PLL_SYSCLK_HZ)))
    {
        return ERROR_STATUS_ERROR;
    }

    *pll_frequency_hz = (u32)pll_output_hz;
    return ERROR_STATUS_OK;
}

/* Decodes the selected live clock tree from RCC registers. */
static ErrorStatus_t RCC_GetLiveClockTree(RCC_ClockTree_t *clock_tree)
{
    RCC_ClockTree_t decoded_tree;
    u32 configuration;
    u32 source;
    u32 ahb_divider;
    u32 apb1_divider;
    u32 apb2_divider;
    u32 adc_divider;

    if (clock_tree == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    configuration = RCC_CFGR;
    source = (configuration & RCC_CFGR_SWS_MASK) >> 2U;
    if (source == RCC_CFGR_CLOCK_SOURCE_HSI)
    {
        decoded_tree.sysclk_hz = RCC_HSI_FREQUENCY_HZ;
    }
    else if (source == RCC_CFGR_CLOCK_SOURCE_HSE)
    {
        if (RCC_ValidateHseConfiguration() != ERROR_STATUS_OK)
        {
            return ERROR_STATUS_ERROR;
        }
        decoded_tree.sysclk_hz = RCC_CFG_HSE_FREQUENCY_HZ;
    }
    else if (source == RCC_CFGR_CLOCK_SOURCE_PLL)
    {
        if (RCC_GetLivePllFrequency(configuration, &decoded_tree.sysclk_hz) != ERROR_STATUS_OK)
        {
            return ERROR_STATUS_ERROR;
        }
    }
    else
    {
        return ERROR_STATUS_ERROR;
    }

    if ((RCC_DecodeAhbPrescaler((configuration & RCC_CFGR_HPRE_MASK) >> RCC_CFGR_HPRE_SHIFT,
                                &ahb_divider) != ERROR_STATUS_OK) ||
        (RCC_DecodeApbPrescaler((configuration & RCC_CFGR_PPRE1_MASK) >> RCC_CFGR_PPRE1_SHIFT,
                                &apb1_divider) != ERROR_STATUS_OK) ||
        (RCC_DecodeApbPrescaler((configuration & RCC_CFGR_PPRE2_MASK) >> RCC_CFGR_PPRE2_SHIFT,
                                &apb2_divider) != ERROR_STATUS_OK) ||
        (RCC_DecodeAdcPrescaler((configuration & RCC_CFGR_ADCPRE_MASK) >> RCC_CFGR_ADCPRE_SHIFT,
                                &adc_divider) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    decoded_tree.hclk_hz = decoded_tree.sysclk_hz / ahb_divider;
    decoded_tree.pclk1_hz = decoded_tree.hclk_hz / apb1_divider;
    decoded_tree.pclk2_hz = decoded_tree.hclk_hz / apb2_divider;
    decoded_tree.adc_hz = decoded_tree.pclk2_hz / adc_divider;

    if ((decoded_tree.hclk_hz > RCC_MAX_HCLK_HZ) ||
        (decoded_tree.pclk1_hz > RCC_MAX_PCLK1_HZ) ||
        (decoded_tree.pclk2_hz > RCC_MAX_PCLK2_HZ) ||
        (decoded_tree.adc_hz > RCC_MAX_ADC_HZ))
    {
        return ERROR_STATUS_ERROR;
    }

    *clock_tree = decoded_tree;
    return ERROR_STATUS_OK;
}

/* Applies the validated compile-time clock configuration without touching FLASH. */
ErrorStatus_t RCC_InitSystemClock(void)
{
    RCC_ClockTree_t configured_tree;
    u8 hse_required;

    if (RCC_BuildConfiguredClockTree(&configured_tree) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    (void)configured_tree;
    hse_required = (RCC_CFG_SYSTEM_CLOCK_SOURCE == RCC_SYSTEM_CLOCK_HSE) ||
                   ((RCC_CFG_SYSTEM_CLOCK_SOURCE == RCC_SYSTEM_CLOCK_PLL) &&
                    (RCC_CFG_PLL_SOURCE == RCC_PLL_SOURCE_HSE));

    RCC_CR |= RCC_CR_HSION_MASK;
    if (RCC_WaitForFlag(&RCC_CR, RCC_CR_HSIRDY_MASK, 1U) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    if (RCC_ConfigurePrescalers(RCC_AHB_PRESCALER_512, RCC_APB_PRESCALER_16,
                                RCC_APB_PRESCALER_16, RCC_ADC_PRESCALER_8) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    if (RCC_SelectSystemClockSource(RCC_SYSTEM_CLOCK_HSI) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    RCC_CR &= ~RCC_CR_PLLON_MASK;
    if (RCC_WaitForFlag(&RCC_CR, RCC_CR_PLLRDY_MASK, 0U) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    if ((hse_required != 0U) && (RCC_StartHse() != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    if ((RCC_CFG_SYSTEM_CLOCK_SOURCE == RCC_SYSTEM_CLOCK_PLL) &&
        (RCC_StartPll() != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    if (RCC_SelectSystemClockSource(RCC_CFG_SYSTEM_CLOCK_SOURCE) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    return RCC_ConfigurePrescalers(RCC_CFG_AHB_PRESCALER, RCC_CFG_APB1_PRESCALER,
                                    RCC_CFG_APB2_PRESCALER, RCC_CFG_ADC_PRESCALER);
}

/* Enables a supported project peripheral clock. */
ErrorStatus_t RCC_EnablePeripheralClock(RCC_Peripheral_t peripheral)
{
    volatile u32 *register_address;
    u32 mask;

    if (RCC_GetClockControl(peripheral, &register_address, &mask) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    *register_address |= mask;
    return ERROR_STATUS_OK;
}

/* Disables a supported project peripheral clock. */
ErrorStatus_t RCC_DisablePeripheralClock(RCC_Peripheral_t peripheral)
{
    volatile u32 *register_address;
    u32 mask;

    if (RCC_GetClockControl(peripheral, &register_address, &mask) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    *register_address &= ~mask;
    return ERROR_STATUS_OK;
}

/* Pulses reset for a supported resettable project peripheral. */
ErrorStatus_t RCC_ResetPeripheral(RCC_Peripheral_t peripheral)
{
    volatile u32 *register_address;
    u32 mask;

    if (RCC_GetResetControl(peripheral, &register_address, &mask) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    *register_address |= mask;
    *register_address &= ~mask;
    return ERROR_STATUS_OK;
}

/* Returns the requested frequency from the live RCC configuration. */
ErrorStatus_t RCC_GetClockFrequency(RCC_Clock_t clock, u32 *frequency_hz)
{
    RCC_ClockTree_t clock_tree;
    u32 frequency;

    if ((frequency_hz == NULL) || (RCC_GetLiveClockTree(&clock_tree) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    switch (clock)
    {
        case RCC_CLOCK_SYSCLK: frequency = clock_tree.sysclk_hz; break;
        case RCC_CLOCK_HCLK: frequency = clock_tree.hclk_hz; break;
        case RCC_CLOCK_PCLK1: frequency = clock_tree.pclk1_hz; break;
        case RCC_CLOCK_PCLK2: frequency = clock_tree.pclk2_hz; break;
        case RCC_CLOCK_ADC: frequency = clock_tree.adc_hz; break;
        default: return ERROR_STATUS_ERROR;
    }

    *frequency_hz = frequency;
    return ERROR_STATUS_OK;
}

/* Returns useful RCC source, readiness, and reset-cause state. */
ErrorStatus_t RCC_GetStatus(RCC_Status_t *status)
{
    RCC_Status_t current_status;
    u32 configuration;
    u32 control;
    u32 reset_status;
    u32 source;

    if (status == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    configuration = RCC_CFGR;
    source = (configuration & RCC_CFGR_SWS_MASK) >> 2U;
    if (source == RCC_CFGR_CLOCK_SOURCE_HSI)
    {
        current_status.system_clock_source = RCC_SYSTEM_CLOCK_HSI;
    }
    else if (source == RCC_CFGR_CLOCK_SOURCE_HSE)
    {
        current_status.system_clock_source = RCC_SYSTEM_CLOCK_HSE;
    }
    else if (source == RCC_CFGR_CLOCK_SOURCE_PLL)
    {
        current_status.system_clock_source = RCC_SYSTEM_CLOCK_PLL;
    }
    else
    {
        return ERROR_STATUS_ERROR;
    }

    control = RCC_CR;
    current_status.hsi_ready = (u8)((control & RCC_CR_HSIRDY_MASK) != 0U);
    current_status.hse_ready = (u8)((control & RCC_CR_HSERDY_MASK) != 0U);
    current_status.pll_ready = (u8)((control & RCC_CR_PLLRDY_MASK) != 0U);
    reset_status = RCC_CSR & RCC_CSR_RESET_FLAGS_MASK;
    current_status.reset_flags = 0U;
    if ((reset_status & (1UL << 31)) != 0U)
    {
        current_status.reset_flags |= RCC_RESET_FLAG_LOW_POWER;
    }
    if ((reset_status & (1UL << 30)) != 0U)
    {
        current_status.reset_flags |= RCC_RESET_FLAG_WINDOW_WATCHDOG;
    }
    if ((reset_status & (1UL << 29)) != 0U)
    {
        current_status.reset_flags |= RCC_RESET_FLAG_INDEPENDENT_WATCHDOG;
    }
    if ((reset_status & (1UL << 28)) != 0U)
    {
        current_status.reset_flags |= RCC_RESET_FLAG_SOFTWARE;
    }
    if ((reset_status & (1UL << 27)) != 0U)
    {
        current_status.reset_flags |= RCC_RESET_FLAG_POWER_ON;
    }
    if ((reset_status & (1UL << 26)) != 0U)
    {
        current_status.reset_flags |= RCC_RESET_FLAG_PIN;
    }

    *status = current_status;
    return ERROR_STATUS_OK;
}

/* Clears only the RCC reset-cause flags. */
void RCC_ClearResetFlags(void)
{
    RCC_CSR |= RCC_CSR_RMVF_MASK;
}
