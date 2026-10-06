/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ESP_INTERFACE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : ESP_INTERFACE
 * Description : This file contains the ESP-WROOM-02D ESP-AT communication functionality.
 */

#ifndef ESP_INTERFACE_H
#define ESP_INTERFACE_H

#include "../../01_LIB/STD_TYPES.h"
#include "../../01_LIB/ERROR_STATUS.h"

typedef enum
{
    ESP_OPERATION_NONE,
    ESP_OPERATION_PREPARE,
    ESP_OPERATION_JOIN_AP,
    ESP_OPERATION_OPEN_TCP,
    ESP_OPERATION_SEND,
    ESP_OPERATION_CLOSE
} ESP_Operation_t;

typedef enum
{
    ESP_RESULT_NONE,
    ESP_RESULT_PENDING,
    ESP_RESULT_SUCCESS,
    ESP_RESULT_FAILURE,
    ESP_RESULT_TIMEOUT
} ESP_Result_t;

typedef enum
{
    ESP_WIFI_UNKNOWN,
    ESP_WIFI_DISCONNECTED,
    ESP_WIFI_ASSOCIATED,
    ESP_WIFI_IPV4_READY
} ESP_WiFiState_t;

typedef enum
{
    ESP_TCP_UNKNOWN,
    ESP_TCP_DISCONNECTED,
    ESP_TCP_CONNECTED
} ESP_TCPState_t;

typedef struct
{
    ESP_Operation_t operation;
    ESP_Result_t result;
    u8 prepared;
    ESP_WiFiState_t wifiState;
    ESP_TCPState_t tcpState;
    u8 recoveryRequired;
} ESP_Status_t;

/* ESP-WROOM-02D, official ESP-AT v2.3.0.0 ESP-WROOM-02 build only.
 * Application owns power/reset, clocks, pins, serial recovery and RX acquisition.
 * Configure/enable USART for 115200 8N1, normal TX/RX, no hardware flow control;
 * TX DMA and TXE/TC IRQ ownership must be disabled. Physical settings are not checked.
 * ESP GPIO15 TX connects to MCU RX; GPIO13 RX connects to MCU TX. CTS/RTS are unused.
 * Serialize ALL calls, including byte delivery; report any lost/corrupt RX bytes.
 * Service RX promptly: there is no HAL RX queue or flow control.
 * Start OK means accepted; inspect result for completion. Invalid starts preserve status.
 * Starts require a positive timeout, no active operation and complete RX framing.
 * Pending processing returns OK. Ordinary AT rejection is result FAILURE with API OK.
 * Recovery-required rejects starts and bytes. Timeout does not cancel an ESP command.
 */

/* Resets local state only; caller first restores idle AT mode and disposes of stale RX. */
ErrorStatus_t ESP_Init(void);

/* Starts preparation; caller guarantees no existing connections/server or pending command. */
ErrorStatus_t ESP_Prepare(u32 timeoutMs);

/* Starts joining when prepared and TCP is not connected; NULL password requires length zero. */
ErrorStatus_t ESP_JoinAccessPoint(const char *ssid, u8 ssidLength,
                                const char *password, u8 passwordLength, u32 timeoutMs);

/* Starts one TCP connection after preparation and IPv4 readiness. */
ErrorStatus_t ESP_OpenTCP(const char *host, u8 hostLength, u16 port, u32 timeoutMs);

/* Requires prepared/connected TCP; borrows immutable data until terminal completion/failure. */
ErrorStatus_t ESP_Send(const u8 *data, u16 length, u32 timeoutMs);

/* Starts closing a connected or unknown TCP connection after preparation. */
ErrorStatus_t ESP_Close(u32 timeoutMs);

/* Parses one acquired byte without USART access; network payload is discarded, not delivered. */
ErrorStatus_t ESP_ProcessByte(u8 byte);

/* Performs one cooperative action after checking the whole-operation deadline.
 * First elapsed interval: successful start return to this entry. Later intervals:
 * previous Process ENTRY to current ENTRY. Caller retains fractional milliseconds.
 * Early responses are latched; deadline checking precedes their final consumption.
 */
ErrorStatus_t ESP_Process(u32 elapsedMilliseconds);

/* Copies retained semantic status; invalid calls leave the output unchanged. */
ErrorStatus_t ESP_GetStatus(ESP_Status_t *status);

/* Records RX loss/corruption, requires external recovery and returns ERROR. */
ErrorStatus_t ESP_NotifyTransportError(void);

#endif
