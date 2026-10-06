/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< USART_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : USART_INTERFACE
 * Description : This file contains USART1 and USART2 serial communication functionality.
 */

#ifndef USART_INTERFACE_H
#define USART_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

typedef enum
{
    USART_INSTANCE_1,
    USART_INSTANCE_2
} USART_Instance_t;

typedef enum
{
    USART_WORD_LENGTH_8,
    USART_WORD_LENGTH_9
} USART_WordLength_t;

typedef enum
{
    USART_PARITY_NONE,
    USART_PARITY_EVEN,
    USART_PARITY_ODD
} USART_Parity_t;

typedef enum
{
    USART_STOP_BITS_1,
    USART_STOP_BITS_2
} USART_StopBits_t;

typedef enum
{
    USART_DIRECTION_TX,
    USART_DIRECTION_RX,
    USART_DIRECTION_TX_RX
} USART_Direction_t;

typedef enum
{
    USART_MODE_NORMAL,
    USART_MODE_HALF_DUPLEX
} USART_Mode_t;

typedef enum
{
    USART_FLOW_NONE,
    USART_FLOW_RTS,
    USART_FLOW_CTS,
    USART_FLOW_RTS_CTS
} USART_FlowControl_t;

typedef enum
{
    USART_FLAG_PARITY_ERROR,
    USART_FLAG_FRAMING_ERROR,
    USART_FLAG_NOISE_ERROR,
    USART_FLAG_OVERRUN_ERROR,
    USART_FLAG_IDLE,
    USART_FLAG_RX_NOT_EMPTY,
    USART_FLAG_TRANSMISSION_COMPLETE,
    USART_FLAG_TX_EMPTY,
    USART_FLAG_CTS
} USART_StatusFlag_t;

typedef enum
{
    USART_INTERRUPT_RXNE,
    USART_INTERRUPT_TXE,
    USART_INTERRUPT_TC,
    USART_INTERRUPT_IDLE,
    USART_INTERRUPT_PARITY_ERROR,
    USART_INTERRUPT_ERROR
} USART_Interrupt_t;

typedef enum
{
    USART_DMA_RX,
    USART_DMA_TX
} USART_DMARequest_t;

typedef struct
{
    u32 peripheralClockHz;
    u32 baudRate;
    USART_WordLength_t wordLength;
    USART_Parity_t parity;
    USART_StopBits_t stopBits;
    USART_Direction_t direction;
    USART_Mode_t mode;
    USART_FlowControl_t flowControl;
} USART_Config_t;

/* Payload bits: 8/no parity=8, 8/parity=7, 9/no parity=9, 9/parity=8.
 * Caller supplies actual PCLK and owns baud-error tolerance, clocks, pins and serialized access.
 * Half duplex supports TX or RX only, with no flow control; disable/configure to turn around.
 * Half duplex uses TX for both directions; caller configures its released/open-drain state.
 * RTS requires RX; CTS requires TX. Physical pin availability remains application-owned.
 * Polling TX rejects TX DMA/TXE/TC IRQ ownership; RX rejects RX DMA/RXNE/IDLE/parity/error IRQs.
 * ERROR interrupt controls EIE only; FE/NE/ORE DMA interrupts also require DMAR.
 * RXNE interrupt also reports ORE. Receive's SR/DR sequence may clear IDLE as well as errors.
 * Timeouts count unsuccessful polls, not time units; zero performs one immediate check.
 * Stop peer traffic and DMA/IRQ producers before disabling; no RX-in-progress flag exists.
 */

/* Validates and configures a disabled USART, leaving UE and local IRQ/DMA enables clear. */
ErrorStatus_t USART_Config(USART_Instance_t instance, const USART_Config_t *config);
/* Sets UE only, preserving configured transmitter and receiver selection. */
ErrorStatus_t USART_Enable(USART_Instance_t instance);
/* Waits for TC when transmitting, then clears UE without reading DR; failure leaves UE intact. */
ErrorStatus_t USART_Disable(USART_Instance_t instance, u32 timeout);
/* Submits one payload frame after TXE; final wire completion requires the TC wait API. */
ErrorStatus_t USART_Transmit(USART_Instance_t instance, u16 data, u32 timeout);
/* Receives one frame or discards an errored frame; output changes only on success. */
ErrorStatus_t USART_Receive(USART_Instance_t instance, u16 *data, u32 timeout);
/* Waits boundedly for final transmission completion without clearing status. */
ErrorStatus_t USART_WaitTransmissionComplete(USART_Instance_t instance, u32 timeout);
/* Reads a status flag as 0 or 1, preserving output on failure. */
ErrorStatus_t USART_GetFlag(USART_Instance_t instance, USART_StatusFlag_t flag, u8 *isSet);
/* Enables one local interrupt source without configuring NVIC. */
ErrorStatus_t USART_EnableInterrupt(USART_Instance_t instance, USART_Interrupt_t interrupt);
/* Disables one local interrupt source. */
ErrorStatus_t USART_DisableInterrupt(USART_Instance_t instance, USART_Interrupt_t interrupt);
/* Enables one local DMA request without configuring a DMA channel. */
ErrorStatus_t USART_EnableDMARequest(USART_Instance_t instance, USART_DMARequest_t request);
/* Disables one local DMA request. */
ErrorStatus_t USART_DisableDMARequest(USART_Instance_t instance, USART_DMARequest_t request);

#endif
