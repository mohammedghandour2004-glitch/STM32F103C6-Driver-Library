/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< USART_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : USART_PROGRAM
 * Description : This file contains USART1 and USART2 serial communication functionality.
 */

#include "USART_interface.h"
#include "USART_private.h"
#include "USART_config.h"

/* Resolves only the two target USART instances without MMIO. */
static ErrorStatus_t USART_Resolve(USART_Instance_t instance, u32 *base, u32 *maxClock)
{
    switch (instance)
    {
        case USART_INSTANCE_1:
            *base = USART1_BASE;
            *maxClock = USART1_MAX_CLOCK;
            break;
        case USART_INSTANCE_2:
            *base = USART2_BASE;
            *maxClock = USART2_MAX_CLOCK;
            break;
        default:
            return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Computes the live user payload mask, excluding parity. */
static u16 USART_PayloadMask(u16 cr1)
{
    u16 mask = ((cr1 & USART_M) != 0U) ? 0x01FFU : 0x00FFU;
    return ((cr1 & USART_PCE) != 0U) ? (u16)(mask >> 1U) : mask;
}

/* Rejects live protocol modes outside this driver's asynchronous contract. */
static ErrorStatus_t USART_CheckMode(u32 base, u16 cr1, u16 cr3)
{
    u16 cr2 = USART_REG(base, USART_CR2_OFFSET);
    if (((cr1 & (USART_SBK | USART_RWU)) != 0U)
        || ((cr2 & (USART_LINEN | USART_CLKEN)) != 0U)
        || (((cr2 & USART_STOP_MASK) != 0U)
            && ((cr2 & USART_STOP_MASK) != USART_STOP_TWO))
        || ((cr3 & (USART_IREN | USART_SCEN)) != 0U)
        || (((cr3 & USART_HDSEL) != 0U)
            && (((cr1 & (USART_TE | USART_RE)) == (USART_TE | USART_RE))
                || ((cr3 & (USART_RTSE | USART_CTSE)) != 0U))))
    {
        return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Polls one status bit with a bounded retry count. */
static ErrorStatus_t USART_Wait(u32 base, u16 mask, u32 timeout)
{
    for (;;)
    {
        if ((USART_REG(base, USART_SR_OFFSET) & mask) != 0U)
        {
            return ERROR_STATUS_OK;
        }
        if (timeout == 0U)
        {
            return ERROR_STATUS_ERROR;
        }
        --timeout;
    }
}

/* Validates semantic settings and calculates the nearest fixed-16 BRR. */
static ErrorStatus_t USART_Build(const USART_Config_t *cfg, u32 maxClock,
                               u16 *cr1, u16 *cr2, u16 *cr3, u16 *brr)
{
    u64 divider;
    u32 control1 = 0U;
    u32 control3 = 0U;
    if ((cfg == NULL) || (cfg->peripheralClockHz == 0U)
        || (cfg->peripheralClockHz > maxClock) || (cfg->baudRate == 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    if (((u64)cfg->baudRate * USART_BRR_MIN > cfg->peripheralClockHz)
        || ((u64)cfg->baudRate * USART_BRR_MAX < cfg->peripheralClockHz))
    {
        return ERROR_STATUS_ERROR;
    }
    divider = ((u64)cfg->peripheralClockHz + (cfg->baudRate / 2U)) / cfg->baudRate;
    if ((divider < USART_BRR_MIN) || (divider > USART_BRR_MAX))
    {
        return ERROR_STATUS_ERROR;
    }
    switch (cfg->wordLength)
    {
        case USART_WORD_LENGTH_8: break;
        case USART_WORD_LENGTH_9: control1 |= USART_M; break;
        default: return ERROR_STATUS_ERROR;
    }
    switch (cfg->parity)
    {
        case USART_PARITY_NONE: break;
        case USART_PARITY_EVEN: control1 |= USART_PCE; break;
        case USART_PARITY_ODD: control1 |= USART_PCE | USART_PS; break;
        default: return ERROR_STATUS_ERROR;
    }
    switch (cfg->stopBits)
    {
        case USART_STOP_BITS_1: *cr2 = 0U; break;
        case USART_STOP_BITS_2: *cr2 = (u16)USART_STOP_TWO; break;
        default: return ERROR_STATUS_ERROR;
    }
    switch (cfg->direction)
    {
        case USART_DIRECTION_TX: control1 |= USART_TE; break;
        case USART_DIRECTION_RX: control1 |= USART_RE; break;
        case USART_DIRECTION_TX_RX: control1 |= USART_TE | USART_RE; break;
        default: return ERROR_STATUS_ERROR;
    }
    switch (cfg->flowControl)
    {
        case USART_FLOW_NONE: break;
        case USART_FLOW_RTS: control3 |= USART_RTSE; break;
        case USART_FLOW_CTS: control3 |= USART_CTSE; break;
        case USART_FLOW_RTS_CTS: control3 |= USART_RTSE | USART_CTSE; break;
        default: return ERROR_STATUS_ERROR;
    }
    if ((((control3 & USART_RTSE) != 0U) && ((control1 & USART_RE) == 0U))
        || (((control3 & USART_CTSE) != 0U) && ((control1 & USART_TE) == 0U)))
    {
        return ERROR_STATUS_ERROR;
    }
    switch (cfg->mode)
    {
        case USART_MODE_NORMAL: break;
        case USART_MODE_HALF_DUPLEX:
            if ((cfg->direction == USART_DIRECTION_TX_RX)
                || (cfg->flowControl != USART_FLOW_NONE))
            {
                return ERROR_STATUS_ERROR;
            }
            control3 |= USART_HDSEL;
            break;
        default: return ERROR_STATUS_ERROR;
    }
    *cr1 = (u16)control1;
    *cr3 = (u16)control3;
    *brr = (u16)divider;
    return ERROR_STATUS_OK;
}

/* Configures only a disabled peripheral after completing input validation. */
ErrorStatus_t USART_Config(USART_Instance_t instance, const USART_Config_t *config)
{
    u32 base;
    u32 maxClock;
    u16 cr1;
    u16 cr2;
    u16 cr3;
    u16 brr;
    u16 previous;
    if ((USART_Resolve(instance, &base, &maxClock) != ERROR_STATUS_OK)
        || (USART_Build(config, maxClock, &cr1, &cr2, &cr3, &brr) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    previous = USART_REG(base, USART_CR1_OFFSET);
    if ((previous & USART_UE) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    USART_REG(base, USART_CR1_OFFSET) = (u16)(previous & ~USART_CR1_OWNED);
    USART_REG(base, USART_CR2_OFFSET) =
        (u16)((USART_REG(base, USART_CR2_OFFSET) & ~USART_CR2_OWNED) | cr2);
    USART_REG(base, USART_CR3_OFFSET) =
        (u16)((USART_REG(base, USART_CR3_OFFSET) & ~USART_CR3_OWNED) | cr3);
    USART_REG(base, USART_BRR_OFFSET) = brr;
    USART_REG(base, USART_CR1_OFFSET) = (u16)((previous & ~USART_CR1_OWNED) | cr1);
    return ERROR_STATUS_OK;
}

/* Sets UE without changing direction or other configuration. */
ErrorStatus_t USART_Enable(USART_Instance_t instance)
{
    u32 base;
    u32 maxClock;
    if (USART_Resolve(instance, &base, &maxClock) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    USART_REG(base, USART_CR1_OFFSET) |= (u16)USART_UE;
    return ERROR_STATUS_OK;
}

/* Waits for final TX completion and preserves unread RX data during shutdown. */
ErrorStatus_t USART_Disable(USART_Instance_t instance, u32 timeout)
{
    u32 base;
    u32 maxClock;
    u16 cr1;
    u16 cr3;
    if (USART_Resolve(instance, &base, &maxClock) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    cr1 = USART_REG(base, USART_CR1_OFFSET);
    if ((cr1 & USART_UE) == 0U) { return ERROR_STATUS_OK; }
    cr3 = USART_REG(base, USART_CR3_OFFSET);
    if (((cr1 & USART_ALL_IRQS) != 0U)
        || ((cr3 & (USART_DMAT | USART_DMAR | USART_EIE | USART_CTSIE)) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    if (((cr1 & USART_TE) != 0U)
        && (USART_Wait(base, (u16)USART_TC, timeout) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    if ((USART_REG(base, USART_SR_OFFSET) & (USART_RXNE | USART_RX_ERRORS)) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    USART_REG(base, USART_CR1_OFFSET) = (u16)(cr1 & ~USART_UE);
    return ERROR_STATUS_OK;
}

/* Submits one validated payload when the transmit data register is empty. */
ErrorStatus_t USART_Transmit(USART_Instance_t instance, u16 data, u32 timeout)
{
    u32 base;
    u32 maxClock;
    u16 cr1;
    u16 cr3;
    if (USART_Resolve(instance, &base, &maxClock) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    cr1 = USART_REG(base, USART_CR1_OFFSET);
    cr3 = USART_REG(base, USART_CR3_OFFSET);
    if (((cr1 & (USART_UE | USART_TE)) != (USART_UE | USART_TE))
        || ((cr1 & USART_TX_IRQS) != 0U) || ((cr3 & USART_DMAT) != 0U)
        || (USART_CheckMode(base, cr1, cr3) != ERROR_STATUS_OK)
        || ((data & (u16)~USART_PayloadMask(cr1)) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    if (USART_Wait(base, (u16)USART_TXE, timeout) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    /* The preceding SR read and this DR write also clear TC. */
    USART_REG(base, USART_DR_OFFSET) = data;
    return ERROR_STATUS_OK;
}

/* Consumes a valid frame or clears and discards an errored reception. */
ErrorStatus_t USART_Receive(USART_Instance_t instance, u16 *data, u32 timeout)
{
    u32 base;
    u32 maxClock;
    u16 cr1;
    u16 cr3;
    if ((data == NULL) || (USART_Resolve(instance, &base, &maxClock) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    cr1 = USART_REG(base, USART_CR1_OFFSET);
    cr3 = USART_REG(base, USART_CR3_OFFSET);
    if (((cr1 & (USART_UE | USART_RE)) != (USART_UE | USART_RE))
        || ((cr1 & USART_RX_IRQS) != 0U)
        || ((cr3 & (USART_DMAR | USART_EIE)) != 0U)
        || (USART_CheckMode(base, cr1, cr3) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    for (;;)
    {
        u16 status = USART_REG(base, USART_SR_OFFSET);
        /* PE may precede RXNE; RM0008 requires RXNE before clearing PE. */
        if (((status & USART_RXNE) != 0U)
            || (((status & USART_PE) == 0U) && ((status & USART_RX_ERRORS) != 0U)))
        {
            u16 received = USART_REG(base, USART_DR_OFFSET);
            if ((status & USART_RX_ERRORS) != 0U)
            {
                return ERROR_STATUS_ERROR;
            }
            *data = (u16)(received & USART_PayloadMask(cr1));
            return ERROR_STATUS_OK;
        }
        if (timeout == 0U) { return ERROR_STATUS_ERROR; }
        --timeout;
    }
}

/* Observes TC without altering flags or peripheral configuration. */
ErrorStatus_t USART_WaitTransmissionComplete(USART_Instance_t instance, u32 timeout)
{
    u32 base;
    u32 maxClock;
    if (USART_Resolve(instance, &base, &maxClock) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    return USART_Wait(base, (u16)USART_TC, timeout);
}

/* Resolves a supported status flag without reading hardware. */
static ErrorStatus_t USART_FlagMask(USART_StatusFlag_t flag, u16 *mask)
{
    switch (flag)
    {
        case USART_FLAG_PARITY_ERROR: *mask = (u16)USART_PE; break;
        case USART_FLAG_FRAMING_ERROR: *mask = (u16)USART_FE; break;
        case USART_FLAG_NOISE_ERROR: *mask = (u16)USART_NE; break;
        case USART_FLAG_OVERRUN_ERROR: *mask = (u16)USART_ORE; break;
        case USART_FLAG_IDLE: *mask = (u16)USART_IDLE; break;
        case USART_FLAG_RX_NOT_EMPTY: *mask = (u16)USART_RXNE; break;
        case USART_FLAG_TRANSMISSION_COMPLETE: *mask = (u16)USART_TC; break;
        case USART_FLAG_TX_EMPTY: *mask = (u16)USART_TXE; break;
        case USART_FLAG_CTS: *mask = (u16)USART_CTS; break;
        default: return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Returns one logical flag value without modifying output on error. */
ErrorStatus_t USART_GetFlag(USART_Instance_t instance, USART_StatusFlag_t flag, u8 *isSet)
{
    u32 base;
    u32 maxClock;
    u16 mask;
    if ((isSet == NULL)
        || (USART_Resolve(instance, &base, &maxClock) != ERROR_STATUS_OK)
        || (USART_FlagMask(flag, &mask) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    *isSet = ((USART_REG(base, USART_SR_OFFSET) & mask) != 0U) ? 1U : 0U;
    return ERROR_STATUS_OK;
}

/* Resolves the register and single enable bit for an interrupt source. */
static ErrorStatus_t USART_InterruptBit(USART_Interrupt_t interrupt, u32 *offset, u16 *mask)
{
    *offset = USART_CR1_OFFSET;
    switch (interrupt)
    {
        case USART_INTERRUPT_RXNE: *mask = (u16)USART_RXNEIE; break;
        case USART_INTERRUPT_TXE: *mask = (u16)USART_TXEIE; break;
        case USART_INTERRUPT_TC: *mask = (u16)USART_TCIE; break;
        case USART_INTERRUPT_IDLE: *mask = (u16)USART_IDLEIE; break;
        case USART_INTERRUPT_PARITY_ERROR: *mask = (u16)USART_PEIE; break;
        case USART_INTERRUPT_ERROR:
            *offset = USART_CR3_OFFSET;
            *mask = (u16)USART_EIE;
            break;
        default: return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Resolves one DMA request enable bit. */
static ErrorStatus_t USART_DMABit(USART_DMARequest_t request, u16 *mask)
{
    switch (request)
    {
        case USART_DMA_RX: *mask = (u16)USART_DMAR; break;
        case USART_DMA_TX: *mask = (u16)USART_DMAT; break;
        default: return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Enables only the selected local interrupt source. */
ErrorStatus_t USART_EnableInterrupt(USART_Instance_t instance, USART_Interrupt_t interrupt)
{
    u32 base;
    u32 maxClock;
    u32 offset;
    u16 mask;
    if ((USART_Resolve(instance, &base, &maxClock) != ERROR_STATUS_OK)
        || (USART_InterruptBit(interrupt, &offset, &mask) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    USART_REG(base, offset) = (u16)(USART_REG(base, offset) | mask);
    return ERROR_STATUS_OK;
}

/* Enables only the selected local DMA request. */
ErrorStatus_t USART_EnableDMARequest(USART_Instance_t instance, USART_DMARequest_t request)
{
    u32 base;
    u32 maxClock;
    u16 mask;
    if ((USART_Resolve(instance, &base, &maxClock) != ERROR_STATUS_OK)
        || (USART_DMABit(request, &mask) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    USART_REG(base, USART_CR3_OFFSET) =
        (u16)(USART_REG(base, USART_CR3_OFFSET) | mask);
    return ERROR_STATUS_OK;
}

/* Disables only the selected local interrupt source. */
ErrorStatus_t USART_DisableInterrupt(USART_Instance_t instance, USART_Interrupt_t interrupt)
{
    u32 base;
    u32 maxClock;
    u32 offset;
    u16 mask;
    if ((USART_Resolve(instance, &base, &maxClock) != ERROR_STATUS_OK)
        || (USART_InterruptBit(interrupt, &offset, &mask) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    USART_REG(base, offset) = (u16)(USART_REG(base, offset) & (u16)~mask);
    return ERROR_STATUS_OK;
}

/* Disables only the selected local DMA request. */
ErrorStatus_t USART_DisableDMARequest(USART_Instance_t instance, USART_DMARequest_t request)
{
    u32 base;
    u32 maxClock;
    u16 mask;
    if ((USART_Resolve(instance, &base, &maxClock) != ERROR_STATUS_OK)
        || (USART_DMABit(request, &mask) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }
    USART_REG(base, USART_CR3_OFFSET) =
        (u16)(USART_REG(base, USART_CR3_OFFSET) & (u16)~mask);
    return ERROR_STATUS_OK;
}
