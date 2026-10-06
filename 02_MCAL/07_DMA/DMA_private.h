/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< DMA_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : DMA_PRIVATE
 * Description : This file contains DMA1 channel configuration and transfer-control functionality.
 */

#ifndef DMA_PRIVATE_H
#define DMA_PRIVATE_H

#include "../../01_LIB/STD_TYPES.h"

#define DMA1_BASE_ADDRESS       UINT32_C(0x40020000)
#define DMA_ISR_OFFSET          UINT32_C(0x00)
#define DMA_IFCR_OFFSET         UINT32_C(0x04)
#define DMA_CHANNEL_BASE_OFFSET UINT32_C(0x08)
#define DMA_CHANNEL_STRIDE      UINT32_C(0x14)
#define DMA_CCR_OFFSET          UINT32_C(0x00)
#define DMA_CNDTR_OFFSET        UINT32_C(0x04)
#define DMA_CPAR_OFFSET         UINT32_C(0x08)
#define DMA_CMAR_OFFSET         UINT32_C(0x0C)
#define DMA_COUNT_MASK          UINT32_C(0xFFFF)

#define DMA_CCR_EN              (UINT32_C(1) << 0U)
#define DMA_CCR_TCIE            (UINT32_C(1) << 1U)
#define DMA_CCR_HTIE            (UINT32_C(1) << 2U)
#define DMA_CCR_TEIE            (UINT32_C(1) << 3U)
#define DMA_CCR_DIR             (UINT32_C(1) << 4U)
#define DMA_CCR_CIRC            (UINT32_C(1) << 5U)
#define DMA_CCR_PINC            (UINT32_C(1) << 6U)
#define DMA_CCR_MINC            (UINT32_C(1) << 7U)
#define DMA_CCR_PSIZE_SHIFT     8U
#define DMA_CCR_MSIZE_SHIFT     10U
#define DMA_CCR_PL_SHIFT        12U
#define DMA_CCR_MEM2MEM         (UINT32_C(1) << 14U)
#define DMA_FLAGS_PER_CHANNEL   4U

#define DMA_REGISTER(OFFSET) \
    (*(volatile u32 *)(uintptr_t)(DMA1_BASE_ADDRESS + (OFFSET)))

#endif
