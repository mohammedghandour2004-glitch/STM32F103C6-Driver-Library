/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< SPI_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : SPI_PROGRAM
 * Description : This file contains SPI1 serial communication functionality.
 */

#include "SPI_interface.h"
#include "SPI_private.h"
#include "SPI_config.h"

/* Identifies receive-only hardware configurations. */
static u8 SPI_IsReceiveOnly(u16 cr1)
{
    return (u8)(((cr1 & SPI_BIDIMODE) != 0U)
        ? ((cr1 & SPI_BIDIOE) == 0U) : ((cr1 & SPI_RXONLY) != 0U));
}

/* Checks live mode restrictions without storing software configuration state. */
static ErrorStatus_t SPI_CheckMode(u16 cr1)
{
    if (((cr1 & (SPI_CRCEN | SPI_CRCNEXT)) != 0U)
        || (((cr1 & SPI_BIDIMODE) != 0U) && ((cr1 & SPI_RXONLY) != 0U))
        || (((cr1 & SPI_MSTR) != 0U) && (SPI_IsReceiveOnly(cr1) != 0U))
        || (((cr1 & (SPI_MSTR | SPI_SSM)) == (SPI_MSTR | SPI_SSM))
            && ((cr1 & SPI_SSI) == 0U)))
    {
        return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Validates polling ownership and the live enabled configuration. */
static ErrorStatus_t SPI_CheckPolling(SPI_Instance_t instance, u16 *cr1)
{
    if (instance != SPI_INSTANCE_1)
    {
        return ERROR_STATUS_ERROR;
    }
    *cr1 = SPI_CR1;
    if (((*cr1 & SPI_SPE) == 0U)
        || (SPI_CheckMode(*cr1) != ERROR_STATUS_OK)
        || ((SPI_CR2 & SPI_ASYNC) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Polls a status condition with a shared retry budget, aborting on errors. */
static ErrorStatus_t SPI_Wait(u16 mask, u16 expected, u32 *budget)
{
    for (;;)
    {
        u16 status = SPI_SR;
        if ((status & SPI_ERRORS) != 0U)
        {
            return ERROR_STATUS_ERROR;
        }
        if ((status & mask) == expected)
        {
            return ERROR_STATUS_OK;
        }
        if (*budget == 0U)
        {
            return ERROR_STATUS_ERROR;
        }
        --(*budget);
    }
}

/* Reads a frame using the documented half-word DR access. */
static u16 SPI_ReadFrame(u16 cr1)
{
    u16 value = SPI_DR;
    return ((cr1 & SPI_DFF) != 0U) ? value : (u16)(value & 0xFFU);
}

/* Encodes semantic settings before any peripheral register access. */
static ErrorStatus_t SPI_BuildConfig(const SPI_Config_t *c, u16 *cr1, u16 *cr2)
{
    u32 br;
    u32 value = 0U;
    if (c == NULL)
    {
        return ERROR_STATUS_ERROR;
    }
    if (((c->role != SPI_ROLE_MASTER) && (c->role != SPI_ROLE_SLAVE))
        || ((c->polarity != SPI_CPOL_LOW) && (c->polarity != SPI_CPOL_HIGH))
        || ((c->phase != SPI_CPHA_FIRST_EDGE) && (c->phase != SPI_CPHA_SECOND_EDGE))
        || ((c->bitOrder != SPI_MSB_FIRST) && (c->bitOrder != SPI_LSB_FIRST))
        || ((c->dataSize != SPI_DATA_8_BIT) && (c->dataSize != SPI_DATA_16_BIT))
        || ((c->nssMode != SPI_NSS_HARDWARE) && (c->nssMode != SPI_NSS_SOFTWARE))
        || (c->softwareNSSLevel > 1U) || (c->hardwareNSSOutput > 1U))
    {
        return ERROR_STATUS_ERROR;
    }
    switch (c->baudPrescaler)
    {
        case SPI_BAUD_DIV_2: br = 0U; break;
        case SPI_BAUD_DIV_4: br = 1U; break;
        case SPI_BAUD_DIV_8: br = 2U; break;
        case SPI_BAUD_DIV_16: br = 3U; break;
        case SPI_BAUD_DIV_32: br = 4U; break;
        case SPI_BAUD_DIV_64: br = 5U; break;
        case SPI_BAUD_DIV_128: br = 6U; break;
        case SPI_BAUD_DIV_256: br = 7U; break;
        default: return ERROR_STATUS_ERROR;
    }
    if ((c->role == SPI_ROLE_SLAVE) && (c->baudPrescaler != SPI_BAUD_DIV_2))
    {
        return ERROR_STATUS_ERROR;
    }
    switch (c->communicationMode)
    {
        case SPI_FULL_DUPLEX: break;
        case SPI_SIMPLEX_RX: value |= SPI_RXONLY; break;
        case SPI_HALF_DUPLEX_TX: value |= SPI_BIDIMODE | SPI_BIDIOE; break;
        case SPI_HALF_DUPLEX_RX: value |= SPI_BIDIMODE; break;
        default: return ERROR_STATUS_ERROR;
    }
    if ((c->role == SPI_ROLE_MASTER)
        && ((c->communicationMode == SPI_SIMPLEX_RX)
            || (c->communicationMode == SPI_HALF_DUPLEX_RX)))
    {
        return ERROR_STATUS_ERROR;
    }
    if (c->nssMode == SPI_NSS_SOFTWARE)
    {
        if ((c->hardwareNSSOutput != 0U)
            || ((c->role == SPI_ROLE_MASTER) && (c->softwareNSSLevel == 0U)))
        {
            return ERROR_STATUS_ERROR;
        }
        value |= SPI_SSM;
        if (c->softwareNSSLevel != 0U) { value |= SPI_SSI; }
    }
    else if ((c->softwareNSSLevel != 0U)
        || ((c->role == SPI_ROLE_SLAVE) && (c->hardwareNSSOutput != 0U)))
    {
        return ERROR_STATUS_ERROR;
    }
    if (c->role == SPI_ROLE_MASTER) { value |= SPI_MSTR; }
    if (c->polarity == SPI_CPOL_HIGH) { value |= SPI_CPOL; }
    if (c->phase == SPI_CPHA_SECOND_EDGE) { value |= SPI_CPHA; }
    if (c->bitOrder == SPI_LSB_FIRST) { value |= SPI_LSBFIRST; }
    if (c->dataSize == SPI_DATA_16_BIT) { value |= SPI_DFF; }
    *cr1 = (u16)(value | (br << SPI_BR_SHIFT));
    *cr2 = (c->hardwareNSSOutput != 0U) ? (u16)SPI_SSOE : 0U;
    return ERROR_STATUS_OK;
}

/* Programs a disabled SPI1 without enabling communication. */
ErrorStatus_t SPI_Config(SPI_Instance_t instance, const SPI_Config_t *config)
{
    u16 cr1;
    u16 cr2;
    if ((instance != SPI_INSTANCE_1)
        || (SPI_BuildConfig(config, &cr1, &cr2) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    if (((SPI_CR1 & SPI_SPE) != 0U) || ((SPI_SR & SPI_BSY) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    SPI_CR2 = (u16)((SPI_CR2 & ~SPI_CR2_OWNED) | cr2);
    SPI_CR1 = cr1;
    return ERROR_STATUS_OK;
}

/* Sets SPE only after checking the live supported configuration. */
ErrorStatus_t SPI_Enable(SPI_Instance_t instance)
{
    u16 cr1;
    if (instance != SPI_INSTANCE_1) { return ERROR_STATUS_ERROR; }
    cr1 = SPI_CR1;
    if ((SPI_CheckMode(cr1) != ERROR_STATUS_OK) || ((SPI_SR & SPI_ERRORS) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    SPI_CR1 = (u16)(cr1 | SPI_SPE);
    return ERROR_STATUS_OK;
}

/* Waits for a safe idle boundary without discarding unread data. */
ErrorStatus_t SPI_Disable(SPI_Instance_t instance)
{
    u16 cr1;
    u32 budget = SPI_DISABLE_POLL_LIMIT;
    if (instance != SPI_INSTANCE_1) { return ERROR_STATUS_ERROR; }
    cr1 = SPI_CR1;
    if ((cr1 & SPI_SPE) == 0U) { return ERROR_STATUS_OK; }
    if ((SPI_CheckMode(cr1) != ERROR_STATUS_OK) || ((SPI_CR2 & SPI_ASYNC) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    if ((SPI_IsReceiveOnly(cr1) == 0U)
        && (SPI_Wait((u16)SPI_TXE, (u16)SPI_TXE, &budget) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    if (SPI_Wait((u16)SPI_BSY, 0U, &budget) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    /* The caller must consume the final receive frame before shutdown. */
    if ((SPI_SR & (SPI_RXNE | SPI_ERRORS)) != 0U) { return ERROR_STATUS_ERROR; }
    SPI_CR1 = (u16)(cr1 & ~SPI_SPE);
    return ERROR_STATUS_OK;
}

/* Exchanges one frame without overwriting unread input. */
ErrorStatus_t SPI_Transfer(SPI_Instance_t instance, u16 txData, u16 *rxData, u32 timeout)
{
    u16 cr1;
    u16 received;
    if ((rxData == NULL) || (SPI_CheckPolling(instance, &cr1) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    if (((cr1 & (SPI_BIDIMODE | SPI_RXONLY)) != 0U)
        || (((cr1 & SPI_DFF) == 0U) && (txData > 0xFFU))
        || ((SPI_SR & (SPI_RXNE | SPI_BSY | SPI_ERRORS)) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    if (SPI_Wait((u16)SPI_TXE, (u16)SPI_TXE, &timeout) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    if ((SPI_SR & (SPI_RXNE | SPI_ERRORS)) != 0U) { return ERROR_STATUS_ERROR; }
    SPI_DR = txData;
    if (SPI_Wait((u16)SPI_RXNE, (u16)SPI_RXNE, &timeout) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    received = SPI_ReadFrame(cr1);
    if ((SPI_SR & SPI_ERRORS) != 0U) { return ERROR_STATUS_ERROR; }
    *rxData = received;
    return ERROR_STATUS_OK;
}

/* Sends a frame, draining its received counterpart in full-duplex mode. */
ErrorStatus_t SPI_Transmit(SPI_Instance_t instance, u16 data, u32 timeout)
{
    u16 cr1;
    u16 discarded;
    if (SPI_CheckPolling(instance, &cr1) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    if ((SPI_IsReceiveOnly(cr1) != 0U)
        || (((cr1 & SPI_DFF) == 0U) && (data > 0xFFU)))
    {
        return ERROR_STATUS_ERROR;
    }
    if ((cr1 & SPI_BIDIMODE) == 0U)
    {
        return SPI_Transfer(instance, data, &discarded, timeout);
    }
    if (SPI_Wait((u16)SPI_TXE, (u16)SPI_TXE, &timeout) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    SPI_DR = data;
    return ERROR_STATUS_OK;
}

/* Receives a slave frame without generating a master clock. */
ErrorStatus_t SPI_Receive(SPI_Instance_t instance, u16 *data, u32 timeout)
{
    u16 cr1;
    u16 received;
    if ((data == NULL) || (SPI_CheckPolling(instance, &cr1) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    if (((cr1 & SPI_MSTR) != 0U)
        || ((cr1 & (SPI_BIDIMODE | SPI_BIDIOE)) == (SPI_BIDIMODE | SPI_BIDIOE)))
    {
        return ERROR_STATUS_ERROR;
    }
    if (SPI_Wait((u16)SPI_RXNE, (u16)SPI_RXNE, &timeout) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    received = SPI_ReadFrame(cr1);
    if ((SPI_SR & SPI_ERRORS) != 0U) { return ERROR_STATUS_ERROR; }
    *data = received;
    return ERROR_STATUS_OK;
}

/* Maps a public status selector to its private bit. */
static ErrorStatus_t SPI_FlagMask(SPI_StatusFlag_t flag, u16 *mask)
{
    switch (flag)
    {
        case SPI_FLAG_RXNE: *mask = (u16)SPI_RXNE; break;
        case SPI_FLAG_TXE: *mask = (u16)SPI_TXE; break;
        case SPI_FLAG_BSY: *mask = (u16)SPI_BSY; break;
        case SPI_FLAG_OVR: *mask = (u16)SPI_OVR; break;
        case SPI_FLAG_MODF: *mask = (u16)SPI_MODF; break;
        case SPI_FLAG_CRCERR: *mask = (u16)SPI_CRCERR; break;
        default: return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Reports one hardware status bit without changing the output on error. */
ErrorStatus_t SPI_GetFlag(SPI_Instance_t instance, SPI_StatusFlag_t flag, u8 *isSet)
{
    u16 mask;
    if ((instance != SPI_INSTANCE_1) || (isSet == NULL)
        || (SPI_FlagMask(flag, &mask) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    *isSet = ((SPI_SR & mask) != 0U) ? 1U : 0U;
    return ERROR_STATUS_OK;
}

/* Applies the distinct hardware clear sequence for each supported error. */
ErrorStatus_t SPI_ClearErrorFlag(SPI_Instance_t instance, SPI_StatusFlag_t flag)
{
    u16 cr1;
    if (instance != SPI_INSTANCE_1) { return ERROR_STATUS_ERROR; }
    switch (flag)
    {
        case SPI_FLAG_OVR:
            (void)SPI_DR;
            (void)SPI_SR;
            break;
        case SPI_FLAG_MODF:
            (void)SPI_SR;
            cr1 = SPI_CR1;
            SPI_CR1 = cr1;
            break;
        case SPI_FLAG_CRCERR:
            SPI_SR = 0U;
            break;
        default: return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Resolves a local interrupt selector. */
static ErrorStatus_t SPI_InterruptMask(SPI_Interrupt_t interrupt, u16 *mask)
{
    switch (interrupt)
    {
        case SPI_INTERRUPT_RXNE: *mask = (u16)SPI_RXNEIE; break;
        case SPI_INTERRUPT_TXE: *mask = (u16)SPI_TXEIE; break;
        case SPI_INTERRUPT_ERROR: *mask = (u16)SPI_ERRIE; break;
        default: return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Resolves a local DMA request selector. */
static ErrorStatus_t SPI_DMAMask(SPI_DMARequest_t request, u16 *mask)
{
    switch (request)
    {
        case SPI_DMA_RX: *mask = (u16)SPI_RXDMAEN; break;
        case SPI_DMA_TX: *mask = (u16)SPI_TXDMAEN; break;
        default: return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Enables only the selected local interrupt bit. */
ErrorStatus_t SPI_EnableInterrupt(SPI_Instance_t instance, SPI_Interrupt_t interrupt)
{
    u16 mask;
    if ((instance != SPI_INSTANCE_1)
        || (SPI_InterruptMask(interrupt, &mask) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    SPI_CR2 = (u16)(SPI_CR2 | mask);
    return ERROR_STATUS_OK;
}

/* Disables only the selected local interrupt bit. */
ErrorStatus_t SPI_DisableInterrupt(SPI_Instance_t instance, SPI_Interrupt_t interrupt)
{
    u16 mask;
    if ((instance != SPI_INSTANCE_1)
        || (SPI_InterruptMask(interrupt, &mask) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    SPI_CR2 = (u16)(SPI_CR2 & (u16)~mask);
    return ERROR_STATUS_OK;
}

/* Enables only the selected local DMA request bit. */
ErrorStatus_t SPI_EnableDMARequest(SPI_Instance_t instance, SPI_DMARequest_t request)
{
    u16 mask;
    if ((instance != SPI_INSTANCE_1)
        || (SPI_DMAMask(request, &mask) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    SPI_CR2 = (u16)(SPI_CR2 | mask);
    return ERROR_STATUS_OK;
}

/* Disables only the selected local DMA request bit. */
ErrorStatus_t SPI_DisableDMARequest(SPI_Instance_t instance, SPI_DMARequest_t request)
{
    u16 mask;
    if ((instance != SPI_INSTANCE_1)
        || (SPI_DMAMask(request, &mask) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    SPI_CR2 = (u16)(SPI_CR2 & (u16)~mask);
    return ERROR_STATUS_OK;
}
