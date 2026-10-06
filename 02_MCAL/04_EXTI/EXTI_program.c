/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< EXTI_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : EXTI_PROGRAM
 * Description : This file contains the external interrupt controller functionality.
 */

#include "EXTI_interface.h"
#include "EXTI_private.h"
#include "EXTI_config.h"

/* Validates the contiguous implemented line range before forming a mask. */
static ErrorStatus_t EXTI_GetLineMask(EXTI_Line_t line, u32 *mask)
{
    if ((u32)line > EXTI_LAST_LINE)
    {
        return ERROR_STATUS_ERROR;
    }

    *mask = UINT32_C(1) << (u32)line;
    return ERROR_STATUS_OK;
}

/* Changes only the selected line's rising and falling trigger bits. */
ErrorStatus_t EXTI_SetTrigger(EXTI_Line_t line, EXTI_Trigger_t trigger)
{
    u32 mask;

    if ((EXTI_GetLineMask(line, &mask) != ERROR_STATUS_OK) ||
        ((trigger != EXTI_TRIGGER_RISING) &&
         (trigger != EXTI_TRIGGER_FALLING) &&
         (trigger != EXTI_TRIGGER_BOTH)))
    {
        return ERROR_STATUS_ERROR;
    }

    switch (trigger)
    {
        case EXTI_TRIGGER_RISING:
            EXTI_REGISTER(EXTI_FTSR_OFFSET) &= ~mask;
            EXTI_REGISTER(EXTI_RTSR_OFFSET) |= mask;
            break;

        case EXTI_TRIGGER_FALLING:
            EXTI_REGISTER(EXTI_RTSR_OFFSET) &= ~mask;
            EXTI_REGISTER(EXTI_FTSR_OFFSET) |= mask;
            break;

        case EXTI_TRIGGER_BOTH:
            EXTI_REGISTER(EXTI_RTSR_OFFSET) |= mask;
            EXTI_REGISTER(EXTI_FTSR_OFFSET) |= mask;
            break;

        default:
            return ERROR_STATUS_ERROR;
    }

    return ERROR_STATUS_OK;
}

/* Unmasks the selected EXTI interrupt line. */
ErrorStatus_t EXTI_EnableInterrupt(EXTI_Line_t line)
{
    u32 mask;

    if (EXTI_GetLineMask(line, &mask) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    EXTI_REGISTER(EXTI_IMR_OFFSET) |= mask;
    return ERROR_STATUS_OK;
}

/* Masks the selected EXTI interrupt line. */
ErrorStatus_t EXTI_DisableInterrupt(EXTI_Line_t line)
{
    u32 mask;

    if (EXTI_GetLineMask(line, &mask) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    EXTI_REGISTER(EXTI_IMR_OFFSET) &= ~mask;
    return ERROR_STATUS_OK;
}

/* Unmasks the selected EXTI event line. */
ErrorStatus_t EXTI_EnableEvent(EXTI_Line_t line)
{
    u32 mask;

    if (EXTI_GetLineMask(line, &mask) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    EXTI_REGISTER(EXTI_EMR_OFFSET) |= mask;
    return ERROR_STATUS_OK;
}

/* Masks the selected EXTI event line. */
ErrorStatus_t EXTI_DisableEvent(EXTI_Line_t line)
{
    u32 mask;

    if (EXTI_GetLineMask(line, &mask) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    EXTI_REGISTER(EXTI_EMR_OFFSET) &= ~mask;
    return ERROR_STATUS_OK;
}

/* Issues a software request without changing masks or edge selection. */
ErrorStatus_t EXTI_GenerateSoftwareTrigger(EXTI_Line_t line)
{
    u32 mask;

    if (EXTI_GetLineMask(line, &mask) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    /* SWIER triggers on 0-to-1; PR clearing rearms the corresponding bit. */
    EXTI_REGISTER(EXTI_SWIER_OFFSET) = mask;
    return ERROR_STATUS_OK;
}

/* Reads the selected pending flag without changing hardware state. */
ErrorStatus_t EXTI_GetPending(EXTI_Line_t line, u8 *isPending)
{
    u32 mask;
    u32 pending;

    if ((isPending == NULL) || (EXTI_GetLineMask(line, &mask) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    pending = EXTI_REGISTER(EXTI_PR_OFFSET);
    *isPending = (u8)((pending & mask) != 0U);
    return ERROR_STATUS_OK;
}

/* Clears the selected pending flag through a direct write-one-to-clear access. */
ErrorStatus_t EXTI_ClearPending(EXTI_Line_t line)
{
    u32 mask;

    if (EXTI_GetLineMask(line, &mask) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    EXTI_REGISTER(EXTI_PR_OFFSET) = mask;
    return ERROR_STATUS_OK;
}
