/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< DMA_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : DMA_PROGRAM
 * Description : This file contains DMA1 channel configuration and transfer-control functionality.
 */

#include "DMA_interface.h"
#include "DMA_private.h"
#include "DMA_config.h"

/* Resolves a valid DMA1 channel without accessing hardware. */
static ErrorStatus_t DMA_ResolveChannel(DMA_Channel_t channel, u32 *offset)
{
    if ((channel < DMA_CHANNEL_1) || (channel > DMA_CHANNEL_7))
    {
        return ERROR_STATUS_ERROR;
    }

    *offset = DMA_CHANNEL_BASE_OFFSET
            + (((u32)channel - 1U) * DMA_CHANNEL_STRIDE);
    return ERROR_STATUS_OK;
}

/* Converts a semantic width to its register encoding and checks alignment. */
static ErrorStatus_t DMA_EncodeWidth(DMA_DataWidth_t width, u32 address,
                                    u32 *encoding)
{
    u32 alignment;

    switch (width)
    {
        case DMA_WIDTH_8_BIT:
            *encoding = 0U;
            alignment = 1U;
            break;
        case DMA_WIDTH_16_BIT:
            *encoding = 1U;
            alignment = 2U;
            break;
        case DMA_WIDTH_32_BIT:
            *encoding = 2U;
            alignment = 4U;
            break;
        default:
            return ERROR_STATUS_ERROR;
    }

    return ((address & (alignment - 1U)) == 0U)
         ? ERROR_STATUS_OK : ERROR_STATUS_ERROR;
}

/* Resolves a validated flag to the shared ISR/IFCR bit position. */
static ErrorStatus_t DMA_ResolveFlag(DMA_Channel_t channel, DMA_Flag_t flag,
                                   u32 *mask)
{
    u32 offset;
    u32 bit;

    if (DMA_ResolveChannel(channel, &offset) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    switch (flag)
    {
        case DMA_FLAG_GLOBAL:
            bit = 0U;
            break;
        case DMA_FLAG_TRANSFER_COMPLETE:
            bit = 1U;
            break;
        case DMA_FLAG_HALF_TRANSFER:
            bit = 2U;
            break;
        case DMA_FLAG_TRANSFER_ERROR:
            bit = 3U;
            break;
        default:
            return ERROR_STATUS_ERROR;
    }

    *mask = UINT32_C(1) << ((((u32)channel - 1U) * DMA_FLAGS_PER_CHANNEL) + bit);
    return ERROR_STATUS_OK;
}

/* Validates and programs a disabled channel with semantic source/destination settings. */
ErrorStatus_t DMA_ConfigChannel(DMA_Channel_t channel,
                               const DMA_ChannelConfig_t *config)
{
    u32 offset;
    u32 sourceSize;
    u32 destinationSize;
    u32 peripheralAddress;
    u32 memoryAddress;
    u32 ccr = 0U;

    if ((DMA_ResolveChannel(channel, &offset) != ERROR_STATUS_OK)
        || (config == NULL))
    {
        return ERROR_STATUS_ERROR;
    }

    if ((config->transferCount == 0U)
        || (config->transferCount > DMA_COUNT_MASK)
        || (DMA_EncodeWidth(config->sourceWidth, config->sourceAddress,
                            &sourceSize) != ERROR_STATUS_OK)
        || (DMA_EncodeWidth(config->destinationWidth, config->destinationAddress,
                            &destinationSize) != ERROR_STATUS_OK)
        || ((config->sourceIncrement != DMA_INCREMENT_FIXED)
            && (config->sourceIncrement != DMA_INCREMENT_ENABLED))
        || ((config->destinationIncrement != DMA_INCREMENT_FIXED)
            && (config->destinationIncrement != DMA_INCREMENT_ENABLED))
        || ((config->mode != DMA_MODE_NORMAL)
            && (config->mode != DMA_MODE_CIRCULAR))
        || (config->transferCompleteInterrupt > 1U)
        || (config->halfTransferInterrupt > 1U)
        || (config->transferErrorInterrupt > 1U))
    {
        return ERROR_STATUS_ERROR;
    }

    switch (config->priority)
    {
        case DMA_PRIORITY_LOW:
            break;
        case DMA_PRIORITY_MEDIUM:
            ccr |= UINT32_C(1) << DMA_CCR_PL_SHIFT;
            break;
        case DMA_PRIORITY_HIGH:
            ccr |= UINT32_C(2) << DMA_CCR_PL_SHIFT;
            break;
        case DMA_PRIORITY_VERY_HIGH:
            ccr |= UINT32_C(3) << DMA_CCR_PL_SHIFT;
            break;
        default:
            return ERROR_STATUS_ERROR;
    }

    switch (config->direction)
    {
        case DMA_DIRECTION_MEMORY_TO_PERIPHERAL:
            peripheralAddress = config->destinationAddress;
            memoryAddress = config->sourceAddress;
            ccr |= DMA_CCR_DIR
                 | (destinationSize << DMA_CCR_PSIZE_SHIFT)
                 | (sourceSize << DMA_CCR_MSIZE_SHIFT);
            if (config->sourceIncrement == DMA_INCREMENT_ENABLED)
            {
                ccr |= DMA_CCR_MINC;
            }
            if (config->destinationIncrement == DMA_INCREMENT_ENABLED)
            {
                ccr |= DMA_CCR_PINC;
            }
            break;

        case DMA_DIRECTION_MEMORY_TO_MEMORY:
        case DMA_DIRECTION_PERIPHERAL_TO_MEMORY:
            if (config->direction == DMA_DIRECTION_MEMORY_TO_MEMORY)
            {
                if (config->mode == DMA_MODE_CIRCULAR)
                {
                    return ERROR_STATUS_ERROR;
                }
                ccr |= DMA_CCR_MEM2MEM;
            }
            /* DIR=0 reads CPAR, including when CPAR names a memory source. */
            peripheralAddress = config->sourceAddress;
            memoryAddress = config->destinationAddress;
            ccr |= (sourceSize << DMA_CCR_PSIZE_SHIFT)
                 | (destinationSize << DMA_CCR_MSIZE_SHIFT);
            if (config->sourceIncrement == DMA_INCREMENT_ENABLED)
            {
                ccr |= DMA_CCR_PINC;
            }
            if (config->destinationIncrement == DMA_INCREMENT_ENABLED)
            {
                ccr |= DMA_CCR_MINC;
            }
            break;

        default:
            return ERROR_STATUS_ERROR;
    }

    if (config->mode == DMA_MODE_CIRCULAR)
    {
        ccr |= DMA_CCR_CIRC;
    }
    if (config->transferCompleteInterrupt != 0U)
    {
        ccr |= DMA_CCR_TCIE;
    }
    if (config->halfTransferInterrupt != 0U)
    {
        ccr |= DMA_CCR_HTIE;
    }
    if (config->transferErrorInterrupt != 0U)
    {
        ccr |= DMA_CCR_TEIE;
    }

    if ((DMA_REGISTER(offset + DMA_CCR_OFFSET) & DMA_CCR_EN) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }

    DMA_REGISTER(offset + DMA_CCR_OFFSET) = 0U;
    DMA_REGISTER(offset + DMA_CNDTR_OFFSET) = config->transferCount;
    DMA_REGISTER(offset + DMA_CPAR_OFFSET) = peripheralAddress;
    DMA_REGISTER(offset + DMA_CMAR_OFFSET) = memoryAddress;
    DMA_REGISTER(offset + DMA_CCR_OFFSET) = ccr;
    return ERROR_STATUS_OK;
}

/* Enables a configured channel with a nonzero remaining count. */
ErrorStatus_t DMA_EnableChannel(DMA_Channel_t channel)
{
    u32 offset;

    if (DMA_ResolveChannel(channel, &offset) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    if ((DMA_REGISTER(offset + DMA_CNDTR_OFFSET) & DMA_COUNT_MASK) == 0U)
    {
        return ERROR_STATUS_ERROR;
    }

    DMA_REGISTER(offset + DMA_CCR_OFFSET) |= DMA_CCR_EN;
    return ERROR_STATUS_OK;
}

/* Clears only the enable bit without resetting channel state. */
ErrorStatus_t DMA_DisableChannel(DMA_Channel_t channel)
{
    u32 offset;

    if (DMA_ResolveChannel(channel, &offset) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    DMA_REGISTER(offset + DMA_CCR_OFFSET) &= ~DMA_CCR_EN;
    return ERROR_STATUS_OK;
}

/* Returns the hardware's current remaining data-item count. */
ErrorStatus_t DMA_GetRemainingCount(DMA_Channel_t channel, u32 *remainingCount)
{
    u32 offset;

    if ((DMA_ResolveChannel(channel, &offset) != ERROR_STATUS_OK)
        || (remainingCount == NULL))
    {
        return ERROR_STATUS_ERROR;
    }

    *remainingCount = DMA_REGISTER(offset + DMA_CNDTR_OFFSET) & DMA_COUNT_MASK;
    return ERROR_STATUS_OK;
}

/* Reads one channel flag as a logical value. */
ErrorStatus_t DMA_GetFlag(DMA_Channel_t channel, DMA_Flag_t flag, u8 *isSet)
{
    u32 mask;

    if ((isSet == NULL)
        || (DMA_ResolveFlag(channel, flag, &mask) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    *isSet = ((DMA_REGISTER(DMA_ISR_OFFSET) & mask) != 0U) ? 1U : 0U;
    return ERROR_STATUS_OK;
}

/* Writes only the selected W1C bit, never reading IFCR. */
ErrorStatus_t DMA_ClearFlag(DMA_Channel_t channel, DMA_Flag_t flag)
{
    u32 mask;

    if (DMA_ResolveFlag(channel, flag, &mask) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    DMA_REGISTER(DMA_IFCR_OFFSET) = mask;
    return ERROR_STATUS_OK;
}
