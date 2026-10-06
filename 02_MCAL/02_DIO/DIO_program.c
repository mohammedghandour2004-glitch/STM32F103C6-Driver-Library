/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< DIO_CONFIG.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : DIO_CONFIG
 * Description : This file contains the GPIO digital input/output control functionality.
 */

#include "DIO_interface.h"
#include "DIO_config.h"
#include "DIO_private.h"

/* Resolves a validated logical port to its GPIO base address. */
static ErrorStatus_t DIO_GetPortBase(DIO_Port_t port, u32 *base_address)
{
    u32 base;

    if (base_address == NULL)
    {
        return ERROR_STATUS_ERROR;
    }

    switch (port)
    {
        case DIO_PORT_A: base = DIO_GPIOA_BASE_ADDRESS; break;
        case DIO_PORT_B: base = DIO_GPIOB_BASE_ADDRESS; break;
        case DIO_PORT_C: base = DIO_GPIOC_BASE_ADDRESS; break;
        case DIO_PORT_D: base = DIO_GPIOD_BASE_ADDRESS; break;
        default: return ERROR_STATUS_ERROR;
    }

    *base_address = base;
    return ERROR_STATUS_OK;
}

/* Checks whether a logical pin identifier is valid. */
static ErrorStatus_t DIO_ValidatePin(DIO_Pin_t pin)
{
    return (pin < DIO_PIN_COUNT) ? ERROR_STATUS_OK : ERROR_STATUS_ERROR;
}

/* Checks whether a logical pin level is valid. */
static ErrorStatus_t DIO_ValidateLevel(DIO_Level_t level)
{
    return ((level == DIO_LEVEL_LOW) || (level == DIO_LEVEL_HIGH)) ?
           ERROR_STATUS_OK : ERROR_STATUS_ERROR;
}

/* Translates one public pin configuration into an STM32F1 CR field. */
static ErrorStatus_t DIO_GetConfigurationField(const DIO_PinConfig_t *config, u32 *field)
{
    u32 cnf;
    u32 mode;

    if ((config == NULL) || (field == NULL) ||
        (DIO_ValidateLevel(config->initial_level) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    switch (config->mode)
    {
        case DIO_MODE_INPUT_ANALOG:
            cnf = DIO_INPUT_ANALOG_CNF;
            mode = DIO_INPUT_MODE_BITS;
            break;

        case DIO_MODE_INPUT_FLOATING:
            cnf = DIO_INPUT_FLOATING_CNF;
            mode = DIO_INPUT_MODE_BITS;
            break;

        case DIO_MODE_INPUT_PULL_UP:
        case DIO_MODE_INPUT_PULL_DOWN:
            cnf = DIO_INPUT_PULL_CNF;
            mode = DIO_INPUT_MODE_BITS;
            break;

        case DIO_MODE_OUTPUT_PUSH_PULL:
            cnf = DIO_OUTPUT_PUSH_PULL_CNF;
            break;

        case DIO_MODE_OUTPUT_OPEN_DRAIN:
            cnf = DIO_OUTPUT_OPEN_DRAIN_CNF;
            break;

        case DIO_MODE_ALTERNATE_PUSH_PULL:
            cnf = DIO_ALTERNATE_PUSH_PULL_CNF;
            break;

        case DIO_MODE_ALTERNATE_OPEN_DRAIN:
            cnf = DIO_ALTERNATE_OPEN_DRAIN_CNF;
            break;

        default:
            return ERROR_STATUS_ERROR;
    }

    if (config->mode <= DIO_MODE_INPUT_PULL_DOWN)
    {
        if (config->speed != DIO_SPEED_NOT_APPLICABLE)
        {
            return ERROR_STATUS_ERROR;
        }
    }
    else
    {
        switch (config->speed)
        {
            case DIO_SPEED_2MHZ: mode = DIO_SPEED_2MHZ_BITS; break;
            case DIO_SPEED_10MHZ: mode = DIO_SPEED_10MHZ_BITS; break;
            case DIO_SPEED_50MHZ: mode = DIO_SPEED_50MHZ_BITS; break;
            default: return ERROR_STATUS_ERROR;
        }
    }

    *field = (cnf << DIO_CNF_SHIFT) | mode;
    return ERROR_STATUS_OK;
}

/* Atomically updates one GPIO output latch bit. */
static void DIO_WriteLatch(u32 base_address, DIO_Pin_t pin, DIO_Level_t level)
{
    u32 pin_mask;

    pin_mask = 1UL << (u32)pin;
    if (level == DIO_LEVEL_HIGH)
    {
        DIO_REGISTER(base_address, DIO_BSRR_OFFSET) = pin_mask;
    }
    else
    {
        DIO_REGISTER(base_address, DIO_BRR_OFFSET) = pin_mask;
    }
}

/* Configures one STM32F1 GPIO pin after all public inputs are validated. */
ErrorStatus_t DIO_ConfigPin(DIO_Port_t port, DIO_Pin_t pin, const DIO_PinConfig_t *config)
{
    u32 base_address;
    u32 field;
    u32 field_shift;
    u32 lock_register;
    u32 register_value;
    u32 register_offset;

    if ((DIO_GetPortBase(port, &base_address) != ERROR_STATUS_OK) ||
        (DIO_ValidatePin(pin) != ERROR_STATUS_OK) ||
        (DIO_GetConfigurationField(config, &field) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    lock_register = DIO_REGISTER(base_address, DIO_LCKR_OFFSET);
    if (((lock_register & DIO_LCKR_KEY_MASK) != 0U) &&
        ((lock_register & (1UL << (u32)pin)) != 0U))
    {
        return ERROR_STATUS_ERROR;
    }

    /* In input-pull mode, ODR selects the resistor direction. */
    if (config->mode == DIO_MODE_INPUT_PULL_UP)
    {
        DIO_WriteLatch(base_address, pin, DIO_LEVEL_HIGH);
    }
    else if (config->mode == DIO_MODE_INPUT_PULL_DOWN)
    {
        DIO_WriteLatch(base_address, pin, DIO_LEVEL_LOW);
    }
    else if (config->mode >= DIO_MODE_OUTPUT_PUSH_PULL)
    {
        DIO_WriteLatch(base_address, pin, config->initial_level);
    }

    if (pin < DIO_PIN_8)
    {
        register_offset = DIO_CRL_OFFSET;
        field_shift = (u32)pin * 4U;
    }
    else
    {
        register_offset = DIO_CRH_OFFSET;
        field_shift = ((u32)pin - 8U) * 4U;
    }

    register_value = DIO_REGISTER(base_address, register_offset);
    register_value &= ~(DIO_CONFIGURATION_FIELD_MASK << field_shift);
    register_value |= (field << field_shift);
    DIO_REGISTER(base_address, register_offset) = register_value;

    return ERROR_STATUS_OK;
}

/* Atomically writes one GPIO output latch bit. */
ErrorStatus_t DIO_WritePin(DIO_Port_t port, DIO_Pin_t pin, DIO_Level_t level)
{
    u32 base_address;

    if ((DIO_GetPortBase(port, &base_address) != ERROR_STATUS_OK) ||
        (DIO_ValidatePin(pin) != ERROR_STATUS_OK) ||
        (DIO_ValidateLevel(level) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    DIO_WriteLatch(base_address, pin, level);
    return ERROR_STATUS_OK;
}

/* Reads one GPIO input data bit. */
ErrorStatus_t DIO_ReadPin(DIO_Port_t port, DIO_Pin_t pin, DIO_Level_t *level)
{
    u32 base_address;
    u32 input_data;

    if ((level == NULL) || (DIO_GetPortBase(port, &base_address) != ERROR_STATUS_OK) ||
        (DIO_ValidatePin(pin) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    input_data = DIO_REGISTER(base_address, DIO_IDR_OFFSET);
    *level = ((input_data & (1UL << (u32)pin)) != 0U) ? DIO_LEVEL_HIGH : DIO_LEVEL_LOW;
    return ERROR_STATUS_OK;
}

/* Reads one GPIO output latch bit. */
ErrorStatus_t DIO_ReadOutputLatch(DIO_Port_t port, DIO_Pin_t pin, DIO_Level_t *level)
{
    u32 base_address;
    u32 output_data;

    if ((level == NULL) || (DIO_GetPortBase(port, &base_address) != ERROR_STATUS_OK) ||
        (DIO_ValidatePin(pin) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    output_data = DIO_REGISTER(base_address, DIO_ODR_OFFSET);
    *level = ((output_data & (1UL << (u32)pin)) != 0U) ? DIO_LEVEL_HIGH : DIO_LEVEL_LOW;
    return ERROR_STATUS_OK;
}

/* Toggles one GPIO latch bit; this is not atomic against another writer. */
ErrorStatus_t DIO_TogglePin(DIO_Port_t port, DIO_Pin_t pin)
{
    u32 base_address;
    u32 output_data;

    if ((DIO_GetPortBase(port, &base_address) != ERROR_STATUS_OK) ||
        (DIO_ValidatePin(pin) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    output_data = DIO_REGISTER(base_address, DIO_ODR_OFFSET);
    if ((output_data & (1UL << (u32)pin)) == 0U)
    {
        DIO_WriteLatch(base_address, pin, DIO_LEVEL_HIGH);
    }
    else
    {
        DIO_WriteLatch(base_address, pin, DIO_LEVEL_LOW);
    }

    return ERROR_STATUS_OK;
}

/* Locks one GPIO configuration field until the next reset. */
ErrorStatus_t DIO_LockPin(DIO_Port_t port, DIO_Pin_t pin)
{
    u32 base_address;
    u32 lock_mask;
    volatile u32 lock_readback;

    if ((DIO_GetPortBase(port, &base_address) != ERROR_STATUS_OK) ||
        (DIO_ValidatePin(pin) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    if ((DIO_REGISTER(base_address, DIO_LCKR_OFFSET) & DIO_LCKR_KEY_MASK) != 0U)
    {
        return ERROR_STATUS_ERROR;
    }

    lock_mask = (DIO_REGISTER(base_address, DIO_LCKR_OFFSET) & DIO_LCKR_PIN_MASK) |
                (1UL << (u32)pin);
    DIO_REGISTER(base_address, DIO_LCKR_OFFSET) = DIO_LCKR_KEY_MASK | lock_mask;
    DIO_REGISTER(base_address, DIO_LCKR_OFFSET) = lock_mask;
    DIO_REGISTER(base_address, DIO_LCKR_OFFSET) = DIO_LCKR_KEY_MASK | lock_mask;
    lock_readback = DIO_REGISTER(base_address, DIO_LCKR_OFFSET);
    lock_readback = DIO_REGISTER(base_address, DIO_LCKR_OFFSET);

    return ((lock_readback & DIO_LCKR_KEY_MASK) != 0U) ? ERROR_STATUS_OK : ERROR_STATUS_ERROR;
}

/* Reports whether one GPIO configuration field is hardware locked. */
ErrorStatus_t DIO_IsPinLocked(DIO_Port_t port, DIO_Pin_t pin, u8 *locked)
{
    u32 base_address;
    u32 lock_register;

    if ((locked == NULL) || (DIO_GetPortBase(port, &base_address) != ERROR_STATUS_OK) ||
        (DIO_ValidatePin(pin) != ERROR_STATUS_OK))
    {
        return ERROR_STATUS_ERROR;
    }

    lock_register = DIO_REGISTER(base_address, DIO_LCKR_OFFSET);
    *locked = (u8)(((lock_register & DIO_LCKR_KEY_MASK) != 0U) &&
                   ((lock_register & (1UL << (u32)pin)) != 0U));
    return ERROR_STATUS_OK;
}
