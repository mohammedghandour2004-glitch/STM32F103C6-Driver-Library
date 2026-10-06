/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< TFT_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : TFT_PROGRAM
 * Description : This file contains the ST7735S TFT display control functionality.
 */

#include "TFT_interface.h"
#include "TFT_private.h"
#include "TFT_config.h"

static const TFT_Pin_t tftPins[TFT_SIGNAL_COUNT] =
{
    { TFT_CS_PORT, TFT_CS_PIN },
    { TFT_DC_PORT, TFT_DC_PIN },
    { TFT_RST_PORT, TFT_RST_PIN }
};

static const DIO_Level_t idleLevels[TFT_SIGNAL_COUNT] =
{
    DIO_LEVEL_HIGH, DIO_LEVEL_LOW, DIO_LEVEL_HIGH
};

static const TFT_InitEntry_t initTable[] =
{
    { TFT_CMD_FRMCTR1, 3U, { 0x01U, 0x2CU, 0x2DU } },
    { TFT_CMD_FRMCTR2, 3U, { 0x01U, 0x2CU, 0x2DU } },
    { TFT_CMD_FRMCTR3, 6U, { 0x01U, 0x2CU, 0x2DU, 0x01U, 0x2CU, 0x2DU } },
    { TFT_CMD_INVCTR, 1U, { 0x07U } },
    { TFT_CMD_PWCTR1, 3U, { 0xA2U, 0x02U, 0x84U } },
    { TFT_CMD_PWCTR2, 1U, { 0xC5U } },
    { TFT_CMD_PWCTR3, 2U, { 0x0AU, 0x00U } },
    { TFT_CMD_PWCTR4, 2U, { 0x8AU, 0x2AU } },
    { TFT_CMD_PWCTR5, 2U, { 0x8AU, 0xEEU } },
    { TFT_CMD_VMCTR1, 1U, { 0x0EU } },
    { TFT_CMD_GAMCTRP1, 16U,
      { 0x0FU, 0x1AU, 0x0FU, 0x18U, 0x2FU, 0x28U, 0x20U, 0x22U,
        0x1FU, 0x1BU, 0x23U, 0x37U, 0x00U, 0x07U, 0x02U, 0x10U } },
    { TFT_CMD_GAMCTRN1, 16U,
      { 0x0FU, 0x1BU, 0x0FU, 0x17U, 0x33U, 0x2CU, 0x29U, 0x2EU,
        0x30U, 0x30U, 0x39U, 0x3FU, 0x00U, 0x07U, 0x03U, 0x10U } },
    /* Opaque settings retained from the approved module sequence. */
    { TFT_CMD_MODULE_F0, 1U, { 0x01U } },
    { TFT_CMD_MODULE_F6, 1U, { 0x00U } },
    { TFT_CMD_COLMOD, 1U, { TFT_RGB565_COLMOD } },
    { TFT_CMD_MADCTL, 1U, { TFT_PORTRAIT_MADCTL } }
};

static TFT_InitState_t initState = TFT_INIT_STATE_IDLE;
static size_t tableIndex;
static u32 elapsedWait;
static u8 configuredCount;

/* Rejects invalid configuration before any DIO or SPI access. */
static ErrorStatus_t TFT_ValidateConfig(void)
{
    u8 i;
    u8 j;

    if ((TFT_SPI_INSTANCE != SPI_INSTANCE_1) ||
        ((TFT_OUTPUT_SPEED != DIO_SPEED_2MHZ) &&
         (TFT_OUTPUT_SPEED != DIO_SPEED_10MHZ) &&
         (TFT_OUTPUT_SPEED != DIO_SPEED_50MHZ)) ||
        (TFT_SPI_TRANSMIT_TIMEOUT <= 0) ||
        ((u64)TFT_SPI_TRANSMIT_TIMEOUT > UINT32_MAX) ||
        (TFT_SPI_COMPLETION_TIMEOUT <= 0) ||
        ((u64)TFT_SPI_COMPLETION_TIMEOUT > UINT32_MAX))
    {
        return ERROR_STATUS_ERROR;
    }
    for (i = 0U; i < TFT_SIGNAL_COUNT; ++i)
    {
        if (((tftPins[i].port != DIO_PORT_A) &&
             (tftPins[i].port != DIO_PORT_B) &&
             (tftPins[i].port != DIO_PORT_C) &&
             (tftPins[i].port != DIO_PORT_D)) ||
            ((unsigned int)tftPins[i].pin >= (unsigned int)DIO_PIN_COUNT))
        {
            return ERROR_STATUS_ERROR;
        }
        for (j = 0U; j < i; ++j)
        {
            if ((tftPins[i].port == tftPins[j].port) &&
                (tftPins[i].pin == tftPins[j].pin))
            {
                return ERROR_STATUS_ERROR;
            }
        }
    }
    return ERROR_STATUS_OK;
}

/* Writes a control signal without retrying or changing other signals. */
static ErrorStatus_t TFT_WriteSignal(TFT_Signal_t signal, DIO_Level_t level)
{
    return DIO_WritePin(tftPins[signal].port, tftPins[signal].pin, level);
}

/* Invalidates state and attempts transaction abort, not SPI recovery. */
static ErrorStatus_t TFT_Fail(void)
{
    initState = TFT_INIT_STATE_IDLE;
    if (configuredCount > 0U)
    {
        (void)TFT_WriteSignal(TFT_SIGNAL_CS, DIO_LEVEL_HIGH);
    }
    return ERROR_STATUS_ERROR;
}

/* Restores idle levels only for pins successfully configured in this start. */
static void TFT_RecoverConfiguredPins(void)
{
    u8 i;
    for (i = 0U; i < configuredCount; ++i)
    {
        (void)TFT_WriteSignal((TFT_Signal_t)i, idleLevels[i]);
    }
}

/* Detects SPI errors without clearing flags or consuming received data. */
static ErrorStatus_t TFT_CheckSPIErrors(void)
{
    static const SPI_StatusFlag_t flags[] =
    {
        SPI_FLAG_OVR, SPI_FLAG_MODF, SPI_FLAG_CRCERR
    };
    u8 i;
    u8 value;
    for (i = 0U; i < 3U; ++i)
    {
        if ((SPI_GetFlag(TFT_SPI_INSTANCE, flags[i], &value) != ERROR_STATUS_OK) ||
            (value != 0U))
        {
            return ERROR_STATUS_ERROR;
        }
    }
    return ERROR_STATUS_OK;
}

/* Polls one completion condition with a finite iteration budget. */
static ErrorStatus_t TFT_WaitFlag(SPI_StatusFlag_t flag, u8 expected)
{
    u32 remaining = (u32)TFT_SPI_COMPLETION_TIMEOUT;
    u8 value;
    while (remaining > 0U)
    {
        --remaining;
        if ((TFT_CheckSPIErrors() != ERROR_STATUS_OK) ||
            (SPI_GetFlag(TFT_SPI_INSTANCE, flag, &value) != ERROR_STATUS_OK))
        {
            return ERROR_STATUS_ERROR;
        }
        if (value == expected)
        {
            return ERROR_STATUS_OK;
        }
    }
    return ERROR_STATUS_ERROR;
}

/* Submits one byte and confirms TXE, idle and error-free completion. */
static ErrorStatus_t TFT_SendByte(u8 value)
{
    if ((SPI_Transmit(TFT_SPI_INSTANCE, value,
                      (u32)TFT_SPI_TRANSMIT_TIMEOUT) != ERROR_STATUS_OK) ||
        (TFT_WaitFlag(SPI_FLAG_TXE, 1U) != ERROR_STATUS_OK) ||
        (TFT_WaitFlag(SPI_FLAG_BSY, 0U) != ERROR_STATUS_OK) ||
        (TFT_CheckSPIErrors() != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Sends a command and parameters with CS already active and SPI idle. */
static ErrorStatus_t TFT_SendSelectedCommand(u8 command, const u8 *parameters, u8 count)
{
    u8 i;
    if ((TFT_WriteSignal(TFT_SIGNAL_DC, DIO_LEVEL_LOW) != ERROR_STATUS_OK) ||
        (TFT_SendByte(command) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    if (count > 0U)
    {
        if (TFT_WriteSignal(TFT_SIGNAL_DC, DIO_LEVEL_HIGH) != ERROR_STATUS_OK)
        {
            return ERROR_STATUS_ERROR;
        }
        for (i = 0U; i < count; ++i)
        {
            if (TFT_SendByte(parameters[i]) != ERROR_STATUS_OK)
            {
                return ERROR_STATUS_ERROR;
            }
        }
    }
    return ERROR_STATUS_OK;
}

/* Completes one CS-bounded command transaction or aborts on failure. */
static ErrorStatus_t TFT_Command(u8 command, const u8 *parameters, u8 count)
{
    if ((TFT_WriteSignal(TFT_SIGNAL_CS, DIO_LEVEL_LOW) != ERROR_STATUS_OK) ||
        (TFT_SendSelectedCommand(command, parameters, count) != ERROR_STATUS_OK) ||
        (TFT_WriteSignal(TFT_SIGNAL_CS, DIO_LEVEL_HIGH) != ERROR_STATUS_OK))
    {
        return TFT_Fail();
    }
    return ERROR_STATUS_OK;
}

/* Accumulates elapsed time without wrapping; consumes no future-stage time. */
static u8 TFT_WaitElapsed(u32 elapsed, u32 required)
{
    if (elapsed > UINT32_MAX - elapsedWait)
    {
        elapsedWait = UINT32_MAX;
    }
    else
    {
        elapsedWait += elapsed;
    }
    if (elapsedWait >= required)
    {
        elapsedWait = 0U;
        return 1U;
    }
    return 0U;
}

/* Validates all wiring and locks before establishing control-pin idle levels. */
ErrorStatus_t TFT_InitStart(void)
{
    u8 i;
    u8 locked;
    ErrorStatus_t lockStatus = ERROR_STATUS_OK;
    DIO_PinConfig_t config;

    initState = TFT_INIT_STATE_IDLE;
    configuredCount = 0U;
    tableIndex = 0U;
    elapsedWait = 0U;
    if (TFT_ValidateConfig() != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    for (i = 0U; i < TFT_SIGNAL_COUNT; ++i)
    {
        if ((DIO_IsPinLocked(tftPins[i].port, tftPins[i].pin, &locked) != ERROR_STATUS_OK) ||
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
    config.speed = TFT_OUTPUT_SPEED;
    for (i = 0U; i < TFT_SIGNAL_COUNT; ++i)
    {
        config.initial_level = idleLevels[i];
        if (DIO_ConfigPin(tftPins[i].port, tftPins[i].pin, &config) != ERROR_STATUS_OK)
        {
            TFT_RecoverConfiguredPins();
            return ERROR_STATUS_ERROR;
        }
        ++configuredCount;
    }
    initState = TFT_INIT_STATE_WAIT_RST_HIGH_1;
    return ERROR_STATUS_OK;
}

/* Advances one initialization stage without millisecond busy waits. */
ErrorStatus_t TFT_InitProcess(u32 elapsedMilliseconds, u8 *complete)
{
    if ((complete == NULL) || (initState == TFT_INIT_STATE_IDLE))
    {
        return ERROR_STATUS_ERROR;
    }
    switch (initState)
    {
        case TFT_INIT_STATE_WAIT_RST_HIGH_1:
            if (TFT_WaitElapsed(elapsedMilliseconds, TFT_RESET_WAIT_MS) != 0U)
            {
                if (TFT_WriteSignal(TFT_SIGNAL_RST, DIO_LEVEL_LOW) != ERROR_STATUS_OK)
                {
                    return TFT_Fail();
                }
                initState = TFT_INIT_STATE_WAIT_RST_LOW;
            }
            break;
        case TFT_INIT_STATE_WAIT_RST_LOW:
            if (TFT_WaitElapsed(elapsedMilliseconds, TFT_RESET_WAIT_MS) != 0U)
            {
                if (TFT_WriteSignal(TFT_SIGNAL_RST, DIO_LEVEL_HIGH) != ERROR_STATUS_OK)
                {
                    return TFT_Fail();
                }
                initState = TFT_INIT_STATE_WAIT_RST_HIGH_2;
            }
            break;
        case TFT_INIT_STATE_WAIT_RST_HIGH_2:
            if (TFT_WaitElapsed(elapsedMilliseconds, TFT_RESET_WAIT_MS) != 0U)
            {
                initState = TFT_INIT_STATE_SEND_TABLE;
            }
            break;
        case TFT_INIT_STATE_SEND_TABLE:
            if (TFT_Command(initTable[tableIndex].command,
                            initTable[tableIndex].parameters,
                            initTable[tableIndex].count) != ERROR_STATUS_OK)
            {
                return ERROR_STATUS_ERROR;
            }
            ++tableIndex;
            if (tableIndex == sizeof(initTable) / sizeof(initTable[0]))
            {
                elapsedWait = 0U;
                initState = TFT_INIT_STATE_WAIT_AFTER_TABLE;
            }
            break;
        case TFT_INIT_STATE_WAIT_AFTER_TABLE:
            if (TFT_WaitElapsed(elapsedMilliseconds, TFT_AFTER_TABLE_MS) != 0U)
            {
                initState = TFT_INIT_STATE_SEND_SLPOUT;
            }
            break;
        case TFT_INIT_STATE_SEND_SLPOUT:
            if (TFT_Command(TFT_CMD_SLPOUT, NULL, 0U) != ERROR_STATUS_OK)
            {
                return ERROR_STATUS_ERROR;
            }
            elapsedWait = 0U;
            initState = TFT_INIT_STATE_WAIT_AFTER_SLPOUT;
            break;
        case TFT_INIT_STATE_WAIT_AFTER_SLPOUT:
            if (TFT_WaitElapsed(elapsedMilliseconds, TFT_AFTER_SLPOUT_MS) != 0U)
            {
                initState = TFT_INIT_STATE_SEND_DISPON;
            }
            break;
        case TFT_INIT_STATE_SEND_DISPON:
            if (TFT_Command(TFT_CMD_DISPON, NULL, 0U) != ERROR_STATUS_OK)
            {
                return ERROR_STATUS_ERROR;
            }
            initState = TFT_INIT_STATE_READY;
            break;
        case TFT_INIT_STATE_READY:
            break;
        default:
            return TFT_Fail();
    }
    *complete = (initState == TFT_INIT_STATE_READY) ? 1U : 0U;
    return ERROR_STATUS_OK;
}

/* Validates and opens an address-window transaction, leaving CS active. */
static ErrorStatus_t TFT_OpenWindow(u16 x, u16 y, u16 width, u16 height)
{
    u16 start;
    u16 end;
    u8 parameters[4];

    if ((x >= TFT_WIDTH) || (y >= TFT_HEIGHT) ||
        (width == 0U) || (height == 0U) ||
        (width > TFT_WIDTH - x) || (height > TFT_HEIGHT - y))
    {
        return ERROR_STATUS_ERROR;
    }
    start = (u16)(x + TFT_COLUMN_OFFSET);
    end = (u16)(x + width - 1U + TFT_COLUMN_OFFSET);
    parameters[0] = (u8)(start >> 8);
    parameters[1] = (u8)(start & 0xFFU);
    parameters[2] = (u8)(end >> 8);
    parameters[3] = (u8)(end & 0xFFU);
    if ((TFT_WriteSignal(TFT_SIGNAL_CS, DIO_LEVEL_LOW) != ERROR_STATUS_OK) ||
        (TFT_SendSelectedCommand(TFT_CMD_CASET, parameters, 4U) != ERROR_STATUS_OK))
    {
        return TFT_Fail();
    }
    start = (u16)(y + TFT_ROW_OFFSET);
    end = (u16)(y + height - 1U + TFT_ROW_OFFSET);
    parameters[0] = (u8)(start >> 8);
    parameters[1] = (u8)(start & 0xFFU);
    parameters[2] = (u8)(end >> 8);
    parameters[3] = (u8)(end & 0xFFU);
    if (TFT_SendSelectedCommand(TFT_CMD_RASET, parameters, 4U) != ERROR_STATUS_OK)
    {
        return TFT_Fail();
    }
    return ERROR_STATUS_OK;
}

/* Streams an exact RGB565 pixel count with explicit wire byte order. */
static ErrorStatus_t TFT_StreamColor(u16 color, u32 count)
{
    u32 i;
    for (i = 0U; i < count; ++i)
    {
        if ((TFT_SendByte((u8)(color >> 8)) != ERROR_STATUS_OK) ||
            (TFT_SendByte((u8)(color & 0xFFU)) != ERROR_STATUS_OK))
        {
            return ERROR_STATUS_ERROR;
        }
    }
    return ERROR_STATUS_OK;
}

/* Fills one validated window under a single CS assertion. */
ErrorStatus_t TFT_FillRectangle(u16 x, u16 y, u16 width, u16 height, u16 color)
{
    if (initState != TFT_INIT_STATE_READY)
    {
        return ERROR_STATUS_ERROR;
    }
    if (TFT_OpenWindow(x, y, width, height) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    if ((TFT_SendSelectedCommand(TFT_CMD_RAMWR, NULL, 0U) != ERROR_STATUS_OK) ||
        (TFT_WriteSignal(TFT_SIGNAL_DC, DIO_LEVEL_HIGH) != ERROR_STATUS_OK) ||
        (TFT_StreamColor(color, (u32)width * (u32)height) != ERROR_STATUS_OK) ||
        (TFT_WriteSignal(TFT_SIGNAL_CS, DIO_LEVEL_HIGH) != ERROR_STATUS_OK))
    {
        return TFT_Fail();
    }
    return ERROR_STATUS_OK;
}

/* Writes exactly one pixel through the common rectangle transaction. */
ErrorStatus_t TFT_DrawPixel(u16 x, u16 y, u16 color)
{
    return TFT_FillRectangle(x, y, 1U, 1U, color);
}

/* Fills the complete fixed portrait area without a framebuffer. */
ErrorStatus_t TFT_FillScreen(u16 color)
{
    return TFT_FillRectangle(0U, 0U, TFT_WIDTH, TFT_HEIGHT, color);
}
