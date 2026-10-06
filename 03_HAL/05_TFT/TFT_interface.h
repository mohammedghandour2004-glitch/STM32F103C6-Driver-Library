/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< TFT_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : TFT_INTERFACE
 * Description : This file contains the ST7735S TFT display control functionality.
 */

#ifndef TFT_INTERFACE_H
#define TFT_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

/* Waveshare SKU 13892 / ST7735S: fixed 128x160 portrait, top-left origin.
 * Colors are RGB565: red[15:11], green[10:5], blue[4:0].
 * Caller owns clocks, power stabilization, physical pin availability, MOSI/SCK
 * configuration and backlight. Initialization does not clear display RAM.
 * Before InitStart, configure and enable SPI1: master, CPOL low, first edge,
 * MSB first, 8-bit, HALF_DUPLEX_TX, software NSS internally HIGH, hardware
 * NSS output disabled, all SPI interrupt sources and DMA requests disabled.
 * Select SCK and board timing within the module/controller electrical limits.
 * The SPI configuration cannot be fully verified through its public API.
 * Serialize all TFT calls and all users of the shared SPI bus.
 * SPI operations are bounded synchronous polls, including during initialization.
 * On operational failure, CS deassertion is best-effort abort cleanup, not proof
 * of byte completion or bus recovery. Recover SPI/bus before calling InitStart.
 */

/* Validates wiring and starts the staged hardware reset without waiting. */
ErrorStatus_t TFT_InitStart(void);

/* Advances at most one timed stage or table entry; reports pending=0, ready=1.
 * Elapsed milliseconds are measured since the previous successful Process call
 * returned, or since InitStart returned. Excess time is discarded at transitions.
 * NULL leaves state unchanged; errors preserve the complete output.
 */
ErrorStatus_t TFT_InitProcess(u32 elapsedMilliseconds, u8 *complete);

/* Writes one RGB565 pixel at x=0..127, y=0..159 when ready. */
ErrorStatus_t TFT_DrawPixel(u16 x, u16 y, u16 color);

/* Synchronously fills a nonempty in-bounds rectangle when ready. */
ErrorStatus_t TFT_FillRectangle(u16 x, u16 y, u16 width, u16 height, u16 color);

/* Synchronously fills all 20480 pixels when ready. */
ErrorStatus_t TFT_FillScreen(u16 color);

#endif
