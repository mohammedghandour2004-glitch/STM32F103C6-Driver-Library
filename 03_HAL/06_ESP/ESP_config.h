/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ESP_CONFIG.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : ESP_CONFIG
 * Description : This file contains the ESP-WROOM-02D ESP-AT communication functionality.
 */

#ifndef ESP_CONFIG_H
#define ESP_CONFIG_H

#include "../../02_MCAL/09_USART/USART_interface.h"

#ifndef ESP_USART_INSTANCE
#define ESP_USART_INSTANCE USART_INSTANCE_1
#endif
#ifndef ESP_COMMAND_CAPACITY
#define ESP_COMMAND_CAPACITY 256U
#endif
#ifndef ESP_LINE_CAPACITY
#define ESP_LINE_CAPACITY 128U
#endif
/* Input limits are project bounds; SSID/host limits are not firmware maxima. */
#ifndef ESP_SSID_MAX_LENGTH
#define ESP_SSID_MAX_LENGTH 32U
#endif
#ifndef ESP_PASSWORD_MAX_LENGTH
#define ESP_PASSWORD_MAX_LENGTH 64U
#endif
#ifndef ESP_HOST_MAX_LENGTH
#define ESP_HOST_MAX_LENGTH 64U
#endif
#ifndef ESP_PAYLOAD_MAX_LENGTH
#define ESP_PAYLOAD_MAX_LENGTH 256U
#endif
/* Decimal representation bound, not an ESP network frame-size claim. */
#ifndef ESP_IPD_MAX_DIGITS
#define ESP_IPD_MAX_DIGITS 10U
#endif

#if (ESP_COMMAND_CAPACITY < 20) || (ESP_COMMAND_CAPACITY > 256)
#error ESP_COMMAND_CAPACITY_must_be_20_to_256_including_CRLF
#endif
#if (ESP_LINE_CAPACITY < 32) || (ESP_LINE_CAPACITY > 256)
#error ESP_LINE_CAPACITY_must_be_32_to_256
#endif
#if (ESP_SSID_MAX_LENGTH < 1) || (ESP_SSID_MAX_LENGTH > 32)
#error ESP_SSID_MAX_LENGTH_must_be_1_to_32
#endif
#if (ESP_PASSWORD_MAX_LENGTH < 0) || (ESP_PASSWORD_MAX_LENGTH > 64)
#error ESP_PASSWORD_MAX_LENGTH_must_be_0_to_64
#endif
#if (ESP_HOST_MAX_LENGTH < 1) || (ESP_HOST_MAX_LENGTH > 64)
#error ESP_HOST_MAX_LENGTH_must_be_1_to_64
#endif
#if (ESP_PAYLOAD_MAX_LENGTH < 1) || (ESP_PAYLOAD_MAX_LENGTH > 256)
#error ESP_PAYLOAD_MAX_LENGTH_must_be_1_to_256
#endif
#if (ESP_IPD_MAX_DIGITS < 1) || (ESP_IPD_MAX_DIGITS > 10)
#error ESP_IPD_MAX_DIGITS_must_be_1_to_10
#endif

#endif
