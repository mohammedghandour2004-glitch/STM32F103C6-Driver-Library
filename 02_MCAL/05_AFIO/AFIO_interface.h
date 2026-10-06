/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< AFIO_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 02_MCAL
 * Component   : AFIO_INTERFACE
 * Description : This file contains alternate-function routing and remapping functionality.
 */

#ifndef AFIO_INTERFACE_H
#define AFIO_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

typedef enum
{
    AFIO_PORT_A,
    AFIO_PORT_B,
    AFIO_PORT_C,
    AFIO_PORT_D
} AFIO_Port_t;

typedef enum
{
    AFIO_EXTI_LINE_0 = 0,
    AFIO_EXTI_LINE_1 = 1,
    AFIO_EXTI_LINE_2 = 2,
    AFIO_EXTI_LINE_3 = 3,
    AFIO_EXTI_LINE_4 = 4,
    AFIO_EXTI_LINE_5 = 5,
    AFIO_EXTI_LINE_6 = 6,
    AFIO_EXTI_LINE_7 = 7,
    AFIO_EXTI_LINE_8 = 8,
    AFIO_EXTI_LINE_9 = 9,
    AFIO_EXTI_LINE_10 = 10,
    AFIO_EXTI_LINE_11 = 11,
    AFIO_EXTI_LINE_12 = 12,
    AFIO_EXTI_LINE_13 = 13,
    AFIO_EXTI_LINE_14 = 14,
    AFIO_EXTI_LINE_15 = 15
} AFIO_EXTILine_t;

typedef enum
{
    AFIO_REMAP_SPI1,
    AFIO_REMAP_USART1,
    AFIO_REMAP_USART2
} AFIO_RemapPeripheral_t;

typedef enum
{
    AFIO_DEBUG_FULL_JTAG_SWD,
    AFIO_DEBUG_JTAG_SWD_NO_NJTRST,
    AFIO_DEBUG_SWD_ONLY,
    AFIO_DEBUG_DISABLED
} AFIO_DebugConfig_t;

/* Caller enables AFIO clock, serializes calls, and owns pin availability/modes. */
/* SWJ shadow starts at full JTAG/SWD reset state. After a bootloader or AFIO-only
   reset, call SetDebugConfig before SetRemap; all SWJ writes must use this driver. */

/* Routes GPIO port input to one EXTI line, preserving neighboring routes. */
ErrorStatus_t AFIO_SetEXTISource(AFIO_EXTILine_t line, AFIO_Port_t port);

/* SPI1: NSS/SCK/MISO/MOSI PA4/5/6/7 -> PA15/PB3/PB4/PB5 (release debug ownership).
   USART1: TX/RX PA9/10 -> PB6/7.
   USART2: CTS/RTS/TX/RX/CK PA0..4 -> PD3..7; RM0008 limits destinations to
   100/144-pin packages, so enabling this bit does not provide usable C6 pins. */
/* Enables or disables the selected peripheral's register-level remap. */
ErrorStatus_t AFIO_SetRemap(AFIO_RemapPeripheral_t peripheral, u8 enabled);

/* Selects debug ownership; disabling SWD removes debugger access through SWD. */
ErrorStatus_t AFIO_SetDebugConfig(AFIO_DebugConfig_t config);

/* Routes Cortex EVENTOUT to a port/pin without configuring its GPIO mode. */
ErrorStatus_t AFIO_EnableEventOutput(AFIO_Port_t port, u8 pin);

/* Disables EVENTOUT while retaining its selected port and pin. */
ErrorStatus_t AFIO_DisableEventOutput(void);

#endif /* AFIO_INTERFACE_H */
