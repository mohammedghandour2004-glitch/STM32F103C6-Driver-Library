/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< HLEDMAT_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : HLEDMAT_PROGRAM
 * Description : This file contains the 8x8 monochrome LED matrix control functionality.
 */

#include "HLEDMAT_interface.h"
#include "HLEDMAT_private.h"
#include "HLEDMAT_config.h"

static const HLEDMAT_Pin_t rowPins[8] =
{
    { HLEDMAT_ROW_0_PORT, HLEDMAT_ROW_0_PIN },
    { HLEDMAT_ROW_1_PORT, HLEDMAT_ROW_1_PIN },
    { HLEDMAT_ROW_2_PORT, HLEDMAT_ROW_2_PIN },
    { HLEDMAT_ROW_3_PORT, HLEDMAT_ROW_3_PIN },
    { HLEDMAT_ROW_4_PORT, HLEDMAT_ROW_4_PIN },
    { HLEDMAT_ROW_5_PORT, HLEDMAT_ROW_5_PIN },
    { HLEDMAT_ROW_6_PORT, HLEDMAT_ROW_6_PIN },
    { HLEDMAT_ROW_7_PORT, HLEDMAT_ROW_7_PIN }
};

static const HLEDMAT_Pin_t columnPins[8] =
{
    { HLEDMAT_COLUMN_0_PORT, HLEDMAT_COLUMN_0_PIN },
    { HLEDMAT_COLUMN_1_PORT, HLEDMAT_COLUMN_1_PIN },
    { HLEDMAT_COLUMN_2_PORT, HLEDMAT_COLUMN_2_PIN },
    { HLEDMAT_COLUMN_3_PORT, HLEDMAT_COLUMN_3_PIN },
    { HLEDMAT_COLUMN_4_PORT, HLEDMAT_COLUMN_4_PIN },
    { HLEDMAT_COLUMN_5_PORT, HLEDMAT_COLUMN_5_PIN },
    { HLEDMAT_COLUMN_6_PORT, HLEDMAT_COLUMN_6_PIN },
    { HLEDMAT_COLUMN_7_PORT, HLEDMAT_COLUMN_7_PIN }
};

static u8 frameBuffer[8];
static u8 nextRow;
static u8 initialized;

/* Returns the opposite of a validated electrical level. */
static DIO_Level_t HLEDMAT_Opposite(DIO_Level_t level)
{
    return (level == DIO_LEVEL_HIGH) ? DIO_LEVEL_LOW : DIO_LEVEL_HIGH;
}

/* Indexes the sixteen connections without duplicating their storage. */
static const HLEDMAT_Pin_t *HLEDMAT_GetPin(u8 index)
{
    return (index < 8U) ? &rowPins[index] : &columnPins[index - 8U];
}

/* Validates all configuration and rejects shared GPIO connections. */
static ErrorStatus_t HLEDMAT_ValidateConfig(void)
{
    u8 i;
    u8 j;
    const HLEDMAT_Pin_t *pin;
    const HLEDMAT_Pin_t *previous;

    if (((HLEDMAT_ROW_ACTIVE_LEVEL != DIO_LEVEL_LOW) &&
         (HLEDMAT_ROW_ACTIVE_LEVEL != DIO_LEVEL_HIGH)) ||
        ((HLEDMAT_COLUMN_ON_LEVEL != DIO_LEVEL_LOW) &&
         (HLEDMAT_COLUMN_ON_LEVEL != DIO_LEVEL_HIGH)) ||
        ((HLEDMAT_OUTPUT_SPEED != DIO_SPEED_2MHZ) &&
         (HLEDMAT_OUTPUT_SPEED != DIO_SPEED_10MHZ) &&
         (HLEDMAT_OUTPUT_SPEED != DIO_SPEED_50MHZ)))
    {
        return ERROR_STATUS_ERROR;
    }

    for (i = 0U; i < 16U; ++i)
    {
        pin = HLEDMAT_GetPin(i);
        if (((pin->port != DIO_PORT_A) && (pin->port != DIO_PORT_B) &&
             (pin->port != DIO_PORT_C) && (pin->port != DIO_PORT_D)) ||
            ((unsigned int)pin->pin >= (unsigned int)DIO_PIN_COUNT))
        {
            return ERROR_STATUS_ERROR;
        }

        for (j = 0U; j < i; ++j)
        {
            previous = HLEDMAT_GetPin(j);
            if ((pin->port == previous->port) && (pin->pin == previous->pin))
            {
                return ERROR_STATUS_ERROR;
            }
        }
    }
    return ERROR_STATUS_OK;
}

/* Attempts every row even when an earlier blanking write fails. */
static ErrorStatus_t HLEDMAT_BlankRows(void)
{
    u8 i;
    ErrorStatus_t status = ERROR_STATUS_OK;

    for (i = 0U; i < 8U; ++i)
    {
        if (DIO_WritePin(rowPins[i].port, rowPins[i].pin,
                         HLEDMAT_Opposite(HLEDMAT_ROW_ACTIVE_LEVEL)) != ERROR_STATUS_OK)
        {
            status = ERROR_STATUS_ERROR;
        }
    }
    return status;
}

/* Invalidates operation and attempts blanking without claiming hardware safety. */
static ErrorStatus_t HLEDMAT_Fail(void)
{
    initialized = 0U;
    (void)HLEDMAT_BlankRows();
    return ERROR_STATUS_ERROR;
}

/* Validates wiring and locks before configuring inactive matrix outputs. */
ErrorStatus_t HLEDMAT_Init(void)
{
    u8 i;
    u8 locked;
    ErrorStatus_t lockStatus = ERROR_STATUS_OK;
    const HLEDMAT_Pin_t *pin;
    DIO_PinConfig_t config;

    initialized = 0U;
    if (HLEDMAT_ValidateConfig() != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    for (i = 0U; i < 16U; ++i)
    {
        pin = HLEDMAT_GetPin(i);
        locked = 0U;
        if ((DIO_IsPinLocked(pin->port, pin->pin, &locked) != ERROR_STATUS_OK) ||
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
    config.speed = HLEDMAT_OUTPUT_SPEED;
    config.initial_level = HLEDMAT_Opposite(HLEDMAT_ROW_ACTIVE_LEVEL);
    for (i = 0U; i < 8U; ++i)
    {
        if (DIO_ConfigPin(rowPins[i].port, rowPins[i].pin, &config) != ERROR_STATUS_OK)
        {
            return HLEDMAT_Fail();
        }
    }

    config.initial_level = HLEDMAT_Opposite(HLEDMAT_COLUMN_ON_LEVEL);
    for (i = 0U; i < 8U; ++i)
    {
        if (DIO_ConfigPin(columnPins[i].port, columnPins[i].pin, &config) != ERROR_STATUS_OK)
        {
            return HLEDMAT_Fail();
        }
    }

    for (i = 0U; i < 8U; ++i)
    {
        frameBuffer[i] = 0U;
    }
    nextRow = 0U;
    initialized = 1U;
    return ERROR_STATUS_OK;
}

/* Copies one logical frame without retaining the caller's pointer. */
ErrorStatus_t HLEDMAT_SetFrame(const u8 frame[8])
{
    u8 i;

    if ((initialized == 0U) || (frame == NULL))
    {
        return ERROR_STATUS_ERROR;
    }
    for (i = 0U; i < 8U; ++i)
    {
        frameBuffer[i] = frame[i];
    }
    return ERROR_STATUS_OK;
}

/* Blanks, programs columns, and selects one row without a timing wait. */
ErrorStatus_t HLEDMAT_Refresh(void)
{
    u8 column;
    u8 rowData;
    DIO_Level_t level;

    if (initialized == 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    if (HLEDMAT_BlankRows() != ERROR_STATUS_OK)
    {
        return HLEDMAT_Fail();
    }

    rowData = frameBuffer[nextRow];
    for (column = 0U; column < 8U; ++column)
    {
        level = (((rowData >> column) & 1U) != 0U) ?
                HLEDMAT_COLUMN_ON_LEVEL : HLEDMAT_Opposite(HLEDMAT_COLUMN_ON_LEVEL);
        if (DIO_WritePin(columnPins[column].port, columnPins[column].pin, level) != ERROR_STATUS_OK)
        {
            return HLEDMAT_Fail();
        }
    }

    if ((rowData != 0U) &&
        (DIO_WritePin(rowPins[nextRow].port, rowPins[nextRow].pin,
                      HLEDMAT_ROW_ACTIVE_LEVEL) != ERROR_STATUS_OK))
    {
        return HLEDMAT_Fail();
    }

    nextRow = (u8)((nextRow + 1U) % 8U);
    return ERROR_STATUS_OK;
}
