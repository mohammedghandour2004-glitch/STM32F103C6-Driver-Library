/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< STP_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : STP_PROGRAM
 * Description : This file contains the 74HC595 serial-to-parallel interface functionality.
 */

#include "STP_interface.h"
#include "STP_private.h"
#include "STP_config.h"

/* Mapping order also defines initialization and partial-init recovery order. */
static const STP_Pin_t stpPins[STP_SIGNAL_COUNT] =
{
    { STP_LATCH_CLOCK_PORT, STP_LATCH_CLOCK_PIN },
    { STP_SHIFT_CLOCK_PORT, STP_SHIFT_CLOCK_PIN },
    { STP_DATA_PORT, STP_DATA_PIN }
};

static u8 initialized;

/* Validates all wiring and rejects duplicate pins before any DIO access. */
static ErrorStatus_t STP_ValidateConfig(void)
{
    u8 i;
    u8 j;

    if ((STP_OUTPUT_SPEED != DIO_SPEED_2MHZ) &&
        (STP_OUTPUT_SPEED != DIO_SPEED_10MHZ) &&
        (STP_OUTPUT_SPEED != DIO_SPEED_50MHZ))
    {
        return ERROR_STATUS_ERROR;
    }

    for (i = 0U; i < STP_SIGNAL_COUNT; ++i)
    {
        if (((stpPins[i].port != DIO_PORT_A) &&
             (stpPins[i].port != DIO_PORT_B) &&
             (stpPins[i].port != DIO_PORT_C) &&
             (stpPins[i].port != DIO_PORT_D)) ||
            ((unsigned int)stpPins[i].pin >= (unsigned int)DIO_PIN_COUNT))
        {
            return ERROR_STATUS_ERROR;
        }
        for (j = 0U; j < i; ++j)
        {
            if ((stpPins[i].port == stpPins[j].port) &&
                (stpPins[i].pin == stpPins[j].pin))
            {
                return ERROR_STATUS_ERROR;
            }
        }
    }
    return ERROR_STATUS_OK;
}

/* Attempts LOW on every pin successfully configured during this Init call. */
static void STP_RecoverConfiguredPins(u8 configuredCount)
{
    u8 i;

    for (i = 0U; i < configuredCount; ++i)
    {
        (void)DIO_WritePin(stpPins[i].port, stpPins[i].pin, DIO_LEVEL_LOW);
    }
}

/* Writes one internal signal and invalidates the driver on failure. */
static ErrorStatus_t STP_WriteSignal(STP_Signal_t signal, DIO_Level_t level)
{
    if (DIO_WritePin(stpPins[signal].port, stpPins[signal].pin, level) != ERROR_STATUS_OK)
    {
        initialized = 0U;
        return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Validates all pins and locks before establishing LOW GPIO idle levels. */
ErrorStatus_t STP_Init(void)
{
    u8 i;
    u8 locked;
    ErrorStatus_t lockStatus = ERROR_STATUS_OK;
    DIO_PinConfig_t config;

    initialized = 0U;
    if (STP_ValidateConfig() != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    for (i = 0U; i < STP_SIGNAL_COUNT; ++i)
    {
        locked = 0U;
        if ((DIO_IsPinLocked(stpPins[i].port, stpPins[i].pin, &locked) != ERROR_STATUS_OK) ||
            (locked != 0U))
        {
            lockStatus = ERROR_STATUS_ERROR;
        }
    }
    if (lockStatus != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    config.mode = DIO_MODE_OUTPUT_PUSH_PULL;
    config.speed = STP_OUTPUT_SPEED;
    config.initial_level = DIO_LEVEL_LOW;
    for (i = 0U; i < STP_SIGNAL_COUNT; ++i)
    {
        if (DIO_ConfigPin(stpPins[i].port, stpPins[i].pin, &config) != ERROR_STATUS_OK)
        {
            STP_RecoverConfiguredPins(i);
            return ERROR_STATUS_ERROR;
        }
    }

    initialized = 1U;
    return ERROR_STATUS_OK;
}

/* Shifts MSB first and latches only after eight complete shift pulses succeed. */
ErrorStatus_t STP_SendByte(u8 data)
{
    u8 i;
    DIO_Level_t level;

    if (initialized == 0U)
    {
        return ERROR_STATUS_ERROR;
    }

    for (i = 0U; i < 8U; ++i)
    {
        level = (((data >> (7U - i)) & 1U) != 0U) ? DIO_LEVEL_HIGH : DIO_LEVEL_LOW;
        if (STP_WriteSignal(STP_SIGNAL_DATA, level) != ERROR_STATUS_OK)
        {
            return ERROR_STATUS_ERROR;
        }
        if (STP_WriteSignal(STP_SIGNAL_SHIFT_CLOCK, DIO_LEVEL_HIGH) != ERROR_STATUS_OK)
        {
            return ERROR_STATUS_ERROR;
        }
        if (STP_WriteSignal(STP_SIGNAL_SHIFT_CLOCK, DIO_LEVEL_LOW) != ERROR_STATUS_OK)
        {
            return ERROR_STATUS_ERROR;
        }
    }

    if (STP_WriteSignal(STP_SIGNAL_LATCH_CLOCK, DIO_LEVEL_HIGH) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    if (STP_WriteSignal(STP_SIGNAL_LATCH_CLOCK, DIO_LEVEL_LOW) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}
