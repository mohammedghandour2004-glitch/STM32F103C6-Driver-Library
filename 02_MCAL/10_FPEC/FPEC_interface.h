/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< FPEC_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : FPEC_INTERFACE
 * Description : This file contains internal Flash programming and erase functionality.
 */

#ifndef FPEC_INTERFACE_H
#define FPEC_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

typedef enum
{
    FPEC_LATENCY_0_WS,
    FPEC_LATENCY_1_WS,
    FPEC_LATENCY_2_WS
} FPEC_Latency_t;

typedef enum
{
    FPEC_FLAG_BUSY,
    FPEC_FLAG_END_OF_OPERATION,
    FPEC_FLAG_PROGRAM_ERROR,
    FPEC_FLAG_WRITE_PROTECTION_ERROR
} FPEC_StatusFlag_t;

/* Caller serializes all Flash access and owns image layout, interrupts and watchdogs.
 * HSI must remain ON throughout programming/erase; application owns this prerequisite.
 * Program/erase reject enabled Flash IRQ sources or an active option-byte mode.
 * Prepare latency before raising SYSCLK; lower SYSCLK before reducing latency.
 * Use 0 WS through 24 MHz, 1 WS through 48 MHz, 2 WS through 72 MHz.
 * Change prefetch only with SYSCLK < 24 MHz and HCLK=SYSCLK; keep it enabled if AHB is divided.
 * Clock preconditions are caller-owned; this driver neither reads nor calls RCC.
 * Flash accesses stall during program/erase: polling is not a wall-clock deadline.
 * Caller must ensure code, constants, vectors and return paths survive the operation,
 * or place them outside the affected Flash. No RAM relocation is provided.
 * Timeout zero checks immediately; each unsuccessful poll consumes one budget unit.
 * Timeout does not cancel hardware: while BSY remains set, no cleanup writes occur.
 * After BSY clears, the next operation or Lock clears stale main operation bits.
 * Operations consume stale/result EOP/error flags; outputs change only on success.
 */

/* Changes only access latency and verifies the requested setting. */
ErrorStatus_t FPEC_SetLatency(FPEC_Latency_t latency);
/* Enables prefetch under the caller-guaranteed clock preconditions above. */
ErrorStatus_t FPEC_EnablePrefetch(void);
/* Disables prefetch under the caller-guaranteed clock preconditions above. */
ErrorStatus_t FPEC_DisablePrefetch(void);
/* Unlocks the idle Flash controller using its private key sequence. */
ErrorStatus_t FPEC_Unlock(void);
/* Locks an idle controller after clearing stale main operation bits. */
ErrorStatus_t FPEC_Lock(void);
/* Programs an aligned main-Flash half-word; requires erased storage except for data=0. */
ErrorStatus_t FPEC_ProgramHalfWord(u32 address, u16 data, u32 timeout);
/* Erases and verifies exactly one page, selected by its aligned start address. */
ErrorStatus_t FPEC_ErasePage(u32 pageAddress, u32 timeout);
/* Destructively erases and verifies all main user Flash; information/option bytes are unaffected. */
ErrorStatus_t FPEC_MassErase(u32 timeout);
/* Reads a hardware flag as 0 or 1 without modifying output on error. */
ErrorStatus_t FPEC_GetFlag(FPEC_StatusFlag_t flag, u8 *isSet);
/* Clears one software-clearable flag while idle; BUSY cannot be cleared. */
ErrorStatus_t FPEC_ClearFlag(FPEC_StatusFlag_t flag);

#endif
