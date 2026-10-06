/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< IR_COMMUNICATION_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : IR_COMMUNICATION_PROGRAM
 * Description : This file contains the NEC infrared communication decoding functionality.
 */

#include "IR_Communication_interface.h"
#include "IR_Communication_private.h"
#include "IR_Communication_config.h"

static IR_Communication_State_t state;
static u32 workingFrame;
static u8 bitIndex;
static IR_Communication_Frame_t pendingFrame;
static u8 pendingValid;
static u8 previousAddress;
static u8 previousCommand;
static u8 previousValid;
static u32 repeatAge;
static u8 finalIsRepeat;
static u8 initialized;

/* Tests an inclusive timing window without accepting the unknown sentinel. */
static u8 IR_InWindow(u32 elapsed, u32 minimum, u32 maximum)
{
    return (u8)((elapsed != UINT32_MAX) &&
                (elapsed >= minimum) && (elapsed <= maximum));
}

/* Rejects zero, reversed, out-of-range, or sentinel-containing windows. */
static u8 IR_ValidWindow(u64 minimum, u64 maximum)
{
    return (u8)((minimum > 0U) && (minimum <= maximum) &&
                (maximum < UINT32_MAX));
}

/* Validates configured ranges and categories classified in the same state. */
static ErrorStatus_t IR_ValidateConfig(void)
{
    if ((IR_ValidWindow((u64)IR_LEADER_LOW_MIN_US,
                        (u64)IR_LEADER_LOW_MAX_US) == 0U) ||
        (IR_ValidWindow((u64)IR_FULL_LEADER_HIGH_MIN_US,
                        (u64)IR_FULL_LEADER_HIGH_MAX_US) == 0U) ||
        (IR_ValidWindow((u64)IR_REPEAT_LEADER_HIGH_MIN_US,
                        (u64)IR_REPEAT_LEADER_HIGH_MAX_US) == 0U) ||
        (IR_ValidWindow((u64)IR_DATA_LOW_MIN_US,
                        (u64)IR_DATA_LOW_MAX_US) == 0U) ||
        (IR_ValidWindow((u64)IR_ZERO_HIGH_MIN_US,
                        (u64)IR_ZERO_HIGH_MAX_US) == 0U) ||
        (IR_ValidWindow((u64)IR_ONE_HIGH_MIN_US,
                        (u64)IR_ONE_HIGH_MAX_US) == 0U) ||
        ((u64)IR_REPEAT_MAX_AGE_US == 0U) ||
        ((u64)IR_REPEAT_MAX_AGE_US >= UINT32_MAX))
    {
        return ERROR_STATUS_ERROR;
    }

    if (!((IR_FULL_LEADER_HIGH_MAX_US < IR_REPEAT_LEADER_HIGH_MIN_US) ||
          (IR_REPEAT_LEADER_HIGH_MAX_US < IR_FULL_LEADER_HIGH_MIN_US)) ||
        !((IR_ZERO_HIGH_MAX_US < IR_ONE_HIGH_MIN_US) ||
          (IR_ONE_HIGH_MAX_US < IR_ZERO_HIGH_MIN_US)))
    {
        return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Discards only the in-progress waveform, preserving pending output. */
static void IR_ResetDecoder(void)
{
    state = IR_STATE_IDLE;
    workingFrame = 0U;
    bitIndex = 0U;
    finalIsRepeat = 0U;
}

/* Discards malformed reception and invalidates repeat association. */
static ErrorStatus_t IR_Reject(void)
{
    IR_ResetDecoder();
    previousValid = 0U;
    repeatAge = UINT32_MAX;
    return ERROR_STATUS_ERROR;
}

/* Ages repeat association without allowing elapsed-time wraparound. */
static void IR_UpdateAge(u32 elapsed)
{
    if ((elapsed == UINT32_MAX) || (elapsed > (UINT32_MAX - repeatAge)))
    {
        repeatAge = UINT32_MAX;
    }
    else
    {
        repeatAge += elapsed;
    }
    if (repeatAge > IR_REPEAT_MAX_AGE_US)
    {
        previousValid = 0U;
    }
}

/* Preserves an unread result instead of silently overwriting it. */
static ErrorStatus_t IR_Publish(u8 address, u8 command, u8 isRepeat)
{
    if (pendingValid != 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    pendingFrame.address = address;
    pendingFrame.command = command;
    pendingFrame.isRepeat = isRepeat;
    pendingValid = 1U;
    return ERROR_STATUS_OK;
}

/* Validates a completed frame and publishes it or reports slot overflow. */
static ErrorStatus_t IR_Complete(void)
{
    u8 address;
    u8 inverseAddress;
    u8 command;
    u8 inverseCommand;
    u8 isRepeat = finalIsRepeat;
    ErrorStatus_t result;

    if (isRepeat != 0U)
    {
        if ((previousValid == 0U) || (repeatAge > IR_REPEAT_MAX_AGE_US))
        {
            return IR_Reject();
        }
        address = previousAddress;
        command = previousCommand;
    }
    else
    {
        if (bitIndex != 32U)
        {
            return IR_Reject();
        }
        address = (u8)(workingFrame & UINT32_C(0xFF));
        inverseAddress = (u8)((workingFrame >> 8U) & UINT32_C(0xFF));
        command = (u8)((workingFrame >> 16U) & UINT32_C(0xFF));
        inverseCommand = (u8)((workingFrame >> 24U) & UINT32_C(0xFF));
        if ((((u32)address ^ (u32)inverseAddress) != UINT32_C(0xFF)) ||
            (((u32)command ^ (u32)inverseCommand) != UINT32_C(0xFF)))
        {
            return IR_Reject();
        }
    }

    /* Valid reception updates association even if the result slot is occupied. */
    previousAddress = address;
    previousCommand = command;
    previousValid = 1U;
    repeatAge = 0U;
    result = IR_Publish(address, command, isRepeat);
    IR_ResetDecoder();
    return result;
}

/* Validates timing settings and initializes all private reception state. */
ErrorStatus_t IR_Communication_Init(void)
{
    initialized = 0U;
    if (IR_ValidateConfig() != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    IR_ResetDecoder();
    pendingFrame.address = 0U;
    pendingFrame.command = 0U;
    pendingFrame.isRepeat = 0U;
    pendingValid = 0U;
    previousAddress = 0U;
    previousCommand = 0U;
    previousValid = 0U;
    repeatAge = UINT32_MAX;
    initialized = 1U;
    return ERROR_STATUS_OK;
}

/* Processes one active-low receiver transition using caller-supplied timing. */
ErrorStatus_t IR_Communication_ProcessEdge(u32 elapsedMicroseconds, u8 levelAfterEdge)
{
    u8 bit;

    if (initialized == 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    if (levelAfterEdge > 1U)
    {
        return IR_Reject();
    }

    IR_UpdateAge(elapsedMicroseconds);
    if ((elapsedMicroseconds == UINT32_MAX) && (state != IR_STATE_IDLE))
    {
        return IR_Reject();
    }

    switch (state)
    {
        case IR_STATE_IDLE:
            if (levelAfterEdge == 0U)
            {
                state = IR_STATE_LEADER_LOW;
            }
            else
            {
                return IR_Reject();
            }
            break;

        case IR_STATE_LEADER_LOW:
            if ((levelAfterEdge != 1U) ||
                (IR_InWindow(elapsedMicroseconds, IR_LEADER_LOW_MIN_US,
                             IR_LEADER_LOW_MAX_US) == 0U))
            {
                return IR_Reject();
            }
            state = IR_STATE_LEADER_HIGH;
            break;

        case IR_STATE_LEADER_HIGH:
            if (levelAfterEdge != 0U)
            {
                return IR_Reject();
            }
            if (IR_InWindow(elapsedMicroseconds, IR_FULL_LEADER_HIGH_MIN_US,
                            IR_FULL_LEADER_HIGH_MAX_US) != 0U)
            {
                workingFrame = 0U;
                bitIndex = 0U;
                finalIsRepeat = 0U;
                state = IR_STATE_DATA_LOW;
            }
            else if (IR_InWindow(elapsedMicroseconds, IR_REPEAT_LEADER_HIGH_MIN_US,
                                 IR_REPEAT_LEADER_HIGH_MAX_US) != 0U)
            {
                finalIsRepeat = 1U;
                state = IR_STATE_FINAL_LOW;
            }
            else
            {
                return IR_Reject();
            }
            break;

        case IR_STATE_DATA_LOW:
            if ((levelAfterEdge != 1U) ||
                (IR_InWindow(elapsedMicroseconds, IR_DATA_LOW_MIN_US,
                             IR_DATA_LOW_MAX_US) == 0U))
            {
                return IR_Reject();
            }
            state = IR_STATE_DATA_HIGH;
            break;

        case IR_STATE_DATA_HIGH:
            if ((levelAfterEdge != 0U) || (bitIndex >= 32U))
            {
                return IR_Reject();
            }
            if (IR_InWindow(elapsedMicroseconds, IR_ZERO_HIGH_MIN_US,
                            IR_ZERO_HIGH_MAX_US) != 0U)
            {
                bit = 0U;
            }
            else if (IR_InWindow(elapsedMicroseconds, IR_ONE_HIGH_MIN_US,
                                 IR_ONE_HIGH_MAX_US) != 0U)
            {
                bit = 1U;
            }
            else
            {
                return IR_Reject();
            }
            workingFrame |= ((u32)bit << bitIndex);
            ++bitIndex;
            if (bitIndex == 32U)
            {
                finalIsRepeat = 0U;
                state = IR_STATE_FINAL_LOW;
            }
            else
            {
                state = IR_STATE_DATA_LOW;
            }
            break;

        case IR_STATE_FINAL_LOW:
            if ((levelAfterEdge != 1U) ||
                (IR_InWindow(elapsedMicroseconds, IR_DATA_LOW_MIN_US,
                             IR_DATA_LOW_MAX_US) == 0U))
            {
                return IR_Reject();
            }
            return IR_Complete();

        default:
            return IR_Reject();
    }
    return ERROR_STATUS_OK;
}

/* Copies and consumes a pending frame without blocking or changing invalid outputs. */
ErrorStatus_t IR_Communication_GetFrame(IR_Communication_Frame_t *frame, u8 *available)
{
    if ((initialized == 0U) || (frame == NULL) || (available == NULL))
    {
        return ERROR_STATUS_ERROR;
    }
    if (pendingValid == 0U)
    {
        *available = 0U;
        return ERROR_STATUS_OK;
    }
    *frame = pendingFrame;
    *available = 1U;
    pendingValid = 0U;
    return ERROR_STATUS_OK;
}
