/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< OS_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 04_SERVICES
 * Component   : OS_INTERFACE
 * Description : This file contains the cooperative task scheduler functionality.
 */

#ifndef OS_INTERFACE_H
#define OS_INTERFACE_H

#include "../01_LIB/STD_TYPES.h"
#include "../01_LIB/ERROR_STATUS.h"

typedef void (*OS_TaskFunction_t)(void);

/* Cooperative, start-once scheduler: control/dispatch calls belong to serialized main context.
 * Application owns the SysTick vector and calls OS_Tick once per serviced exception only.
 * Before Start: provide privileged STK access, exclusive ownership, a routed handler,
 * no stale pending SysTick exception, and a stable clock matching the configured period.
 * OS cannot clear a pending exception. After Start, no other code may control STK.
 * Callbacks must return, must not call OS_Tick or recursively dispatch, and own their errors.
 * Long callbacks delay dispatch; there are no preemption or real-time deadline guarantees.
 * Releases coalesce to one pending run; serviced ticks cannot reconstruct missed periods.
 * HAL contracts requiring actual elapsed time still apply; task periods are not timestamps.
 */

/* Clears local tasks only; repeated pre-start Init resets configuration, but Started rejects it. */
ErrorStatus_t OS_Init(void);

/* Creates an unused slot before Start; period is positive, delay 0/1 releases on tick 1, N on N. */
ErrorStatus_t OS_CreateTask(u8 taskId, OS_TaskFunction_t task,
                          u32 periodTicks, u32 initialDelayTicks);

/* Acquires SysTick with input-clock tick configuration; task periods/delays use scheduler ticks.
 * On failure, cleanup is best-effort: restore external STK state, then Init/recreate before retry.
 * Success may already have serviced a legitimate tick before this call returns.
 */
ErrorStatus_t OS_Start(void);

/* Performs bounded ISR bookkeeping only; does nothing before Start. */
void OS_Tick(void);

/* Claims and runs at most one callback in rotating slot order; idle returns OK, recursion ERROR. */
ErrorStatus_t OS_Dispatch(void);

#endif
