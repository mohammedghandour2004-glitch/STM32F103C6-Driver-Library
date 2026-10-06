/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< STK_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : STK_PROGRAM
 * Description : This file contains the Cortex-M3 SysTick timer functionality.
 */

#include "STK_interface.h"
#include "STK_private.h"
#include "STK_config.h"

/* Configures a repeating period while stopped and preserves interrupt enable. */
ErrorStatus_t STK_Configure(STK_ClockSource_t clockSource, u32 periodTicks)
{
    u32 source;
    u32 control;

    if ((periodTicks < STK_PERIOD_MIN) || (periodTicks > STK_PERIOD_MAX))
    {
        return ERROR_STATUS_ERROR;
    }

    switch (clockSource)
    {
        case STK_CLOCK_HCLK: source = STK_CTRL_CLKSOURCE_MASK; break;
        case STK_CLOCK_HCLK_DIV_8: source = 0U; break;
        default: return ERROR_STATUS_ERROR;
    }

    control = STK_REGISTER(STK_CTRL_OFFSET) & STK_CTRL_WRITABLE_MASK;
    STK_REGISTER(STK_CTRL_OFFSET) = control & ~STK_CTRL_ENABLE_MASK;
    STK_REGISTER(STK_LOAD_OFFSET) = periodTicks - 1U;
    /* Writing VAL clears both the current count and COUNTFLAG. */
    STK_REGISTER(STK_VAL_OFFSET) = 0U;
    STK_REGISTER(STK_CTRL_OFFSET) = (control & STK_CTRL_TICKINT_MASK) | source;
    return ERROR_STATUS_OK;
}

/* Enables counting while preserving source and interrupt-enable state. */
ErrorStatus_t STK_Start(void)
{
    u32 control = STK_REGISTER(STK_CTRL_OFFSET) & STK_CTRL_WRITABLE_MASK;

    STK_REGISTER(STK_CTRL_OFFSET) = control | STK_CTRL_ENABLE_MASK;
    return ERROR_STATUS_OK;
}

/* Stops counting without altering LOAD or VAL. */
ErrorStatus_t STK_Stop(void)
{
    u32 control = STK_REGISTER(STK_CTRL_OFFSET) & STK_CTRL_WRITABLE_MASK;

    STK_REGISTER(STK_CTRL_OFFSET) = control & ~STK_CTRL_ENABLE_MASK;
    return ERROR_STATUS_OK;
}

/* Enables exception requests without changing counting or clock selection. */
ErrorStatus_t STK_EnableInterrupt(void)
{
    u32 control = STK_REGISTER(STK_CTRL_OFFSET) & STK_CTRL_WRITABLE_MASK;

    STK_REGISTER(STK_CTRL_OFFSET) = control | STK_CTRL_TICKINT_MASK;
    return ERROR_STATUS_OK;
}

/* Disables new exception requests without changing counting or clock selection. */
ErrorStatus_t STK_DisableInterrupt(void)
{
    u32 control = STK_REGISTER(STK_CTRL_OFFSET) & STK_CTRL_WRITABLE_MASK;

    STK_REGISTER(STK_CTRL_OFFSET) = control & ~STK_CTRL_TICKINT_MASK;
    return ERROR_STATUS_OK;
}

/* Reads only the implemented current-counter bits. */
ErrorStatus_t STK_GetCurrentValue(u32 *currentValue)
{
    if (currentValue == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    *currentValue = STK_REGISTER(STK_VAL_OFFSET) & STK_COUNTER_MASK;
    return ERROR_STATUS_OK;
}

/* Reads and consumes COUNTFLAG through one control-register read. */
ErrorStatus_t STK_GetCountFlag(u8 *countFlag)
{
    u32 control;

    if (countFlag == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    control = STK_REGISTER(STK_CTRL_OFFSET);
    *countFlag = (u8)((control & STK_CTRL_COUNTFLAG_MASK) != 0U);
    return ERROR_STATUS_OK;
}
