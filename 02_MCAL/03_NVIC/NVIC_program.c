/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< NVIC_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : NVIC_PROGRAM
 * Description : Description : This file contains the Cortex-M3 interrupt controller functionality.
 */

#include "NVIC_interface.h"
#include "NVIC_private.h"
#include "NVIC_config.h"

/* Accepts only interrupt sources present on STM32F103C6. */
static ErrorStatus_t NVIC_ValidateIRQ(NVIC_IRQ_t irq)
{
    switch (irq)
    {
        case NVIC_IRQ_WWDG:
        case NVIC_IRQ_PVD:
        case NVIC_IRQ_TAMPER:
        case NVIC_IRQ_RTC:
        case NVIC_IRQ_FLASH:
        case NVIC_IRQ_RCC:
        case NVIC_IRQ_EXTI0:
        case NVIC_IRQ_EXTI1:
        case NVIC_IRQ_EXTI2:
        case NVIC_IRQ_EXTI3:
        case NVIC_IRQ_EXTI4:
        case NVIC_IRQ_DMA1_CHANNEL1:
        case NVIC_IRQ_DMA1_CHANNEL2:
        case NVIC_IRQ_DMA1_CHANNEL3:
        case NVIC_IRQ_DMA1_CHANNEL4:
        case NVIC_IRQ_DMA1_CHANNEL5:
        case NVIC_IRQ_DMA1_CHANNEL6:
        case NVIC_IRQ_DMA1_CHANNEL7:
        case NVIC_IRQ_ADC1_2:
        case NVIC_IRQ_USB_HP_CAN_TX:
        case NVIC_IRQ_USB_LP_CAN_RX0:
        case NVIC_IRQ_CAN_RX1:
        case NVIC_IRQ_CAN_SCE:
        case NVIC_IRQ_EXTI9_5:
        case NVIC_IRQ_TIM1_BRK:
        case NVIC_IRQ_TIM1_UP:
        case NVIC_IRQ_TIM1_TRG_COM:
        case NVIC_IRQ_TIM1_CC:
        case NVIC_IRQ_TIM2:
        case NVIC_IRQ_TIM3:
        case NVIC_IRQ_I2C1_EV:
        case NVIC_IRQ_I2C1_ER:
        case NVIC_IRQ_SPI1:
        case NVIC_IRQ_USART1:
        case NVIC_IRQ_USART2:
        case NVIC_IRQ_EXTI15_10:
        case NVIC_IRQ_RTC_ALARM:
        case NVIC_IRQ_USB_WAKEUP:
            return ERROR_STATUS_OK;
        default:
            return ERROR_STATUS_ERROR;
    }
}

/* Resolves the word register containing a validated IRQ bit. */
static uintptr_t NVIC_GetBankAddress(u32 offset, NVIC_IRQ_t irq)
{
    return (uintptr_t)NVIC_BASE_ADDRESS + offset +
           (((u32)irq / NVIC_IRQ_BANK_BITS) * NVIC_REGISTER_BYTES);
}

/* Forms the bit mask for a validated IRQ within its register bank. */
static u32 NVIC_GetIRQMask(NVIC_IRQ_t irq)
{
    return UINT32_C(1) << ((u32)irq % NVIC_IRQ_BANK_BITS);
}

/* Enables one IRQ using write-one-to-set semantics. */
ErrorStatus_t NVIC_EnableIRQ(NVIC_IRQ_t irq)
{
    if (NVIC_ValidateIRQ(irq) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    NVIC_REGISTER32(NVIC_GetBankAddress(NVIC_ISER_OFFSET, irq)) = NVIC_GetIRQMask(irq);
    return ERROR_STATUS_OK;
}

/* Disables one IRQ using write-one-to-clear semantics. */
ErrorStatus_t NVIC_DisableIRQ(NVIC_IRQ_t irq)
{
    if (NVIC_ValidateIRQ(irq) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    NVIC_REGISTER32(NVIC_GetBankAddress(NVIC_ICER_OFFSET, irq)) = NVIC_GetIRQMask(irq);
    return ERROR_STATUS_OK;
}

/* Sets one IRQ pending without changing other IRQs. */
ErrorStatus_t NVIC_SetPendingIRQ(NVIC_IRQ_t irq)
{
    if (NVIC_ValidateIRQ(irq) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    NVIC_REGISTER32(NVIC_GetBankAddress(NVIC_ISPR_OFFSET, irq)) = NVIC_GetIRQMask(irq);
    return ERROR_STATUS_OK;
}

/* Clears one IRQ pending without changing other IRQs. */
ErrorStatus_t NVIC_ClearPendingIRQ(NVIC_IRQ_t irq)
{
    if (NVIC_ValidateIRQ(irq) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }

    NVIC_REGISTER32(NVIC_GetBankAddress(NVIC_ICPR_OFFSET, irq)) = NVIC_GetIRQMask(irq);
    return ERROR_STATUS_OK;
}

/* Reads the selected IRQ pending bit. */
ErrorStatus_t NVIC_GetPendingIRQ(NVIC_IRQ_t irq, u8 *isPending)
{
    u32 state;

    if ((isPending == NULL) || (NVIC_ValidateIRQ(irq) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    state = NVIC_REGISTER32(NVIC_GetBankAddress(NVIC_ISPR_OFFSET, irq));
    *isPending = (u8)((state & NVIC_GetIRQMask(irq)) != 0U);
    return ERROR_STATUS_OK;
}

/* Reads the selected IRQ active bit. */
ErrorStatus_t NVIC_GetActiveIRQ(NVIC_IRQ_t irq, u8 *isActive)
{
    u32 state;

    if ((isActive == NULL) || (NVIC_ValidateIRQ(irq) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    state = NVIC_REGISTER32(NVIC_GetBankAddress(NVIC_IABR_OFFSET, irq));
    *isActive = (u8)((state & NVIC_GetIRQMask(irq)) != 0U);
    return ERROR_STATUS_OK;
}

/* Writes only the target byte's implemented priority bits. */
ErrorStatus_t NVIC_SetPriority(NVIC_IRQ_t irq, u8 priority)
{
    if ((priority > NVIC_PRIORITY_MAX) || (NVIC_ValidateIRQ(irq) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    NVIC_REGISTER8((uintptr_t)NVIC_BASE_ADDRESS + NVIC_IPR_OFFSET + (u32)irq) =
        (u8)((u32)priority << NVIC_PRIORITY_SHIFT);
    return ERROR_STATUS_OK;
}

/* Decodes the target IRQ priority byte into a logical priority. */
ErrorStatus_t NVIC_GetPriority(NVIC_IRQ_t irq, u8 *priority)
{
    u8 encoded_priority;

    if ((priority == NULL) || (NVIC_ValidateIRQ(irq) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    encoded_priority = NVIC_REGISTER8((uintptr_t)NVIC_BASE_ADDRESS + NVIC_IPR_OFFSET + (u32)irq);
    *priority = (u8)((encoded_priority >> NVIC_PRIORITY_SHIFT) & NVIC_PRIORITY_MAX);
    return ERROR_STATUS_OK;
}

/* Selects a verified priority split using the required AIRCR write key. */
ErrorStatus_t NVIC_SetPriorityGrouping(NVIC_PriorityGrouping_t grouping)
{
    u32 encoding;
    u32 aircr;

    switch (grouping)
    {
        case NVIC_GROUP_4_PREEMPT_0_SUB: encoding = NVIC_GROUP_ENCODING_4_0; break;
        case NVIC_GROUP_3_PREEMPT_1_SUB: encoding = NVIC_GROUP_ENCODING_3_1; break;
        case NVIC_GROUP_2_PREEMPT_2_SUB: encoding = NVIC_GROUP_ENCODING_2_2; break;
        case NVIC_GROUP_1_PREEMPT_3_SUB: encoding = NVIC_GROUP_ENCODING_1_3; break;
        case NVIC_GROUP_0_PREEMPT_4_SUB: encoding = NVIC_GROUP_ENCODING_0_4; break;
        default: return ERROR_STATUS_ERROR;
    }

    aircr = NVIC_REGISTER32(NVIC_SCB_BASE_ADDRESS + NVIC_AIRCR_OFFSET);
    /* Replace the read key and never replay reset/debug command bits. */
    aircr &= ~(NVIC_AIRCR_KEY_MASK | NVIC_AIRCR_GROUP_MASK | NVIC_AIRCR_COMMAND_MASK);
    aircr |= NVIC_AIRCR_WRITE_KEY | (encoding << NVIC_AIRCR_GROUP_SHIFT);
    NVIC_REGISTER32(NVIC_SCB_BASE_ADDRESS + NVIC_AIRCR_OFFSET) = aircr;
    return ERROR_STATUS_OK;
}

/* Decodes the effective grouping of the four implemented priority bits. */
ErrorStatus_t NVIC_GetPriorityGrouping(NVIC_PriorityGrouping_t *grouping)
{
    u32 encoding;
    NVIC_PriorityGrouping_t decoded_grouping;

    if (grouping == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    encoding = (NVIC_REGISTER32(NVIC_SCB_BASE_ADDRESS + NVIC_AIRCR_OFFSET) &
                NVIC_AIRCR_GROUP_MASK) >> NVIC_AIRCR_GROUP_SHIFT;

    /* PRIGROUP 0..3 all place the four implemented bits in preemption priority. */
    if (encoding <= NVIC_GROUP_ENCODING_4_0)
    {
        decoded_grouping = NVIC_GROUP_4_PREEMPT_0_SUB;
    }
    else
    {
        switch (encoding)
        {
            case NVIC_GROUP_ENCODING_3_1: decoded_grouping = NVIC_GROUP_3_PREEMPT_1_SUB; break;
            case NVIC_GROUP_ENCODING_2_2: decoded_grouping = NVIC_GROUP_2_PREEMPT_2_SUB; break;
            case NVIC_GROUP_ENCODING_1_3: decoded_grouping = NVIC_GROUP_1_PREEMPT_3_SUB; break;
            case NVIC_GROUP_ENCODING_0_4: decoded_grouping = NVIC_GROUP_0_PREEMPT_4_SUB; break;
            default: return ERROR_STATUS_ERROR;
        }
    }

    *grouping = decoded_grouping;
    return ERROR_STATUS_OK;
}
