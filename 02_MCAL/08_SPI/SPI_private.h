/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< SPI_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : SPI_PRIVATE
 * Description : This file contains SPI1 serial communication functionality.
 */

#ifndef SPI_PRIVATE_H
#define SPI_PRIVATE_H

#include "../../01_LIB/STD_TYPES.h"

#define SPI1_BASE UINT32_C(0x40013000)
#define SPI_REG(OFFSET) (*(volatile u16 *)(uintptr_t)(SPI1_BASE + (OFFSET)))
#define SPI_CR1 SPI_REG(0x00U)
#define SPI_CR2 SPI_REG(0x04U)
#define SPI_SR  SPI_REG(0x08U)
#define SPI_DR  SPI_REG(0x0CU)

#define SPI_CPHA (UINT32_C(1) << 0U)
#define SPI_CPOL (UINT32_C(1) << 1U)
#define SPI_MSTR (UINT32_C(1) << 2U)
#define SPI_SPE (UINT32_C(1) << 6U)
#define SPI_LSBFIRST (UINT32_C(1) << 7U)
#define SPI_SSI (UINT32_C(1) << 8U)
#define SPI_SSM (UINT32_C(1) << 9U)
#define SPI_RXONLY (UINT32_C(1) << 10U)
#define SPI_DFF (UINT32_C(1) << 11U)
#define SPI_CRCNEXT (UINT32_C(1) << 12U)
#define SPI_CRCEN (UINT32_C(1) << 13U)
#define SPI_BIDIOE (UINT32_C(1) << 14U)
#define SPI_BIDIMODE (UINT32_C(1) << 15U)
#define SPI_RXDMAEN (UINT32_C(1) << 0U)
#define SPI_TXDMAEN (UINT32_C(1) << 1U)
#define SPI_SSOE (UINT32_C(1) << 2U)
#define SPI_ERRIE (UINT32_C(1) << 5U)
#define SPI_RXNEIE (UINT32_C(1) << 6U)
#define SPI_TXEIE (UINT32_C(1) << 7U)
#define SPI_RXNE (UINT32_C(1) << 0U)
#define SPI_TXE (UINT32_C(1) << 1U)
#define SPI_CRCERR (UINT32_C(1) << 4U)
#define SPI_MODF (UINT32_C(1) << 5U)
#define SPI_OVR (UINT32_C(1) << 6U)
#define SPI_BSY (UINT32_C(1) << 7U)
#define SPI_BR_SHIFT 3U
#define SPI_ERRORS (SPI_CRCERR | SPI_MODF | SPI_OVR)
#define SPI_ASYNC (SPI_RXDMAEN | SPI_TXDMAEN | SPI_ERRIE | SPI_RXNEIE | SPI_TXEIE)
#define SPI_CR2_OWNED (SPI_ASYNC | SPI_SSOE)

#endif
