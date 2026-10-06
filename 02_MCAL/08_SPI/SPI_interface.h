/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< SPI_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : SPI_INTERFACE
 * Description : This file contains SPI1 serial communication functionality.
 */

#ifndef SPI_INTERFACE_H
#define SPI_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

typedef enum
{
    SPI_INSTANCE_1
} SPI_Instance_t;

typedef enum
{
    SPI_ROLE_MASTER,
    SPI_ROLE_SLAVE
} SPI_Role_t;

typedef enum
{
    SPI_CPOL_LOW,
    SPI_CPOL_HIGH
} SPI_ClockPolarity_t;

typedef enum
{
    SPI_CPHA_FIRST_EDGE,
    SPI_CPHA_SECOND_EDGE
} SPI_ClockPhase_t;

typedef enum
{
    SPI_MSB_FIRST,
    SPI_LSB_FIRST
} SPI_BitOrder_t;

typedef enum
{
    SPI_BAUD_DIV_2 = 2,
    SPI_BAUD_DIV_4 = 4,
    SPI_BAUD_DIV_8 = 8,
    SPI_BAUD_DIV_16 = 16,
    SPI_BAUD_DIV_32 = 32,
    SPI_BAUD_DIV_64 = 64,
    SPI_BAUD_DIV_128 = 128,
    SPI_BAUD_DIV_256 = 256
} SPI_BaudPrescaler_t;

typedef enum
{
    SPI_DATA_8_BIT,
    SPI_DATA_16_BIT
} SPI_DataSize_t;

typedef enum
{
    SPI_FULL_DUPLEX,
    SPI_SIMPLEX_RX,
    SPI_HALF_DUPLEX_TX,
    SPI_HALF_DUPLEX_RX
} SPI_CommunicationMode_t;

typedef enum
{
    SPI_NSS_HARDWARE,
    SPI_NSS_SOFTWARE
} SPI_NSSMode_t;

typedef enum
{
    SPI_FLAG_RXNE,
    SPI_FLAG_TXE,
    SPI_FLAG_BSY,
    SPI_FLAG_OVR,
    SPI_FLAG_MODF,
    SPI_FLAG_CRCERR
} SPI_StatusFlag_t;

typedef enum
{
    SPI_INTERRUPT_RXNE,
    SPI_INTERRUPT_TXE,
    SPI_INTERRUPT_ERROR
} SPI_Interrupt_t;

typedef enum
{
    SPI_DMA_RX,
    SPI_DMA_TX
} SPI_DMARequest_t;

typedef struct
{
    SPI_Role_t role;
    SPI_ClockPolarity_t polarity;
    SPI_ClockPhase_t phase;
    SPI_BaudPrescaler_t baudPrescaler;
    SPI_BitOrder_t bitOrder;
    SPI_DataSize_t dataSize;
    SPI_CommunicationMode_t communicationMode;
    SPI_NSSMode_t nssMode;
    u8 softwareNSSLevel;
    u8 hardwareNSSOutput;
} SPI_Config_t;

/* Master supports FULL_DUPLEX and HALF_DUPLEX_TX; slave supports all four modes.
 * Slave baudPrescaler must be DIV_2 (unused); hardware NSS requires softwareNSSLevel=0.
 * Software NSS requires hardwareNSSOutput=0 and, for master, softwareNSSLevel=1.
 * Caller owns clocks, pins, NSS timing, serialized access and external master coordination.
 * Select PCLK2/prescaler and external SCK within the datasheet limits (18 MHz maximum).
 * Stop DMA/IRQ producers before shutdown; polling rejects enabled DMA/IRQ sources.
 * Timeouts count unsuccessful status polls, not time units; zero allows an immediate check.
 * A timeout does not cancel a submitted frame. Recover pending data/errors before retrying.
 * Receive/transfer outputs remain unchanged on failure; no automatic flag clearing occurs.
 */

/* Configures a disabled SPI1; clears local IRQ/DMA enables and disables CRC. */
ErrorStatus_t SPI_Config(SPI_Instance_t instance, const SPI_Config_t *config);
/* Enables SPI without changing its configuration. */
ErrorStatus_t SPI_Enable(SPI_Instance_t instance);
/* Waits boundedly for idle, preserving unread data; errors leave SPE unchanged. */
ErrorStatus_t SPI_Disable(SPI_Instance_t instance);
/* Sends one frame; in full duplex receives and discards the corresponding input. */
ErrorStatus_t SPI_Transmit(SPI_Instance_t instance, u16 data, u32 timeout);
/* Receives a slave frame only; master reception requires SPI_Transfer. */
ErrorStatus_t SPI_Receive(SPI_Instance_t instance, u16 *data, u32 timeout);
/* Exchanges one full-duplex frame; pending input is rejected, not discarded. */
ErrorStatus_t SPI_Transfer(SPI_Instance_t instance, u16 txData, u16 *rxData, u32 timeout);
/* Returns a status flag as 0 or 1, preserving output on error. */
ErrorStatus_t SPI_GetFlag(SPI_Instance_t instance, SPI_StatusFlag_t flag, u8 *isSet);
/* Clears OVR (discards DR), MODF (caller fixes NSS), or CRCERR; does not restore master role. */
ErrorStatus_t SPI_ClearErrorFlag(SPI_Instance_t instance, SPI_StatusFlag_t flag);
/* Enables one local SPI interrupt source. */
ErrorStatus_t SPI_EnableInterrupt(SPI_Instance_t instance, SPI_Interrupt_t interrupt);
/* Disables one local SPI interrupt source. */
ErrorStatus_t SPI_DisableInterrupt(SPI_Instance_t instance, SPI_Interrupt_t interrupt);
/* Enables one local DMA request without configuring DMA. */
ErrorStatus_t SPI_EnableDMARequest(SPI_Instance_t instance, SPI_DMARequest_t request);
/* Disables one local DMA request without configuring DMA. */
ErrorStatus_t SPI_DisableDMARequest(SPI_Instance_t instance, SPI_DMARequest_t request);

#endif
