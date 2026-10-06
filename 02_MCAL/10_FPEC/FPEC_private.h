/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< FPEC_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : FPEC_PRIVATE
 * Description : This file contains internal Flash programming and erase functionality.
 */

#ifndef FPEC_PRIVATE_H
#define FPEC_PRIVATE_H

#include "../../01_LIB/STD_TYPES.h"

#define FPEC_BASE UINT32_C(0x40022000)
#define FPEC_REGISTER(OFFSET) (*(volatile u32 *)(uintptr_t)(FPEC_BASE + (OFFSET)))
#define FPEC_ACR FPEC_REGISTER(0x00U)
#define FPEC_KEYR FPEC_REGISTER(0x04U)
#define FPEC_SR FPEC_REGISTER(0x0CU)
#define FPEC_CR FPEC_REGISTER(0x10U)
#define FPEC_AR FPEC_REGISTER(0x14U)
#define FPEC_HALFWORD(ADDRESS) (*(volatile u16 *)(uintptr_t)(ADDRESS))
#define FPEC_FLASH_START UINT32_C(0x08000000)
#define FPEC_FLASH_SIZE UINT32_C(0x8000)
#define FPEC_FLASH_END (FPEC_FLASH_START + FPEC_FLASH_SIZE - 1U)
#define FPEC_PAGE_SIZE UINT32_C(0x400)
#define FPEC_KEY1 UINT32_C(0x45670123)
#define FPEC_KEY2 UINT32_C(0xCDEF89AB)
#define FPEC_LATENCY_MASK UINT32_C(7)
#define FPEC_PRFTBE (UINT32_C(1) << 4U)
#define FPEC_PRFTBS (UINT32_C(1) << 5U)
#define FPEC_BSY (UINT32_C(1) << 0U)
#define FPEC_PGERR (UINT32_C(1) << 2U)
#define FPEC_WRPRTERR (UINT32_C(1) << 4U)
#define FPEC_EOP (UINT32_C(1) << 5U)
#define FPEC_PG (UINT32_C(1) << 0U)
#define FPEC_PER (UINT32_C(1) << 1U)
#define FPEC_MER (UINT32_C(1) << 2U)
#define FPEC_OPTPG (UINT32_C(1) << 4U)
#define FPEC_OPTER (UINT32_C(1) << 5U)
#define FPEC_STRT (UINT32_C(1) << 6U)
#define FPEC_LOCK (UINT32_C(1) << 7U)
#define FPEC_ERRIE (UINT32_C(1) << 10U)
#define FPEC_EOPIE (UINT32_C(1) << 12U)
#define FPEC_MAIN_BITS (FPEC_PG | FPEC_PER | FPEC_MER | FPEC_STRT)
#define FPEC_ERRORS (FPEC_PGERR | FPEC_WRPRTERR)
#define FPEC_CLEARABLE (FPEC_EOP | FPEC_ERRORS)
#define FPEC_OTHER_OWNER (FPEC_OPTPG | FPEC_OPTER | FPEC_ERRIE | FPEC_EOPIE)

#endif
