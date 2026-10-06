/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ADC_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : ADC_INTERFACE
 * Description : This file contains ADC1 and ADC2 analog-to-digital conversion functionality.
 */

#ifndef ADC_INTERFACE_H
#define ADC_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

typedef enum
{
    ADC_INSTANCE_1,
    ADC_INSTANCE_2
} ADC_Instance_t;

typedef enum
{
    ADC_CHANNEL_0 = 0,
    ADC_CHANNEL_1 = 1,
    ADC_CHANNEL_2 = 2,
    ADC_CHANNEL_3 = 3,
    ADC_CHANNEL_4 = 4,
    ADC_CHANNEL_5 = 5,
    ADC_CHANNEL_6 = 6,
    ADC_CHANNEL_7 = 7,
    ADC_CHANNEL_8 = 8,
    ADC_CHANNEL_9 = 9,
    ADC_CHANNEL_10 = 10,
    ADC_CHANNEL_11 = 11,
    ADC_CHANNEL_12 = 12,
    ADC_CHANNEL_13 = 13,
    ADC_CHANNEL_14 = 14,
    ADC_CHANNEL_15 = 15,
    ADC_CHANNEL_TEMPERATURE = 16,
    ADC_CHANNEL_VREFINT = 17
} ADC_Channel_t;

typedef enum
{
    ADC_SAMPLE_1_5_CYCLES,
    ADC_SAMPLE_7_5_CYCLES,
    ADC_SAMPLE_13_5_CYCLES,
    ADC_SAMPLE_28_5_CYCLES,
    ADC_SAMPLE_41_5_CYCLES,
    ADC_SAMPLE_55_5_CYCLES,
    ADC_SAMPLE_71_5_CYCLES,
    ADC_SAMPLE_239_5_CYCLES
} ADC_SampleTime_t;

typedef enum
{
    ADC_ALIGN_RIGHT,
    ADC_ALIGN_LEFT
} ADC_DataAlignment_t;

typedef enum
{
    ADC_CONVERSION_SINGLE,
    ADC_CONVERSION_CONTINUOUS
} ADC_ConversionMode_t;

typedef enum
{
    ADC_FLAG_END_OF_CONVERSION
} ADC_StatusFlag_t;

typedef enum
{
    ADC_INTERRUPT_EOC
} ADC_Interrupt_t;

typedef struct
{
    u32 adcClockHz;
    ADC_DataAlignment_t alignment;
    ADC_ConversionMode_t conversionMode;
} ADC_Config_t;

/* Caller owns clocks, analog pins, physical pin availability and serialized ADC access.
 * Supply actual ADCCLK within the datasheet's 0.6..14 MHz range.
 * Config, sequence, sampling and internal-channel changes require ADON=0.
 * Enable DMA after calibration, before Start; DMA changes reject an outstanding run.
 * After Enable, wait at least 1 us and two ADC clock cycles before calibration/start.
 * Calibrate after power-up; no API supplies delays or tracks whether calibration was done.
 * After enabling internal channels, allow 10 us sensor startup; use >=17.1 us sampling
 * for temperature and VREFINT. Caller selects cycles from the actual ADC clock.
 * ADC1 scans require local DMA enabled at Start; caller configures DMA1 channel 1.
 * ADC2 accepts one rank only. Polling accepts one rank with DMA/EOC interrupt disabled.
 * Continuous polling returns an available sample, not a lossless stream; use DMA for that.
 * Timeouts count unsuccessful polls; zero checks immediately and never cancels hardware.
 * A timed-out calibration must finish before retrying; it is not automatically aborted.
 * STRT is sticky, not BUSY: repeated starts require polling completion, or Disable/Enable
 * after IRQ/DMA acquisition. Re-enabling requires the stabilization delay again.
 */

/* Configures an already-powered-off ADC and leaves it off, with DMA/interrupts disabled. */
ErrorStatus_t ADC_Config(ADC_Instance_t instance, const ADC_Config_t *config);
/* Powers on once; an already-enabled ADC is not rewritten or retriggered. */
ErrorStatus_t ADC_Enable(ADC_Instance_t instance);
/* Explicitly stops conversion and powers down; rejects ongoing calibration. */
ErrorStatus_t ADC_Disable(ADC_Instance_t instance);
/* Resets and performs calibration after caller-provided power stabilization. */
ErrorStatus_t ADC_Calibrate(ADC_Instance_t instance, u32 timeout);
/* Sets one channel's sampling field while powered off. */
ErrorStatus_t ADC_SetChannelSampleTime(ADC_Instance_t instance, ADC_Channel_t channel,
                                      ADC_SampleTime_t sampleTime);
/* Configures ADC1 ranks 1..16 or ADC2's single rank while powered off. */
ErrorStatus_t ADC_ConfigRegularSequence(ADC_Instance_t instance,
                                       const ADC_Channel_t *channels, u8 length);
/* Starts the live validated regular sequence using SWSTART. */
ErrorStatus_t ADC_StartRegularConversion(ADC_Instance_t instance);
/* Polls one-channel data and returns 0..4095; output is unchanged on failure. */
ErrorStatus_t ADC_ReadRegularData(ADC_Instance_t instance, u16 *data, u32 timeout);
/* Returns the selected status as 0 or 1 without changing output on error. */
ErrorStatus_t ADC_GetFlag(ADC_Instance_t instance, ADC_StatusFlag_t flag, u8 *isSet);
/* Enables the local EOC interrupt only. */
ErrorStatus_t ADC_EnableInterrupt(ADC_Instance_t instance, ADC_Interrupt_t interrupt);
/* Disables the local EOC interrupt only. */
ErrorStatus_t ADC_DisableInterrupt(ADC_Instance_t instance, ADC_Interrupt_t interrupt);
/* Enables ADC1's local DMA request while not converting/calibrating; ADC2 is rejected. */
ErrorStatus_t ADC_EnableDMARequest(ADC_Instance_t instance);
/* Disables ADC1's local DMA request while not converting/calibrating; ADC2 is rejected. */
ErrorStatus_t ADC_DisableDMARequest(ADC_Instance_t instance);
/* Enables ADC1's shared temperature/VREFINT circuitry while powered off. */
ErrorStatus_t ADC_EnableInternalChannels(ADC_Instance_t instance);
/* Disables ADC1's shared temperature/VREFINT circuitry while powered off. */
ErrorStatus_t ADC_DisableInternalChannels(ADC_Instance_t instance);

#endif
