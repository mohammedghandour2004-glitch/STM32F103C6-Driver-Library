/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< OS_PROGRAM.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 04_SERVICES
 * Component   : OS_PROGRAM
 * Description : This file contains the cooperative task scheduler functionality.
 */

#ifndef OS_PROGRAM_H
#define OS_PROGRAM_H

/* Include this implementation in exactly ONE application translation unit.
 * Every other translation unit includes OS_interface.h only; public definitions stay external.
 */
#include "OS_interface.h"
#include "OS_private.h"
#include "OS_config.h"
#include "../02_MCAL/06_STK/STK_interface.h"

/* Conservative volatile publication before the ISR gate; not general thread synchronization. */
static volatile OS_TaskControl_t osTasks[OS_MAX_TASKS];
static volatile OS_State_t osState = OS_STATE_UNINITIALIZED;
static u16 osCursor;
static u8 osDispatchActive;

/* Validates scheduler configuration without touching SysTick. */
static ErrorStatus_t OS_ValidateConfig(void)
{
    if ((OS_MAX_TASKS < 1U) || (OS_MAX_TASKS > 255U) ||
        (OS_STK_PERIOD_TICKS < 2U) || (OS_STK_PERIOD_TICKS > 16777216U) ||
        ((OS_STK_CLOCK_SOURCE != STK_CLOCK_HCLK) &&
         (OS_STK_CLOCK_SOURCE != STK_CLOCK_HCLK_DIV_8)))
    {
        return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Closes the ISR gate before attempting both independent hardware cleanup operations. */
static ErrorStatus_t OS_FailStart(void)
{
    ErrorStatus_t disableResult;
    ErrorStatus_t stopResult;
    osState = OS_STATE_INITIALIZED;
    disableResult = STK_DisableInterrupt();
    stopResult = STK_Stop();
    if ((disableResult != ERROR_STATUS_OK) || (stopResult != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    /* Successful cleanup does not turn the original failed start into success. */
    return ERROR_STATUS_ERROR;
}

/* Clears pre-start scheduler configuration without accessing SysTick. */
ErrorStatus_t OS_Init(void)
{
    u16 i;
    if (osState == OS_STATE_STARTED)
    {
        return ERROR_STATUS_ERROR;
    }
    for (i = 0U; i < OS_MAX_TASKS; ++i)
    {
        osTasks[i].function = NULL;
        osTasks[i].periodTicks = 0U;
        osTasks[i].remainingTicks = 0U;
        osTasks[i].request = 0U;
        osTasks[i].acknowledgment = 0U;
    }
    osCursor = 0U;
    osDispatchActive = 0U;
    osState = OS_STATE_INITIALIZED;
    return ERROR_STATUS_OK;
}

/* Commits one fully validated task to an unused pre-start slot. */
ErrorStatus_t OS_CreateTask(u8 taskId, OS_TaskFunction_t task,
                          u32 periodTicks, u32 initialDelayTicks)
{
    if ((osState != OS_STATE_INITIALIZED) || (taskId >= OS_MAX_TASKS) ||
        (task == NULL) || (periodTicks == 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    if (osTasks[taskId].function != NULL)
    {
        return ERROR_STATUS_ERROR;
    }
    osTasks[taskId].periodTicks = periodTicks;
    osTasks[taskId].remainingTicks = initialDelayTicks;
    osTasks[taskId].request = 0U;
    osTasks[taskId].acknowledgment = 0U;
    osTasks[taskId].function = task;
    return ERROR_STATUS_OK;
}

/* Configures the owned tick source and publishes state before allowing tick requests. */
ErrorStatus_t OS_Start(void)
{
    u16 i;
    if ((osState != OS_STATE_INITIALIZED) || (OS_ValidateConfig() != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    if (STK_DisableInterrupt() != ERROR_STATUS_OK)
    {
        return OS_FailStart();
    }
    if (STK_Stop() != ERROR_STATUS_OK)
    {
        return OS_FailStart();
    }
    if (STK_Configure(OS_STK_CLOCK_SOURCE, OS_STK_PERIOD_TICKS) != ERROR_STATUS_OK)
    {
        return OS_FailStart();
    }
    for (i = 0U; i < OS_MAX_TASKS; ++i)
    {
        osTasks[i].request = 0U;
        osTasks[i].acknowledgment = 0U;
    }
    osCursor = 0U;
    osDispatchActive = 0U;
    osState = OS_STATE_STARTED;
    if (STK_EnableInterrupt() != ERROR_STATUS_OK)
    {
        return OS_FailStart();
    }
    if (STK_Start() != ERROR_STATUS_OK)
    {
        return OS_FailStart();
    }
    return ERROR_STATUS_OK;
}

/* Advances serviced-tick countdowns and coalesces each task to one pending release. */
void OS_Tick(void)
{
    u16 i;
    if (osState != OS_STATE_STARTED)
    {
        return;
    }
    for (i = 0U; i < OS_MAX_TASKS; ++i)
    {
        if (osTasks[i].function != NULL)
        {
            if (osTasks[i].remainingTicks > 0U)
            {
                --osTasks[i].remainingTicks;
            }
            if (osTasks[i].remainingTicks == 0U)
            {
                if (osTasks[i].request == osTasks[i].acknowledgment)
                {
                    osTasks[i].request = (osTasks[i].request == 0U) ? 1U : 0U;
                }
                osTasks[i].remainingTicks = osTasks[i].periodTicks;
            }
        }
    }
}

/* Claims at most one release and advances the fair cursor before invoking its callback. */
ErrorStatus_t OS_Dispatch(void)
{
    u16 scanned;
    u16 slot;
    u8 observed;
    OS_TaskFunction_t function;
    if ((osState != OS_STATE_STARTED) || (osDispatchActive != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    slot = osCursor;
    for (scanned = 0U; scanned < OS_MAX_TASKS; ++scanned)
    {
        function = osTasks[slot].function;
        if (function != NULL)
        {
            observed = osTasks[slot].request;
            if (observed != osTasks[slot].acknowledgment)
            {
                /* Pre-claim releases coalesce; post-ack releases remain pending during the call. */
                osTasks[slot].acknowledgment = observed;
                osCursor = (u16)(slot + 1U);
                if (osCursor >= OS_MAX_TASKS)
                {
                    osCursor = 0U;
                }
                osDispatchActive = 1U;
                function();
                osDispatchActive = 0U;
                return ERROR_STATUS_OK;
            }
        }
        ++slot;
        if (slot >= OS_MAX_TASKS)
        {
            slot = 0U;
        }
    }
    return ERROR_STATUS_OK;
}

#endif
