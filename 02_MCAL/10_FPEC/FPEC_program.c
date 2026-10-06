/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< FPEC_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : FPEC_PROGRAM
 * Description : This file contains internal Flash programming and erase functionality.
 */

#include "FPEC_interface.h"
#include "FPEC_private.h"
#include "FPEC_config.h"

/* Resolves the private status mask without accessing hardware. */
static ErrorStatus_t FPEC_FlagMask(FPEC_StatusFlag_t flag, u32 *mask)
{
    switch (flag)
    {
        case FPEC_FLAG_BUSY: *mask = FPEC_BSY; break;
        case FPEC_FLAG_END_OF_OPERATION: *mask = FPEC_EOP; break;
        case FPEC_FLAG_PROGRAM_ERROR: *mask = FPEC_PGERR; break;
        case FPEC_FLAG_WRITE_PROTECTION_ERROR: *mask = FPEC_WRPRTERR; break;
        default: return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Checks idle/unlocked ownership without changing registers. */
static ErrorStatus_t FPEC_CheckReady(void)
{
    if ((FPEC_SR & FPEC_BSY) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    if ((FPEC_CR & (FPEC_LOCK | FPEC_OTHER_OWNER)) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Removes stale main-operation state and flags only after an idle check. */
static void FPEC_Prepare(void)
{
    FPEC_CR &= ~FPEC_MAIN_BITS;
    FPEC_SR = FPEC_CLEARABLE;
}

/* Polls completion and cleans up only once hardware permits register writes. */
static ErrorStatus_t FPEC_Finish(u32 timeout)
{
    u32 status;
    for (;;)
    {
        status = FPEC_SR;
        if ((status & FPEC_BSY) == 0U)
        {
            break;
        }
        if (timeout == 0U)
        {
            return ERROR_STATUS_ERROR;
        }
        --timeout;
    }
    FPEC_CR &= ~FPEC_MAIN_BITS;
    FPEC_SR = status & FPEC_CLEARABLE;
    return (((status & FPEC_ERRORS) == 0U) && ((status & FPEC_EOP) != 0U))
        ? ERROR_STATUS_OK : ERROR_STATUS_ERROR;
}

/* Verifies an already-completed erase over a fixed, validated region. */
static ErrorStatus_t FPEC_VerifyErased(u32 address, u32 size)
{
    u32 offset;
    for (offset = 0U; offset < size; offset += 2U)
    {
        if (FPEC_HALFWORD(address + offset) != UINT16_C(0xFFFF))
        {
            return ERROR_STATUS_ERROR;
        }
    }
    return ERROR_STATUS_OK;
}

/* Updates latency without writing the read-only prefetch status bit. */
ErrorStatus_t FPEC_SetLatency(FPEC_Latency_t latency)
{
    u32 encoding;
    switch (latency)
    {
        case FPEC_LATENCY_0_WS: encoding = 0U; break;
        case FPEC_LATENCY_1_WS: encoding = 1U; break;
        case FPEC_LATENCY_2_WS: encoding = 2U; break;
        default: return ERROR_STATUS_ERROR;
    }
    if ((FPEC_SR & FPEC_BSY) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    FPEC_ACR = (FPEC_ACR & ~(FPEC_LATENCY_MASK | FPEC_PRFTBS)) | encoding;
    return ((FPEC_ACR & FPEC_LATENCY_MASK) == encoding)
        ? ERROR_STATUS_OK : ERROR_STATUS_ERROR;
}

/* Enables prefetch without changing latency or half-cycle configuration. */
ErrorStatus_t FPEC_EnablePrefetch(void)
{
    if ((FPEC_SR & FPEC_BSY) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    FPEC_ACR = (FPEC_ACR & ~FPEC_PRFTBS) | FPEC_PRFTBE;
    return ((FPEC_ACR & FPEC_PRFTBE) != 0U) ? ERROR_STATUS_OK : ERROR_STATUS_ERROR;
}

/* Disables prefetch without changing latency or half-cycle configuration. */
ErrorStatus_t FPEC_DisablePrefetch(void)
{
    if ((FPEC_SR & FPEC_BSY) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    FPEC_ACR = FPEC_ACR & ~(FPEC_PRFTBS | FPEC_PRFTBE);
    return ((FPEC_ACR & FPEC_PRFTBE) == 0U) ? ERROR_STATUS_OK : ERROR_STATUS_ERROR;
}

/* Unlocks only when needed and verifies that the key sequence succeeded. */
ErrorStatus_t FPEC_Unlock(void)
{
    if ((FPEC_SR & FPEC_BSY) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    if ((FPEC_CR & FPEC_LOCK) == 0U)
    {
        return ERROR_STATUS_OK;
    }
    FPEC_KEYR = FPEC_KEY1;
    FPEC_KEYR = FPEC_KEY2;
    return ((FPEC_CR & FPEC_LOCK) == 0U) ? ERROR_STATUS_OK : ERROR_STATUS_ERROR;
}

/* Safely removes stale main-operation selection and locks the idle controller. */
ErrorStatus_t FPEC_Lock(void)
{
    u32 control;
    if ((FPEC_SR & FPEC_BSY) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    control = FPEC_CR;
    if ((control & FPEC_LOCK) != 0U)
    {
        return ERROR_STATUS_OK;
    }
    if ((control & FPEC_OTHER_OWNER) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    FPEC_CR = (control & ~FPEC_MAIN_BITS) | FPEC_LOCK;
    return ((FPEC_CR & FPEC_LOCK) != 0U) ? ERROR_STATUS_OK : ERROR_STATUS_ERROR;
}

/* Programs and verifies one half-word without automatically erasing or locking. */
ErrorStatus_t FPEC_ProgramHalfWord(u32 address, u16 data, u32 timeout)
{
    u16 previous;
    if ((address < FPEC_FLASH_START) || (address > (FPEC_FLASH_END - 1U))
        || ((address & 1U) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    if (FPEC_CheckReady() != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    previous = FPEC_HALFWORD(address);
    /* PM0075 permits non-erased programming only for the all-zero half-word. */
    if ((previous != UINT16_C(0xFFFF)) && (data != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    FPEC_Prepare();
    FPEC_CR |= FPEC_PG;
    FPEC_HALFWORD(address) = data;
    if (FPEC_Finish(timeout) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    return (FPEC_HALFWORD(address) == data) ? ERROR_STATUS_OK : ERROR_STATUS_ERROR;
}

/* Erases exactly one aligned main-Flash page and verifies all its half-words. */
ErrorStatus_t FPEC_ErasePage(u32 pageAddress, u32 timeout)
{
    if ((pageAddress < FPEC_FLASH_START) || (pageAddress > FPEC_FLASH_END)
        || (((pageAddress - FPEC_FLASH_START) % FPEC_PAGE_SIZE) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    if (FPEC_CheckReady() != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    FPEC_Prepare();
    FPEC_CR |= FPEC_PER;
    FPEC_AR = pageAddress;
    FPEC_CR |= FPEC_STRT;
    if (FPEC_Finish(timeout) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    return FPEC_VerifyErased(pageAddress, FPEC_PAGE_SIZE);
}

/* Erases all main user pages while leaving the information block untouched. */
ErrorStatus_t FPEC_MassErase(u32 timeout)
{
    if (FPEC_CheckReady() != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    FPEC_Prepare();
    FPEC_CR |= FPEC_MER;
    FPEC_CR |= FPEC_STRT;
    if (FPEC_Finish(timeout) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    return FPEC_VerifyErased(FPEC_FLASH_START, FPEC_FLASH_SIZE);
}

/* Returns the selected status as a logical value. */
ErrorStatus_t FPEC_GetFlag(FPEC_StatusFlag_t flag, u8 *isSet)
{
    u32 mask;
    if ((isSet == NULL) || (FPEC_FlagMask(flag, &mask) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    *isSet = ((FPEC_SR & mask) != 0U) ? 1U : 0U;
    return ERROR_STATUS_OK;
}

/* Directly writes one documented W1C bit; BUSY is never writable. */
ErrorStatus_t FPEC_ClearFlag(FPEC_StatusFlag_t flag)
{
    u32 mask;
    if ((FPEC_FlagMask(flag, &mask) != ERROR_STATUS_OK) || (mask == FPEC_BSY))
    {
        return ERROR_STATUS_ERROR;
    }
    if ((FPEC_SR & FPEC_BSY) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    FPEC_SR = mask;
    return ERROR_STATUS_OK;
}
