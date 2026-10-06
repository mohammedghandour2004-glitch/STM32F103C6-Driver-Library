/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< USART_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : USART_PRIVATE
 * Description : This file contains USART1 and USART2 serial communication functionality.
 */

#ifndef USART_PRIVATE_H
#define USART_PRIVATE_H

#include "../../01_LIB/STD_TYPES.h"

#define USART1_BASE UINT32_C(0x40013800)
#define USART2_BASE UINT32_C(0x40004400)
#define USART1_MAX_CLOCK UINT32_C(72000000)
#define USART2_MAX_CLOCK UINT32_C(36000000)
#define USART_SR_OFFSET  0x00U
#define USART_DR_OFFSET  0x04U
#define USART_BRR_OFFSET 0x08U
#define USART_CR1_OFFSET 0x0CU
#define USART_CR2_OFFSET 0x10U
#define USART_CR3_OFFSET 0x14U
#define USART_REG(BASE, OFFSET) (*(volatile u16 *)(uintptr_t)((BASE) + (OFFSET)))

#define USART_PE (UINT32_C(1) << 0U)
#define USART_FE (UINT32_C(1) << 1U)
#define USART_NE (UINT32_C(1) << 2U)
#define USART_ORE (UINT32_C(1) << 3U)
#define USART_IDLE (UINT32_C(1) << 4U)
#define USART_RXNE (UINT32_C(1) << 5U)
#define USART_TC (UINT32_C(1) << 6U)
#define USART_TXE (UINT32_C(1) << 7U)
#define USART_CTS (UINT32_C(1) << 9U)
#define USART_SBK (UINT32_C(1) << 0U)
#define USART_RWU (UINT32_C(1) << 1U)
#define USART_RE (UINT32_C(1) << 2U)
#define USART_TE (UINT32_C(1) << 3U)
#define USART_IDLEIE (UINT32_C(1) << 4U)
#define USART_RXNEIE (UINT32_C(1) << 5U)
#define USART_TCIE (UINT32_C(1) << 6U)
#define USART_TXEIE (UINT32_C(1) << 7U)
#define USART_PEIE (UINT32_C(1) << 8U)
#define USART_PS (UINT32_C(1) << 9U)
#define USART_PCE (UINT32_C(1) << 10U)
#define USART_M (UINT32_C(1) << 12U)
#define USART_UE (UINT32_C(1) << 13U)
#define USART_CLKEN (UINT32_C(1) << 11U)
#define USART_LINEN (UINT32_C(1) << 14U)
#define USART_EIE (UINT32_C(1) << 0U)
#define USART_IREN (UINT32_C(1) << 1U)
#define USART_HDSEL (UINT32_C(1) << 3U)
#define USART_SCEN (UINT32_C(1) << 5U)
#define USART_DMAR (UINT32_C(1) << 6U)
#define USART_DMAT (UINT32_C(1) << 7U)
#define USART_RTSE (UINT32_C(1) << 8U)
#define USART_CTSE (UINT32_C(1) << 9U)
#define USART_CTSIE (UINT32_C(1) << 10U)
#define USART_STOP_MASK UINT32_C(0x3000)
#define USART_STOP_TWO  UINT32_C(0x2000)
#define USART_CR1_OWNED UINT32_C(0x3FFF)
#define USART_CR2_OWNED UINT32_C(0x7840)
#define USART_CR3_OWNED UINT32_C(0x07FF)
#define USART_BRR_MIN  UINT32_C(16)
#define USART_BRR_MAX  UINT32_C(65535)
#define USART_RX_ERRORS (USART_PE | USART_FE | USART_NE | USART_ORE)
#define USART_TX_IRQS (USART_TXEIE | USART_TCIE)
#define USART_RX_IRQS (USART_RXNEIE | USART_PEIE | USART_IDLEIE)
#define USART_ALL_IRQS (USART_TX_IRQS | USART_RX_IRQS)

#endif
