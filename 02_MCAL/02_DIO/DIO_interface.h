/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< DIO_CONFIG.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : DIO_CONFIG
 * Description : Description : This file contains the GPIO digital input/output control functionality.
 */

#ifndef DIO_INTERFACE_H
#define DIO_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

typedef enum
{
    DIO_PORT_A,
    DIO_PORT_B,
    DIO_PORT_C,
    DIO_PORT_D,
    DIO_PORT_COUNT
} DIO_Port_t;

typedef enum
{
    DIO_PIN_0,
    DIO_PIN_1,
    DIO_PIN_2,
    DIO_PIN_3,
    DIO_PIN_4,
    DIO_PIN_5,
    DIO_PIN_6,
    DIO_PIN_7,
    DIO_PIN_8,
    DIO_PIN_9,
    DIO_PIN_10,
    DIO_PIN_11,
    DIO_PIN_12,
    DIO_PIN_13,
    DIO_PIN_14,
    DIO_PIN_15,
    DIO_PIN_COUNT
} DIO_Pin_t;

typedef enum
{
    DIO_MODE_INPUT_ANALOG,
    DIO_MODE_INPUT_FLOATING,
    DIO_MODE_INPUT_PULL_UP,
    DIO_MODE_INPUT_PULL_DOWN,
    DIO_MODE_OUTPUT_PUSH_PULL,
    DIO_MODE_OUTPUT_OPEN_DRAIN,
    DIO_MODE_ALTERNATE_PUSH_PULL,
    DIO_MODE_ALTERNATE_OPEN_DRAIN
} DIO_Mode_t;

typedef enum
{
    DIO_SPEED_NOT_APPLICABLE,
    DIO_SPEED_2MHZ,
    DIO_SPEED_10MHZ,
    DIO_SPEED_50MHZ
} DIO_Speed_t;

typedef enum
{
    DIO_LEVEL_LOW,
    DIO_LEVEL_HIGH
} DIO_Level_t;

typedef struct
{
    DIO_Mode_t mode;
    DIO_Speed_t speed;
    DIO_Level_t initial_level;
} DIO_PinConfig_t;

/* The application verifies physical bonding against its selected package pinout. */

/* Configures one STM32F1 GPIO register-level pin. */
ErrorStatus_t DIO_ConfigPin(DIO_Port_t port, DIO_Pin_t pin, const DIO_PinConfig_t *config);

/* Atomically writes one GPIO output latch bit. */
ErrorStatus_t DIO_WritePin(DIO_Port_t port, DIO_Pin_t pin, DIO_Level_t level);

/* Reads one GPIO input data bit. */
ErrorStatus_t DIO_ReadPin(DIO_Port_t port, DIO_Pin_t pin, DIO_Level_t *level);

/* Reads one GPIO output latch bit. */
ErrorStatus_t DIO_ReadOutputLatch(DIO_Port_t port, DIO_Pin_t pin, DIO_Level_t *level);

/* Toggles one GPIO output latch bit. */
ErrorStatus_t DIO_TogglePin(DIO_Port_t port, DIO_Pin_t pin);

/* Locks one GPIO configuration field until reset. */
ErrorStatus_t DIO_LockPin(DIO_Port_t port, DIO_Pin_t pin);

/* Reports whether one GPIO configuration field is locked. */
ErrorStatus_t DIO_IsPinLocked(DIO_Port_t port, DIO_Pin_t pin, u8 *locked);

#endif /* DIO_INTERFACE_H */
