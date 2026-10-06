/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< DMA_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : DMA_INTERFACE
 * Description : This file contains DMA1 channel configuration and transfer-control functionality.
 */

#ifndef DMA_INTERFACE_H
#define DMA_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

typedef enum
{
    DMA_CHANNEL_1 = 1,
    DMA_CHANNEL_2,
    DMA_CHANNEL_3,
    DMA_CHANNEL_4,
    DMA_CHANNEL_5,
    DMA_CHANNEL_6,
    DMA_CHANNEL_7
} DMA_Channel_t;

typedef enum
{
    DMA_DIRECTION_PERIPHERAL_TO_MEMORY,
    DMA_DIRECTION_MEMORY_TO_PERIPHERAL,
    DMA_DIRECTION_MEMORY_TO_MEMORY
} DMA_Direction_t;

typedef enum
{
    DMA_WIDTH_8_BIT,
    DMA_WIDTH_16_BIT,
    DMA_WIDTH_32_BIT
} DMA_DataWidth_t;

typedef enum
{
    DMA_INCREMENT_FIXED,
    DMA_INCREMENT_ENABLED
} DMA_Increment_t;

typedef enum
{
    DMA_MODE_NORMAL,
    DMA_MODE_CIRCULAR
} DMA_Mode_t;

typedef enum
{
    DMA_PRIORITY_LOW,
    DMA_PRIORITY_MEDIUM,
    DMA_PRIORITY_HIGH,
    DMA_PRIORITY_VERY_HIGH
} DMA_Priority_t;

typedef enum
{
    DMA_FLAG_GLOBAL,
    DMA_FLAG_TRANSFER_COMPLETE,
    DMA_FLAG_HALF_TRANSFER,
    DMA_FLAG_TRANSFER_ERROR
} DMA_Flag_t;

typedef struct
{
    DMA_Direction_t direction;
    u32 sourceAddress;
    u32 destinationAddress;
    u32 transferCount;
    DMA_DataWidth_t sourceWidth;
    DMA_DataWidth_t destinationWidth;
    DMA_Increment_t sourceIncrement;
    DMA_Increment_t destinationIncrement;
    DMA_Mode_t mode;
    DMA_Priority_t priority;
    u8 transferCompleteInterrupt;
    u8 halfTransferInterrupt;
    u8 transferErrorInterrupt;
} DMA_ChannelConfig_t;

/* Caller owns clocks, IRQs, request routing and serialized channel access.
 * Addresses must name DMA-accessible storage with suitable permissions/lifetime.
 * Width alignment is checked; memory-map validity and buffer capacity are not.
 * transferCount is 1..65535 data items, not bytes; interrupt fields are 0 or 1.
 * Flags survive configuration/disable; clear stale flags before a new transfer.
 */

/* Configures a disabled channel without starting it; an enabled channel is rejected. */
ErrorStatus_t DMA_ConfigChannel(DMA_Channel_t channel,
                               const DMA_ChannelConfig_t *config);

/* Enables a nonempty channel; memory-to-memory transfers start immediately. */
ErrorStatus_t DMA_EnableChannel(DMA_Channel_t channel);

/* Disables a channel while preserving its configuration and flags. */
ErrorStatus_t DMA_DisableChannel(DMA_Channel_t channel);

/* Reads the remaining data-item count, leaving the output unchanged on error. */
ErrorStatus_t DMA_GetRemainingCount(DMA_Channel_t channel, u32 *remainingCount);

/* Reads a flag as 0 or 1, leaving the output unchanged on error. */
ErrorStatus_t DMA_GetFlag(DMA_Channel_t channel, DMA_Flag_t flag, u8 *isSet);

/* Clears the selected flag; GLOBAL clears all flags of this channel. */
ErrorStatus_t DMA_ClearFlag(DMA_Channel_t channel, DMA_Flag_t flag);

#endif
