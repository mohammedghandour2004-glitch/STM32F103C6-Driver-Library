/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ESP_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : ESP_PRIVATE
 * Description : This file contains the ESP-WROOM-02D ESP-AT communication functionality.
 */

#ifndef ESP_PRIVATE_H
#define ESP_PRIVATE_H

#define ESP_CR 0x0DU
#define ESP_LF 0x0AU
#define ESP_PREPARE_COUNT 8U
#define ESP_ECHO_OFF_STEP 1U

#define ESP_CMD_AT "AT"
#define ESP_CMD_ECHO_OFF "ATE0"
#define ESP_CMD_STORE_OFF "AT+SYSSTORE=0"
#define ESP_CMD_STATION "AT+CWMODE=1,0"
#define ESP_CMD_NORMAL "AT+CIPMODE=0"
#define ESP_CMD_SINGLE "AT+CIPMUX=0"
#define ESP_CMD_ENDPOINT_OFF "AT+CIPDINFO=0"
#define ESP_CMD_ACTIVE_RX "AT+CIPRECVMODE=0"
#define ESP_CMD_JOIN "AT+CWJAP="
#define ESP_CMD_OPEN "AT+CIPSTART=\"TCP\","
#define ESP_CMD_SEND "AT+CIPSEND="
#define ESP_CMD_CLOSE "AT+CIPCLOSE"

#define ESP_TOKEN_OK "OK"
#define ESP_TOKEN_ERROR "ERROR"
#define ESP_TOKEN_SEND_OK "SEND OK"
#define ESP_TOKEN_SEND_FAIL "SEND FAIL"
#define ESP_TOKEN_WIFI_CONNECTED "WIFI CONNECTED"
#define ESP_TOKEN_WIFI_IP "WIFI GOT IP"
#define ESP_TOKEN_WIFI_DISCONNECT "WIFI DISCONNECT"
#define ESP_TOKEN_CONNECT "CONNECT"
#define ESP_TOKEN_CLOSED "CLOSED"
#define ESP_TOKEN_READY "ready"
#define ESP_TOKEN_JOIN_DIAGNOSTIC "+CWJAP:"
#define ESP_IPD_PREFIX "+IPD,"

typedef enum
{
    ESP_TX_IDLE,
    ESP_TX_COMMAND,
    ESP_TX_COMMAND_TC,
    ESP_TX_WAIT_RESPONSE,
    ESP_TX_LOAD_PREPARE,
    ESP_TX_WAIT_PROMPT,
    ESP_TX_PAYLOAD,
    ESP_TX_PAYLOAD_TC,
    ESP_TX_WAIT_SEND_RESULT
} ESP_TransactionState_t;

typedef enum
{
    ESP_RESPONSE_NONE,
    ESP_RESPONSE_COMMAND,
    ESP_RESPONSE_SEND_RESULT
} ESP_ResponsePhase_t;

typedef enum
{
    ESP_PARSE_TEXT,
    ESP_PARSE_IPD_HEADER,
    ESP_PARSE_IPD_PAYLOAD
} ESP_ParserMode_t;

typedef enum
{
    ESP_REPLY_NONE,
    ESP_REPLY_OK,
    ESP_REPLY_ERROR,
    ESP_REPLY_SEND_OK,
    ESP_REPLY_SEND_FAIL
} ESP_Reply_t;

#endif
