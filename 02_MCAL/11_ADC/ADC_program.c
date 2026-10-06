/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ADC_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : ADC_PROGRAM
 * Description : This file contains ADC1 and ADC2 analog-to-digital conversion functionality.
 */

#include "ADC_interface.h"
#include "ADC_private.h"
#include "ADC_config.h"

/* Resolves an implemented instance without MMIO. */
static ErrorStatus_t ADC_Resolve(ADC_Instance_t instance, u32 *base)
{
    switch (instance)
    {
        case ADC_INSTANCE_1: *base = ADC1_BASE; break;
        case ADC_INSTANCE_2: *base = ADC2_BASE; break;
        default: return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Validates the register-level channel and internal-channel ownership. */
static ErrorStatus_t ADC_CheckChannel(ADC_Instance_t instance, ADC_Channel_t channel)
{
    if (((u32)channel > (u32)ADC_CHANNEL_VREFINT)
        || ((instance == ADC_INSTANCE_2) && ((u32)channel > 15U)))
    {
        return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Rejects shared dual mode rather than changing the other ADC's operation. */
static ErrorStatus_t ADC_CheckIndependent(void)
{
    return ((ADC_REG(ADC1_BASE, ADC_CR1_OFFSET) & ADC_DUAL_MASK) == 0U)
        ? ERROR_STATUS_OK : ERROR_STATUS_ERROR;
}

/* Requires power-down for changes that could disturb conversion or ADON triggering. */
static ErrorStatus_t ADC_CheckOff(u32 base)
{
    if ((ADC_CheckIndependent() != ERROR_STATUS_OK)
        || ((ADC_REG(base, ADC_CR2_OFFSET) & (ADC_ADON | ADC_CALIBRATION)) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Decodes the regular sequence length from hardware. */
static u32 ADC_Length(u32 base)
{
    return ((ADC_REG(base, ADC_SQR1_OFFSET) & ADC_LENGTH_MASK) >> ADC_LENGTH_SHIFT) + 1U;
}

/* Checks that live settings remain within the independent regular software-trigger model. */
static ErrorStatus_t ADC_CheckLive(u32 base)
{
    u32 cr1 = ADC_REG(base, ADC_CR1_OFFSET);
    u32 cr2 = ADC_REG(base, ADC_CR2_OFFSET);
    if ((ADC_CheckIndependent() != ERROR_STATUS_OK)
        || ((cr1 & ADC_ADVANCED) != 0U)
        || ((cr2 & (ADC_ADON | ADC_CALIBRATION | ADC_JEXTTRIG | ADC_JSWSTART))
            != ADC_ADON)
        || ((cr2 & (ADC_EXTSEL_MASK | ADC_EXTTRIG)) != (ADC_EXTSEL_MASK | ADC_EXTTRIG)))
    {
        return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Waits for calibration control bits to clear using a shared bounded budget. */
static ErrorStatus_t ADC_WaitClear(u32 base, u32 mask, u32 *budget)
{
    for (;;)
    {
        if ((ADC_REG(base, ADC_CR2_OFFSET) & mask) == 0U)
        {
            return ERROR_STATUS_OK;
        }
        if (*budget == 0U) { return ERROR_STATUS_ERROR; }
        --(*budget);
    }
}

/* Configures only an ADC that is already powered off. */
ErrorStatus_t ADC_Config(ADC_Instance_t instance, const ADC_Config_t *config)
{
    u32 base;
    u32 cr2 = ADC_EXTSEL_MASK | ADC_EXTTRIG;
    u32 length;
    if ((ADC_Resolve(instance, &base) != ERROR_STATUS_OK) || (config == NULL))
    {
        return ERROR_STATUS_ERROR;
    }
    if ((config->adcClockHz < ADC_MIN_CLOCK) || (config->adcClockHz > ADC_MAX_CLOCK))
    {
        return ERROR_STATUS_ERROR;
    }
    switch (config->alignment)
    {
        case ADC_ALIGN_RIGHT: break;
        case ADC_ALIGN_LEFT: cr2 |= ADC_ALIGN; break;
        default: return ERROR_STATUS_ERROR;
    }
    switch (config->conversionMode)
    {
        case ADC_CONVERSION_SINGLE: break;
        case ADC_CONVERSION_CONTINUOUS: cr2 |= ADC_CONT; break;
        default: return ERROR_STATUS_ERROR;
    }
    if (ADC_CheckOff(base) != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    length = ADC_Length(base);
    if ((instance == ADC_INSTANCE_2) && (length != 1U)) { return ERROR_STATUS_ERROR; }
    ADC_REG(base, ADC_CR1_OFFSET) =
        (ADC_REG(base, ADC_CR1_OFFSET) & ~(ADC_ADVANCED | ADC_EOCIE | ADC_SCAN))
        | ((length > 1U) ? ADC_SCAN : 0U);
    ADC_REG(base, ADC_CR2_OFFSET) =
        (ADC_REG(base, ADC_CR2_OFFSET) & ~ADC_CR2_OWNED) | cr2;
    ADC_REG(base, ADC_SR_OFFSET) = ADC_SR_VALID & ~(ADC_EOC | ADC_STRT);
    return ERROR_STATUS_OK;
}

/* Powers on without performing the legacy second ADON write. */
ErrorStatus_t ADC_Enable(ADC_Instance_t instance)
{
    u32 base;
    u32 cr2;
    if (ADC_Resolve(instance, &base) != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    if (ADC_CheckIndependent() != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    cr2 = ADC_REG(base, ADC_CR2_OFFSET);
    if ((cr2 & ADC_ADON) != 0U) { return ERROR_STATUS_OK; }
    if ((cr2 & (ADC_CALIBRATION | ADC_SWSTART | ADC_JSWSTART)) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    ADC_REG(base, ADC_CR2_OFFSET) = cr2 | ADC_ADON;
    return ERROR_STATUS_OK;
}

/* Explicitly stops conversion using documented ADON power-down behavior. */
ErrorStatus_t ADC_Disable(ADC_Instance_t instance)
{
    u32 base;
    u32 cr2;
    if (ADC_Resolve(instance, &base) != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    if (ADC_CheckIndependent() != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    cr2 = ADC_REG(base, ADC_CR2_OFFSET);
    if ((cr2 & ADC_CALIBRATION) != 0U) { return ERROR_STATUS_ERROR; }
    if ((cr2 & ADC_ADON) != 0U)
    {
        ADC_REG(base, ADC_CR2_OFFSET) = cr2 & ~(ADC_ADON | ADC_SWSTART);
    }
    ADC_REG(base, ADC_SR_OFFSET) = ADC_SR_VALID & ~ADC_STRT;
    return ERROR_STATUS_OK;
}

/* Performs reset-calibration then calibration after caller-provided stabilization. */
ErrorStatus_t ADC_Calibrate(ADC_Instance_t instance, u32 timeout)
{
    u32 base;
    u32 cr2;
    if (ADC_Resolve(instance, &base) != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    if (ADC_CheckLive(base) != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    cr2 = ADC_REG(base, ADC_CR2_OFFSET);
    if (((cr2 & (ADC_SWSTART | ADC_DMA)) != 0U)
        || ((ADC_REG(base, ADC_CR1_OFFSET) & ADC_EOCIE) != 0U)
        || ((ADC_REG(base, ADC_SR_OFFSET) & (ADC_STRT | ADC_EOC)) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    ADC_REG(base, ADC_CR2_OFFSET) = cr2 | ADC_RSTCAL;
    if (ADC_WaitClear(base, ADC_RSTCAL, &timeout) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    ADC_REG(base, ADC_CR2_OFFSET) = ADC_REG(base, ADC_CR2_OFFSET) | ADC_CAL;
    return ADC_WaitClear(base, ADC_CAL, &timeout);
}

/* Sets only the requested sampling-time field while the ADC is off. */
ErrorStatus_t ADC_SetChannelSampleTime(ADC_Instance_t instance, ADC_Channel_t channel,
                                      ADC_SampleTime_t sampleTime)
{
    u32 base;
    u32 encoding;
    u32 offset;
    u32 shift;
    if ((ADC_Resolve(instance, &base) != ERROR_STATUS_OK)
        || (ADC_CheckChannel(instance, channel) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    switch (sampleTime)
    {
        case ADC_SAMPLE_1_5_CYCLES: encoding = 0U; break;
        case ADC_SAMPLE_7_5_CYCLES: encoding = 1U; break;
        case ADC_SAMPLE_13_5_CYCLES: encoding = 2U; break;
        case ADC_SAMPLE_28_5_CYCLES: encoding = 3U; break;
        case ADC_SAMPLE_41_5_CYCLES: encoding = 4U; break;
        case ADC_SAMPLE_55_5_CYCLES: encoding = 5U; break;
        case ADC_SAMPLE_71_5_CYCLES: encoding = 6U; break;
        case ADC_SAMPLE_239_5_CYCLES: encoding = 7U; break;
        default: return ERROR_STATUS_ERROR;
    }
    if (ADC_CheckOff(base) != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    offset = ((u32)channel < 10U) ? ADC_SMPR2_OFFSET : ADC_SMPR1_OFFSET;
    shift = (((u32)channel < 10U) ? (u32)channel : ((u32)channel - 10U)) * 3U;
    ADC_REG(base, offset) = (ADC_REG(base, offset) & ~(UINT32_C(7) << shift))
                         | (encoding << shift);
    return ERROR_STATUS_OK;
}

/* Encodes all ranks before touching SQR registers and sets SCAN deterministically. */
ErrorStatus_t ADC_ConfigRegularSequence(ADC_Instance_t instance,
                                       const ADC_Channel_t *channels, u8 length)
{
    u32 base;
    u32 sqr1 = 0U;
    u32 sqr2 = 0U;
    u32 sqr3 = 0U;
    u32 i;
    if ((ADC_Resolve(instance, &base) != ERROR_STATUS_OK) || (channels == NULL)
        || (length == 0U) || (length > ADC_MAX_RANKS)
        || ((instance == ADC_INSTANCE_2) && (length != 1U)))
    {
        return ERROR_STATUS_ERROR;
    }
    for (i = 0U; i < length; ++i)
    {
        u32 channel;
        if (ADC_CheckChannel(instance, channels[i]) != ERROR_STATUS_OK)
        {
            return ERROR_STATUS_ERROR;
        }
        channel = (u32)channels[i];
        if (i < 6U) { sqr3 |= channel << (i * 5U); }
        else if (i < 12U) { sqr2 |= channel << ((i - 6U) * 5U); }
        else { sqr1 |= channel << ((i - 12U) * 5U); }
    }
    sqr1 |= ((u32)length - 1U) << ADC_LENGTH_SHIFT;
    if (ADC_CheckOff(base) != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    ADC_REG(base, ADC_SQR3_OFFSET) = (ADC_REG(base, ADC_SQR3_OFFSET) & ~ADC_SQR23_MASK) | sqr3;
    ADC_REG(base, ADC_SQR2_OFFSET) = (ADC_REG(base, ADC_SQR2_OFFSET) & ~ADC_SQR23_MASK) | sqr2;
    ADC_REG(base, ADC_SQR1_OFFSET) = (ADC_REG(base, ADC_SQR1_OFFSET) & ~ADC_SQR1_MASK) | sqr1;
    ADC_REG(base, ADC_CR1_OFFSET) = (ADC_REG(base, ADC_CR1_OFFSET) & ~ADC_SCAN)
                                | ((length > 1U) ? ADC_SCAN : 0U);
    return ERROR_STATUS_OK;
}

/* Validates every live rank and requires enabled internal circuitry where used. */
static ErrorStatus_t ADC_CheckSequence(ADC_Instance_t instance, u32 base, u32 length, u32 cr2)
{
    u32 i;
    for (i = 0U; i < length; ++i)
    {
        u32 offset = (i < 6U) ? ADC_SQR3_OFFSET : ((i < 12U) ? ADC_SQR2_OFFSET : ADC_SQR1_OFFSET);
        u32 shift = ((i < 6U) ? i : ((i < 12U) ? i - 6U : i - 12U)) * 5U;
        u32 channel = (ADC_REG(base, offset) >> shift) & ADC_CHANNEL_MASK;
        if ((ADC_CheckChannel(instance, (ADC_Channel_t)channel) != ERROR_STATUS_OK)
            || ((channel >= 16U) && ((cr2 & ADC_TSVREFE) == 0U)))
        {
            return ERROR_STATUS_ERROR;
        }
    }
    return ERROR_STATUS_OK;
}

/* Starts a validated software-trigger sequence without retriggering an existing run. */
ErrorStatus_t ADC_StartRegularConversion(ADC_Instance_t instance)
{
    u32 base;
    u32 cr1;
    u32 cr2;
    u32 length;
    if (ADC_Resolve(instance, &base) != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    if (ADC_CheckLive(base) != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    cr1 = ADC_REG(base, ADC_CR1_OFFSET);
    cr2 = ADC_REG(base, ADC_CR2_OFFSET);
    length = ADC_Length(base);
    if (((instance == ADC_INSTANCE_2) && ((length != 1U) || ((cr2 & ADC_DMA) != 0U)))
        || ((length > 1U) && ((cr2 & ADC_DMA) == 0U))
        || (((cr1 & ADC_SCAN) != 0U) != (length > 1U))
        || ((cr2 & ADC_SWSTART) != 0U)
        || ((ADC_REG(base, ADC_SR_OFFSET) & (ADC_EOC | ADC_STRT)) != 0U)
        || (ADC_CheckSequence(instance, base, length, cr2) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    ADC_REG(base, ADC_CR2_OFFSET) = cr2 | ADC_SWSTART;
    return ERROR_STATUS_OK;
}

/* Reads only single-channel polling results and normalizes the alignment. */
ErrorStatus_t ADC_ReadRegularData(ADC_Instance_t instance, u16 *data, u32 timeout)
{
    u32 base;
    u32 cr2;
    u32 result;
    if ((data == NULL) || (ADC_Resolve(instance, &base) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    if (ADC_CheckLive(base) != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    cr2 = ADC_REG(base, ADC_CR2_OFFSET);
    if ((ADC_Length(base) != 1U) || ((cr2 & ADC_DMA) != 0U)
        || ((ADC_REG(base, ADC_CR1_OFFSET) & (ADC_EOCIE | ADC_SCAN)) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    for (;;)
    {
        if ((ADC_REG(base, ADC_SR_OFFSET) & ADC_EOC) != 0U) { break; }
        if (timeout == 0U) { return ERROR_STATUS_ERROR; }
        --timeout;
    }
    result = ADC_REG(base, ADC_DR_OFFSET);
    if ((cr2 & ADC_ALIGN) != 0U) { result >>= 4U; }
    if ((cr2 & ADC_CONT) == 0U)
    {
        /* Single-channel completion proves it is safe to retire the sticky start flag. */
        ADC_REG(base, ADC_SR_OFFSET) = ADC_SR_VALID & ~ADC_STRT;
    }
    *data = (u16)(result & ADC_RESULT_MASK);
    return ERROR_STATUS_OK;
}

/* Reports EOC without reading or consuming the data register. */
ErrorStatus_t ADC_GetFlag(ADC_Instance_t instance, ADC_StatusFlag_t flag, u8 *isSet)
{
    u32 base;
    if ((isSet == NULL) || (flag != ADC_FLAG_END_OF_CONVERSION)
        || (ADC_Resolve(instance, &base) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    *isSet = ((ADC_REG(base, ADC_SR_OFFSET) & ADC_EOC) != 0U) ? 1U : 0U;
    return ERROR_STATUS_OK;
}

/* Enables only the implemented EOC interrupt source. */
ErrorStatus_t ADC_EnableInterrupt(ADC_Instance_t instance, ADC_Interrupt_t interrupt)
{
    u32 base;
    if ((interrupt != ADC_INTERRUPT_EOC) || (ADC_Resolve(instance, &base) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    ADC_REG(base, ADC_CR1_OFFSET) |= ADC_EOCIE;
    return ERROR_STATUS_OK;
}

/* Enables ADC1 DMA before starting, avoiding an unchanged ADON rewrite. */
ErrorStatus_t ADC_EnableDMARequest(ADC_Instance_t instance)
{
    u32 base;
    if ((instance != ADC_INSTANCE_1) || (ADC_Resolve(instance, &base) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    if ((ADC_CheckIndependent() != ERROR_STATUS_OK)
        || ((ADC_REG(base, ADC_CR2_OFFSET) & (ADC_CALIBRATION | ADC_SWSTART | ADC_JSWSTART)) != 0U)
        || ((ADC_REG(base, ADC_SR_OFFSET) & ADC_STRT) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    if ((ADC_REG(base, ADC_CR2_OFFSET) & ADC_DMA) == 0U)
    {
        ADC_REG(base, ADC_CR2_OFFSET) |= ADC_DMA;
    }
    return ERROR_STATUS_OK;
}

/* Enables ADC1's internal channels only while powered off. */
ErrorStatus_t ADC_EnableInternalChannels(ADC_Instance_t instance)
{
    u32 base;
    if ((instance != ADC_INSTANCE_1) || (ADC_Resolve(instance, &base) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    if (ADC_CheckOff(base) != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    ADC_REG(base, ADC_CR2_OFFSET) |= ADC_TSVREFE;
    return ERROR_STATUS_OK;
}

/* Disables only the implemented EOC interrupt source. */
ErrorStatus_t ADC_DisableInterrupt(ADC_Instance_t instance, ADC_Interrupt_t interrupt)
{
    u32 base;
    if ((interrupt != ADC_INTERRUPT_EOC) || (ADC_Resolve(instance, &base) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    ADC_REG(base, ADC_CR1_OFFSET) &= ~ADC_EOCIE;
    return ERROR_STATUS_OK;
}

/* Disables ADC1 DMA after stopping, avoiding an unchanged ADON rewrite. */
ErrorStatus_t ADC_DisableDMARequest(ADC_Instance_t instance)
{
    u32 base;
    if ((instance != ADC_INSTANCE_1) || (ADC_Resolve(instance, &base) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    if ((ADC_CheckIndependent() != ERROR_STATUS_OK)
        || ((ADC_REG(base, ADC_CR2_OFFSET) & (ADC_CALIBRATION | ADC_SWSTART | ADC_JSWSTART)) != 0U)
        || ((ADC_REG(base, ADC_SR_OFFSET) & ADC_STRT) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    if ((ADC_REG(base, ADC_CR2_OFFSET) & ADC_DMA) != 0U)
    {
        ADC_REG(base, ADC_CR2_OFFSET) &= ~ADC_DMA;
    }
    return ERROR_STATUS_OK;
}

/* Disables ADC1's internal channels only while powered off. */
ErrorStatus_t ADC_DisableInternalChannels(ADC_Instance_t instance)
{
    u32 base;
    if ((instance != ADC_INSTANCE_1) || (ADC_Resolve(instance, &base) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    if (ADC_CheckOff(base) != ERROR_STATUS_OK) { return ERROR_STATUS_ERROR; }
    ADC_REG(base, ADC_CR2_OFFSET) &= ~ADC_TSVREFE;
    return ERROR_STATUS_OK;
}
