/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< AFIO_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : AFIO_PROGRAM
 * Description : This file contains alternate-function routing and remapping functionality.
 */

#include "AFIO_interface.h"
#include "AFIO_private.h"
#include "AFIO_config.h"

/* Only SWJ_CFG needs a shadow: hardware returns undefined data for this field. */
static u8 AFIO_SwjShadow = AFIO_SWJ_FULL_ENCODING;

/* Validates a logical port and translates its hardware selection code. */
static ErrorStatus_t AFIO_EncodePort(AFIO_Port_t port, u32 *encoding)
{
    switch (port)
    {
        case AFIO_PORT_A: *encoding = AFIO_PORT_A_ENCODING; break;
        case AFIO_PORT_B: *encoding = AFIO_PORT_B_ENCODING; break;
        case AFIO_PORT_C: *encoding = AFIO_PORT_C_ENCODING; break;
        case AFIO_PORT_D: *encoding = AFIO_PORT_D_ENCODING; break;
        default: return ERROR_STATUS_ERROR;
    }

    return ERROR_STATUS_OK;
}

/* Updates readable MAPR fields while replacing undefined SWJ readback. */
static void AFIO_UpdateMapr(u32 mask, u32 value, u8 swj_encoding)
{
    u32 mapr = AFIO_REGISTER(AFIO_MAPR_OFFSET);

    mapr &= ~(mask | AFIO_MAPR_SWJ_MASK);
    mapr |= value | ((u32)swj_encoding << AFIO_MAPR_SWJ_SHIFT);
    AFIO_REGISTER(AFIO_MAPR_OFFSET) = mapr;
    AFIO_SwjShadow = swj_encoding;
}

/* Selects the GPIO port feeding one EXTI line. */
ErrorStatus_t AFIO_SetEXTISource(AFIO_EXTILine_t line, AFIO_Port_t port)
{
    u32 encoding;
    u32 offset;
    u32 shift;
    u32 exticr;

    if (((u32)line > AFIO_EXTI_LAST_LINE) ||
        (AFIO_EncodePort(port, &encoding) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    offset = AFIO_EXTICR1_OFFSET + (((u32)line / AFIO_EXTICR_FIELDS) * AFIO_REGISTER_BYTES);
    shift = ((u32)line % AFIO_EXTICR_FIELDS) * AFIO_EXTICR_FIELD_BITS;
    exticr = AFIO_REGISTER(offset);
    exticr &= ~(AFIO_EXTICR_FIELD_MASK << shift);
    exticr |= encoding << shift;
    AFIO_REGISTER(offset) = exticr;
    return ERROR_STATUS_OK;
}

/* Changes one peripheral remap without changing debug ownership. */
ErrorStatus_t AFIO_SetRemap(AFIO_RemapPeripheral_t peripheral, u8 enabled)
{
    u32 mask;

    if (enabled > 1U)
    {
        return ERROR_STATUS_ERROR;
    }

    switch (peripheral)
    {
        case AFIO_REMAP_SPI1: mask = AFIO_MAPR_SPI1_MASK; break;
        case AFIO_REMAP_USART1: mask = AFIO_MAPR_USART1_MASK; break;
        case AFIO_REMAP_USART2: mask = AFIO_MAPR_USART2_MASK; break;
        default: return ERROR_STATUS_ERROR;
    }

    AFIO_UpdateMapr(mask, (enabled != 0U) ? mask : 0U, AFIO_SwjShadow);
    return ERROR_STATUS_OK;
}

/* Changes debug ownership while retaining all readable remap fields. */
ErrorStatus_t AFIO_SetDebugConfig(AFIO_DebugConfig_t config)
{
    u8 encoding;

    switch (config)
    {
        case AFIO_DEBUG_FULL_JTAG_SWD: encoding = AFIO_SWJ_FULL_ENCODING; break;
        case AFIO_DEBUG_JTAG_SWD_NO_NJTRST: encoding = AFIO_SWJ_NO_NJTRST_ENCODING; break;
        case AFIO_DEBUG_SWD_ONLY: encoding = AFIO_SWJ_SWD_ENCODING; break;
        case AFIO_DEBUG_DISABLED: encoding = AFIO_SWJ_DISABLED_ENCODING; break;
        default: return ERROR_STATUS_ERROR;
    }

    AFIO_UpdateMapr(0U, 0U, encoding);
    return ERROR_STATUS_OK;
}

/* Selects and enables the Cortex EVENTOUT destination. */
ErrorStatus_t AFIO_EnableEventOutput(AFIO_Port_t port, u8 pin)
{
    u32 encoding;
    u32 evcr;

    if ((pin > AFIO_EVENT_LAST_PIN) ||
        (AFIO_EncodePort(port, &encoding) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    evcr = AFIO_REGISTER(AFIO_EVCR_OFFSET);
    evcr &= ~(AFIO_EVCR_PORT_MASK | AFIO_EVCR_PIN_MASK | AFIO_EVCR_EVOE_MASK);
    evcr |= (encoding << AFIO_EVCR_PORT_SHIFT) | (u32)pin | AFIO_EVCR_EVOE_MASK;
    AFIO_REGISTER(AFIO_EVCR_OFFSET) = evcr;
    return ERROR_STATUS_OK;
}

/* Disables EVENTOUT without changing its destination. */
ErrorStatus_t AFIO_DisableEventOutput(void)
{
    AFIO_REGISTER(AFIO_EVCR_OFFSET) &= ~AFIO_EVCR_EVOE_MASK;
    return ERROR_STATUS_OK;
}
