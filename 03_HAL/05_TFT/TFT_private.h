/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< TFT_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : TFT_PRIVATE
 * Description : This file contains the private definitions for the ST7735S TFT display control functionality.
 */

#ifndef TFT_PRIVATE_H
#define TFT_PRIVATE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../02_MCAL/02_DIO/DIO_interface.h"

#define TFT_WIDTH 128U
#define TFT_HEIGHT 160U
#define TFT_COLUMN_OFFSET 2U
#define TFT_ROW_OFFSET 1U
#define TFT_PORTRAIT_MADCTL 0x00U
#define TFT_RGB565_COLMOD 0x05U
#define TFT_RESET_WAIT_MS 100U
#define TFT_AFTER_TABLE_MS 200U
#define TFT_AFTER_SLPOUT_MS 120U

#define TFT_CMD_SLPOUT 0x11U
#define TFT_CMD_DISPON 0x29U
#define TFT_CMD_CASET 0x2AU
#define TFT_CMD_RASET 0x2BU
#define TFT_CMD_RAMWR 0x2CU
#define TFT_CMD_MADCTL 0x36U
#define TFT_CMD_COLMOD 0x3AU
#define TFT_CMD_FRMCTR1 0xB1U
#define TFT_CMD_FRMCTR2 0xB2U
#define TFT_CMD_FRMCTR3 0xB3U
#define TFT_CMD_INVCTR 0xB4U
#define TFT_CMD_PWCTR1 0xC0U
#define TFT_CMD_PWCTR2 0xC1U
#define TFT_CMD_PWCTR3 0xC2U
#define TFT_CMD_PWCTR4 0xC3U
#define TFT_CMD_PWCTR5 0xC4U
#define TFT_CMD_VMCTR1 0xC5U
#define TFT_CMD_GAMCTRP1 0xE0U
#define TFT_CMD_GAMCTRN1 0xE1U
#define TFT_CMD_MODULE_F0 0xF0U
#define TFT_CMD_MODULE_F6 0xF6U

typedef struct
{
    DIO_Port_t port;
    DIO_Pin_t pin;
} TFT_Pin_t;

typedef enum
{
    TFT_SIGNAL_CS,
    TFT_SIGNAL_DC,
    TFT_SIGNAL_RST,
    TFT_SIGNAL_COUNT
} TFT_Signal_t;

typedef enum
{
    TFT_INIT_STATE_IDLE,
    TFT_INIT_STATE_WAIT_RST_HIGH_1,
    TFT_INIT_STATE_WAIT_RST_LOW,
    TFT_INIT_STATE_WAIT_RST_HIGH_2,
    TFT_INIT_STATE_SEND_TABLE,
    TFT_INIT_STATE_WAIT_AFTER_TABLE,
    TFT_INIT_STATE_SEND_SLPOUT,
    TFT_INIT_STATE_WAIT_AFTER_SLPOUT,
    TFT_INIT_STATE_SEND_DISPON,
    TFT_INIT_STATE_READY
} TFT_InitState_t;

typedef struct
{
    u8 command;
    u8 count;
    u8 parameters[16];
} TFT_InitEntry_t;

#endif
