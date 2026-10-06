/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< DAC_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : DAC_PROGRAM
 * Description : This file contains the external 8-bit R-2R DAC control functionality.
 */

#include "DAC_interface.h"
#include "DAC_private.h"
#include "DAC_config.h"

static const DAC_Pin_t dacPins[8] =
{
    { DAC_D0_PORT, DAC_D0_PIN },
    { DAC_D1_PORT, DAC_D1_PIN },
    { DAC_D2_PORT, DAC_D2_PIN },
    { DAC_D3_PORT, DAC_D3_PIN },
    { DAC_D4_PORT, DAC_D4_PIN },
    { DAC_D5_PORT, DAC_D5_PIN },
    { DAC_D6_PORT, DAC_D6_PIN },
    { DAC_D7_PORT, DAC_D7_PIN }
};

static u8 initialized;

/* Validates every connection and rejects shared pins before any DIO access. */
static ErrorStatus_t DAC_ValidateConfig(void)
{
    u8 i;
    u8 j;

    if ((DAC_OUTPUT_SPEED != DIO_SPEED_2MHZ) &&
        (DAC_OUTPUT_SPEED != DIO_SPEED_10MHZ) &&
        (DAC_OUTPUT_SPEED != DIO_SPEED_50MHZ))
    {
        return ERROR_STATUS_ERROR;
    }

    for (i = 0U; i < 8U; ++i)
    {
        if (((dacPins[i].port != DIO_PORT_A) &&
             (dacPins[i].port != DIO_PORT_B) &&
             (dacPins[i].port != DIO_PORT_C) &&
             (dacPins[i].port != DIO_PORT_D)) ||
            ((unsigned int)dacPins[i].pin >= (unsigned int)DIO_PIN_COUNT))
        {
            return ERROR_STATUS_ERROR;
        }
        for (j = 0U; j < i; ++j)
        {
            if ((dacPins[i].port == dacPins[j].port) &&
                (dacPins[i].pin == dacPins[j].pin))
            {
                return ERROR_STATUS_ERROR;
            }
        }
    }
    return ERROR_STATUS_OK;
}

/* Attempts LOW only on outputs successfully configured during this Init call. */
static void DAC_RecoverConfiguredPins(u8 configuredCount)
{
    u8 i;

    for (i = 0U; i < configuredCount; ++i)
    {
        (void)DIO_WritePin(dacPins[i].port, dacPins[i].pin, DIO_LEVEL_LOW);
    }
}

/* Validates wiring and all locks before configuring D0 through D7 LOW. */
ErrorStatus_t DAC_Init(void)
{
    u8 i;
    u8 locked;
    ErrorStatus_t lockStatus = ERROR_STATUS_OK;
    DIO_PinConfig_t config;

    initialized = 0U;
    if (DAC_ValidateConfig() != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    for (i = 0U; i < 8U; ++i)
    {
        locked = 0U;
        if ((DIO_IsPinLocked(dacPins[i].port, dacPins[i].pin, &locked) != ERROR_STATUS_OK) ||
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
    config.speed = DAC_OUTPUT_SPEED;
    config.initial_level = DIO_LEVEL_LOW;
    for (i = 0U; i < 8U; ++i)
    {
        if (DIO_ConfigPin(dacPins[i].port, dacPins[i].pin, &config) != ERROR_STATUS_OK)
        {
            DAC_RecoverConfiguredPins(i);
            return ERROR_STATUS_ERROR;
        }
    }

    initialized = 1U;
    return ERROR_STATUS_OK;
}

/* Writes all eight sample bits sequentially and invalidates state on failure. */
ErrorStatus_t DAC_Write(u8 value)
{
    u8 i;
    DIO_Level_t level;

    if (initialized == 0U)
    {
        return ERROR_STATUS_ERROR;
    }

    for (i = 0U; i < 8U; ++i)
    {
        level = (((value >> i) & 1U) != 0U) ? DIO_LEVEL_HIGH : DIO_LEVEL_LOW;
        if (DIO_WritePin(dacPins[i].port, dacPins[i].pin, level) != ERROR_STATUS_OK)
        {
            initialized = 0U;
            return ERROR_STATUS_ERROR;
        }
    }
    return ERROR_STATUS_OK;
}
