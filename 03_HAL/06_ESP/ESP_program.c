/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ESP_PROGRAM.c >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : ESP_PROGRAM
 * Description : This file contains the ESP-WROOM-02D ESP-AT communication functionality.
 */

#include "ESP_interface.h"
#include "ESP_private.h"
#include "ESP_config.h"

typedef struct
{
    u8 bytes[ESP_COMMAND_CAPACITY];
    u16 length;
} ESP_Command_t;

static const char * const prepareCommands[ESP_PREPARE_COUNT] =
{
    ESP_CMD_AT, ESP_CMD_ECHO_OFF, ESP_CMD_STORE_OFF, ESP_CMD_STATION,
    ESP_CMD_NORMAL, ESP_CMD_SINGLE, ESP_CMD_ENDPOINT_OFF, ESP_CMD_ACTIVE_RX
};

static u8 initialized;
static ESP_Status_t status;
static ESP_TransactionState_t transaction;
static ESP_Command_t command;
static u16 commandIndex;
static u8 line[ESP_LINE_CAPACITY];
static u16 lineLength;
static u8 sawCR;
static ESP_ParserMode_t parserMode;
static u32 ipdLength;
static u8 ipdDigits;
static u32 discardRemaining;
static u32 age;
static u32 deadline;
static u8 prepareStep;
static const u8 *payload;
static u16 payloadLength;
static u16 payloadIndex;
static ESP_Reply_t reply;
/* RX ownership follows completed submission, independently of cooperative TC processing. */
static ESP_ResponsePhase_t responsePhase;
static u8 sendAccepted;
static u8 promptSeen;
static u8 promptSpace;
static u8 connectSeen;
static u8 connectBeforeOK;
static u8 linkLost;
/* Later unsolicited reports must win over a terminal awaiting cooperative consumption. */
static u8 wifiAfterReply;
static u8 tcpAfterReply;

/* Clears bytes through volatile accesses so credential erasure is not optimized away. */
static void ESP_ClearBytes(u8 *bytes, u16 count)
{
    volatile u8 *destination = bytes;
    u16 i;
    for (i = 0U; i < count; ++i)
    {
        destination[i] = 0U;
    }
}

/* Appends one byte without exceeding the encoded command capacity. */
static ErrorStatus_t ESP_AppendByte(ESP_Command_t *target, u8 byte)
{
    if (target->length >= ESP_COMMAND_CAPACITY)
    {
        return ERROR_STATUS_ERROR;
    }
    target->bytes[target->length++] = byte;
    return ERROR_STATUS_OK;
}

/* Appends a bounded private literal, excluding its C terminator. */
static ErrorStatus_t ESP_AppendLiteral(ESP_Command_t *target, const char *literal)
{
    u16 i;
    for (i = 0U; i < ESP_COMMAND_CAPACITY; ++i)
    {
        if (literal[i] == '\0')
        {
            return ERROR_STATUS_OK;
        }
        if (ESP_AppendByte(target, (u8)literal[i]) != ERROR_STATUS_OK)
        {
            return ERROR_STATUS_ERROR;
        }
    }
    return ERROR_STATUS_ERROR;
}

/* Encodes a u16 as decimal without formatted-I/O dependencies. */
static ErrorStatus_t ESP_AppendDecimal(ESP_Command_t *target, u16 value)
{
    u8 digits[5];
    u8 count = 0U;
    do
    {
        digits[count++] = (u8)('0' + value % 10U);
        value = (u16)(value / 10U);
    } while (value != 0U);
    while (count > 0U)
    {
        if (ESP_AppendByte(target, digits[--count]) != ERROR_STATUS_OK)
        {
            return ERROR_STATUS_ERROR;
        }
    }
    return ERROR_STATUS_OK;
}

/* Encodes an explicit-length printable argument with the approved AT escapes. */
static ErrorStatus_t ESP_AppendQuoted(ESP_Command_t *target, const char *text, u8 length)
{
    u8 i;
    u8 byte;
    if (ESP_AppendByte(target, (u8)'"') != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    for (i = 0U; i < length; ++i)
    {
        byte = (u8)text[i];
        if ((byte < 0x20U) || (byte > 0x7EU))
        {
            return ERROR_STATUS_ERROR;
        }
        if ((byte == (u8)',') || (byte == (u8)'"') || (byte == (u8)'\\'))
        {
            if (ESP_AppendByte(target, (u8)'\\') != ERROR_STATUS_OK)
            {
                return ERROR_STATUS_ERROR;
            }
        }
        if (ESP_AppendByte(target, byte) != ERROR_STATUS_OK)
        {
            return ERROR_STATUS_ERROR;
        }
    }
    return ESP_AppendByte(target, (u8)'"');
}

/* Appends the two wire terminators within the same command capacity. */
static ErrorStatus_t ESP_AppendCRLF(ESP_Command_t *target)
{
    if (ESP_AppendByte(target, ESP_CR) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    return ESP_AppendByte(target, ESP_LF);
}

/* Resets observations belonging to one command, not the operation deadline. */
static void ESP_ClearObservations(void)
{
    reply = ESP_REPLY_NONE;
    responsePhase = ESP_RESPONSE_NONE;
    sendAccepted = 0U;
    promptSeen = 0U;
    connectSeen = 0U;
    connectBeforeOK = 0U;
    linkLost = 0U;
    wifiAfterReply = 0U;
    tcpAfterReply = 0U;
}

/* Discards parser framing only when locally resetting or abandoning synchronization. */
static void ESP_ResetParser(void)
{
    ESP_ClearBytes(line, (u16)ESP_LINE_CAPACITY);
    lineLength = 0U;
    sawCR = 0U;
    parserMode = ESP_PARSE_TEXT;
    ipdLength = 0U;
    ipdDigits = 0U;
    discardRemaining = 0U;
    promptSpace = 0U;
}

/* Releases command credentials and the borrowed payload without touching RX framing. */
static void ESP_ReleaseTransaction(void)
{
    ESP_ClearBytes(command.bytes, command.length);
    command.length = 0U;
    commandIndex = 0U;
    payload = NULL;
    payloadLength = 0U;
    payloadIndex = 0U;
    transaction = ESP_TX_IDLE;
    ESP_ClearObservations();
}

/* Stops uncertain protocol activity while preserving any previously completed result. */
static ErrorStatus_t ESP_RequireRecovery(ESP_Result_t reason)
{
    if (status.result == ESP_RESULT_PENDING)
    {
        status.result = reason;
    }
    status.prepared = 0U;
    status.wifiState = ESP_WIFI_UNKNOWN;
    status.tcpState = ESP_TCP_UNKNOWN;
    status.recoveryRequired = 1U;
    ESP_ReleaseTransaction();
    ESP_ResetParser();
    return ERROR_STATUS_ERROR;
}

/* Completes a synchronized operation, retaining semantic outcome for the caller. */
static void ESP_Finish(u8 success)
{
    status.result = (success != 0U) ? ESP_RESULT_SUCCESS : ESP_RESULT_FAILURE;
    if (success != 0U)
    {
        switch (status.operation)
        {
            case ESP_OPERATION_PREPARE: status.prepared = 1U; break;
            case ESP_OPERATION_JOIN_AP:
                if (wifiAfterReply == 0U) { status.wifiState = ESP_WIFI_IPV4_READY; }
                break;
            case ESP_OPERATION_CLOSE:
                if (tcpAfterReply == 0U) { status.tcpState = ESP_TCP_DISCONNECTED; }
                break;
            default: break;
        }
    }
    else if ((status.operation == ESP_OPERATION_OPEN_TCP) ||
             (status.operation == ESP_OPERATION_CLOSE) ||
             (status.operation == ESP_OPERATION_SEND))
    {
        if ((tcpAfterReply == 0U) && (status.tcpState != ESP_TCP_DISCONNECTED))
        {
            status.tcpState = ESP_TCP_UNKNOWN;
        }
    }
    else if ((status.operation == ESP_OPERATION_JOIN_AP) &&
             (wifiAfterReply == 0U) &&
             (status.wifiState != ESP_WIFI_DISCONNECTED))
    {
        status.wifiState = ESP_WIFI_UNKNOWN;
    }
    ESP_ReleaseTransaction();
}

/* Rejects starts without changing retained status or partially received framing. */
static ErrorStatus_t ESP_CanStart(u32 timeoutMs)
{
    if ((initialized == 0U) || (status.recoveryRequired != 0U) ||
        (status.result == ESP_RESULT_PENDING) || (timeoutMs == 0U) ||
        (parserMode != ESP_PARSE_TEXT) || (lineLength != 0U) || (sawCR != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    return ERROR_STATUS_OK;
}

/* Commits a fully built operation and erases its temporary command copy. */
static void ESP_Accept(ESP_Operation_t operation, ESP_Command_t *built, u32 timeoutMs)
{
    u16 i;
    ESP_ReleaseTransaction();
    for (i = 0U; i < built->length; ++i)
    {
        command.bytes[i] = built->bytes[i];
    }
    command.length = built->length;
    ESP_ClearBytes(built->bytes, built->length);
    built->length = 0U;
    status.operation = operation;
    status.result = ESP_RESULT_PENDING;
    age = 0U;
    deadline = timeoutMs;
    transaction = ESP_TX_COMMAND;
}

/* Builds one private fixed command including its explicit wire terminators. */
static ErrorStatus_t ESP_BuildFixed(ESP_Command_t *built, const char *literal)
{
    built->length = 0U;
    if (ESP_AppendLiteral(built, literal) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    return ESP_AppendCRLF(built);
}

/* Validates the real USART instance and every preparation command before hardware use. */
static ErrorStatus_t ESP_ValidateConfig(void)
{
    u8 i;
    ESP_Command_t built;
    if ((ESP_USART_INSTANCE != USART_INSTANCE_1) &&
        (ESP_USART_INSTANCE != USART_INSTANCE_2))
    {
        return ERROR_STATUS_ERROR;
    }
    for (i = 0U; i < ESP_PREPARE_COUNT; ++i)
    {
        if (ESP_BuildFixed(&built, prepareCommands[i]) != ERROR_STATUS_OK)
        {
            ESP_ClearBytes(built.bytes, built.length);
            return ERROR_STATUS_ERROR;
        }
        ESP_ClearBytes(built.bytes, built.length);
    }
    return ERROR_STATUS_OK;
}

/* Resets local state without asserting that the external ESP has been recovered. */
ErrorStatus_t ESP_Init(void)
{
    initialized = 0U;
    ESP_ReleaseTransaction();
    ESP_ClearBytes(command.bytes, (u16)ESP_COMMAND_CAPACITY);
    ESP_ResetParser();
    status.operation = ESP_OPERATION_NONE;
    status.result = ESP_RESULT_NONE;
    status.prepared = 0U;
    status.wifiState = ESP_WIFI_UNKNOWN;
    status.tcpState = ESP_TCP_UNKNOWN;
    status.recoveryRequired = 0U;
    prepareStep = 0U;
    age = 0U;
    deadline = 0U;
    if (ESP_ValidateConfig() != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    initialized = 1U;
    return ERROR_STATUS_OK;
}

/* Accepts the first command of the exact approved eight-step preparation. */
ErrorStatus_t ESP_Prepare(u32 timeoutMs)
{
    ESP_Command_t built;
    if (ESP_CanStart(timeoutMs) != ERROR_STATUS_OK)
    {
        return ERROR_STATUS_ERROR;
    }
    if (ESP_BuildFixed(&built, prepareCommands[0]) != ERROR_STATUS_OK)
    {
        ESP_ClearBytes(built.bytes, built.length);
        return ERROR_STATUS_ERROR;
    }
    ESP_Accept(ESP_OPERATION_PREPARE, &built, timeoutMs);
    prepareStep = 0U;
    status.prepared = 0U;
    return ERROR_STATUS_OK;
}

/* Builds a complete escaped join command before changing any transaction state. */
ErrorStatus_t ESP_JoinAccessPoint(const char *ssid, u8 ssidLength,
                                const char *password, u8 passwordLength, u32 timeoutMs)
{
    ESP_Command_t built;
    if ((ESP_CanStart(timeoutMs) != ERROR_STATUS_OK) || (status.prepared == 0U) ||
        (status.tcpState == ESP_TCP_CONNECTED) || (ssid == NULL) ||
        (ssidLength == 0U) || (ssidLength > ESP_SSID_MAX_LENGTH) ||
        (passwordLength > ESP_PASSWORD_MAX_LENGTH) ||
        ((passwordLength > 0U) && (password == NULL)))
    {
        return ERROR_STATUS_ERROR;
    }
    built.length = 0U;
    if ((ESP_AppendLiteral(&built, ESP_CMD_JOIN) != ERROR_STATUS_OK) ||
        (ESP_AppendQuoted(&built, ssid, ssidLength) != ERROR_STATUS_OK) ||
        (ESP_AppendByte(&built, (u8)',') != ERROR_STATUS_OK) ||
        (ESP_AppendQuoted(&built, password, passwordLength) != ERROR_STATUS_OK) ||
        (ESP_AppendCRLF(&built) != ERROR_STATUS_OK))
    {
        ESP_ClearBytes(built.bytes, built.length);
        return ERROR_STATUS_ERROR;
    }
    ESP_Accept(ESP_OPERATION_JOIN_AP, &built, timeoutMs);
    return ERROR_STATUS_OK;
}

/* Accepts one normal TCP open request with a bounded printable host argument. */
ErrorStatus_t ESP_OpenTCP(const char *host, u8 hostLength, u16 port, u32 timeoutMs)
{
    ESP_Command_t built;
    if ((ESP_CanStart(timeoutMs) != ERROR_STATUS_OK) || (status.prepared == 0U) ||
        (status.wifiState != ESP_WIFI_IPV4_READY) || (status.tcpState == ESP_TCP_CONNECTED) ||
        (host == NULL) || (hostLength == 0U) || (hostLength > ESP_HOST_MAX_LENGTH) || (port == 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    built.length = 0U;
    if ((ESP_AppendLiteral(&built, ESP_CMD_OPEN) != ERROR_STATUS_OK) ||
        (ESP_AppendQuoted(&built, host, hostLength) != ERROR_STATUS_OK) ||
        (ESP_AppendByte(&built, (u8)',') != ERROR_STATUS_OK) ||
        (ESP_AppendDecimal(&built, port) != ERROR_STATUS_OK) ||
        (ESP_AppendCRLF(&built) != ERROR_STATUS_OK))
    {
        ESP_ClearBytes(built.bytes, built.length);
        return ERROR_STATUS_ERROR;
    }
    ESP_Accept(ESP_OPERATION_OPEN_TCP, &built, timeoutMs);
    return ERROR_STATUS_OK;
}

/* Accepts a length-based send and borrows the raw payload until completion. */
ErrorStatus_t ESP_Send(const u8 *data, u16 length, u32 timeoutMs)
{
    ESP_Command_t built;
    if ((ESP_CanStart(timeoutMs) != ERROR_STATUS_OK) || (status.prepared == 0U) ||
        (status.tcpState != ESP_TCP_CONNECTED) || (data == NULL) ||
        (length == 0U) || (length > ESP_PAYLOAD_MAX_LENGTH))
    {
        return ERROR_STATUS_ERROR;
    }
    built.length = 0U;
    if ((ESP_AppendLiteral(&built, ESP_CMD_SEND) != ERROR_STATUS_OK) ||
        (ESP_AppendDecimal(&built, length) != ERROR_STATUS_OK) ||
        (ESP_AppendCRLF(&built) != ERROR_STATUS_OK))
    {
        ESP_ClearBytes(built.bytes, built.length);
        return ERROR_STATUS_ERROR;
    }
    ESP_Accept(ESP_OPERATION_SEND, &built, timeoutMs);
    payload = data;
    payloadLength = length;
    return ERROR_STATUS_OK;
}

/* Accepts an explicit close without adding hidden prerequisite commands. */
ErrorStatus_t ESP_Close(u32 timeoutMs)
{
    ESP_Command_t built;
    if ((ESP_CanStart(timeoutMs) != ERROR_STATUS_OK) || (status.prepared == 0U) ||
        ((status.tcpState != ESP_TCP_CONNECTED) && (status.tcpState != ESP_TCP_UNKNOWN)))
    {
        return ERROR_STATUS_ERROR;
    }
    if (ESP_BuildFixed(&built, ESP_CMD_CLOSE) != ERROR_STATUS_OK)
    {
        ESP_ClearBytes(built.bytes, built.length);
        return ERROR_STATUS_ERROR;
    }
    ESP_Accept(ESP_OPERATION_CLOSE, &built, timeoutMs);
    return ERROR_STATUS_OK;
}

/* Compares an explicit-length line to a private token without substring matching. */
static u8 ESP_LineEquals(const char *token)
{
    u16 i;
    for (i = 0U; i < lineLength; ++i)
    {
        if ((token[i] == '\0') || (line[i] != (u8)token[i]))
        {
            return 0U;
        }
    }
    return (token[lineLength] == '\0') ? 1U : 0U;
}

/* Checks a bounded line prefix for structured diagnostics and framing. */
static u8 ESP_LineStarts(const char *prefix, u16 length)
{
    u16 i;
    if (lineLength < length)
    {
        return 0U;
    }
    for (i = 0U; i < length; ++i)
    {
        if (line[i] != (u8)prefix[i])
        {
            return 0U;
        }
    }
    return 1U;
}

/* Allows early command observations only after the complete command was submitted. */
static u8 ESP_CommandResponseWindow(void)
{
    return (u8)((status.result == ESP_RESULT_PENDING) &&
                (responsePhase == ESP_RESPONSE_COMMAND));
}

/* Allows an early send result after every requested payload byte was submitted. */
static u8 ESP_SendResultWindow(void)
{
    return (u8)((status.result == ESP_RESULT_PENDING) &&
                (status.operation == ESP_OPERATION_SEND) && (payloadIndex == payloadLength) &&
                (responsePhase == ESP_RESPONSE_SEND_RESULT));
}

/* Recognizes only the exact outstanding preparation echo before echo is disabled. */
static u8 ESP_IsExpectedEcho(void)
{
    u16 i;
    if ((status.result != ESP_RESULT_PENDING) || (status.operation != ESP_OPERATION_PREPARE) ||
        (prepareStep > ESP_ECHO_OFF_STEP) || (command.length < 2U) ||
        (lineLength != command.length - 2U))
    {
        return 0U;
    }
    for (i = 0U; i < lineLength; ++i)
    {
        if (line[i] != command.bytes[i])
        {
            return 0U;
        }
    }
    return 1U;
}

/* Latches a single terminal response without prematurely completing an operation. */
static ErrorStatus_t ESP_LatchReply(ESP_Reply_t observed)
{
    if (reply != ESP_REPLY_NONE)
    {
        return ESP_RequireRecovery(ESP_RESULT_FAILURE);
    }
    reply = observed;
    return ERROR_STATUS_OK;
}

/* Updates asynchronous link observations independently of retained operation results. */
static void ESP_ObserveDisconnect(u8 wifi)
{
    if (wifi != 0U)
    {
        status.wifiState = ESP_WIFI_DISCONNECTED;
        if (reply != ESP_REPLY_NONE) { wifiAfterReply = 1U; }
    }
    status.tcpState = ESP_TCP_DISCONNECTED;
    if (reply != ESP_REPLY_NONE) { tcpAfterReply = 1U; }
    if ((status.result == ESP_RESULT_PENDING) &&
        ((status.operation == ESP_OPERATION_SEND) || (status.operation == ESP_OPERATION_OPEN_TCP)))
    {
        linkLost = 1U;
    }
}

/* Interprets complete verified text tokens, leaving unknown bounded lines non-terminal. */
static ErrorStatus_t ESP_ParseLine(void)
{
    u16 i;
    if ((lineLength == 0U) || (ESP_IsExpectedEcho() != 0U))
    {
        return ERROR_STATUS_OK;
    }
    if (ESP_LineEquals(ESP_TOKEN_READY) != 0U)
    {
        return ESP_RequireRecovery(ESP_RESULT_FAILURE);
    }
    if (ESP_LineEquals(ESP_TOKEN_WIFI_CONNECTED) != 0U)
    {
        status.wifiState = ESP_WIFI_ASSOCIATED;
        if (reply != ESP_REPLY_NONE) { wifiAfterReply = 1U; }
    }
    else if (ESP_LineEquals(ESP_TOKEN_WIFI_IP) != 0U)
    {
        status.wifiState = ESP_WIFI_IPV4_READY;
        if (reply != ESP_REPLY_NONE) { wifiAfterReply = 1U; }
    }
    else if (ESP_LineEquals(ESP_TOKEN_WIFI_DISCONNECT) != 0U)
    {
        ESP_ObserveDisconnect(1U);
    }
    else if (ESP_LineEquals(ESP_TOKEN_CONNECT) != 0U)
    {
        status.tcpState = ESP_TCP_CONNECTED;
        if (reply != ESP_REPLY_NONE) { tcpAfterReply = 1U; }
        if ((status.operation == ESP_OPERATION_OPEN_TCP) && (ESP_CommandResponseWindow() != 0U))
        {
            connectSeen = 1U;
        }
    }
    else if (ESP_LineEquals(ESP_TOKEN_CLOSED) != 0U)
    {
        ESP_ObserveDisconnect(0U);
    }
    else if (ESP_LineStarts(ESP_TOKEN_JOIN_DIAGNOSTIC, 7U) != 0U)
    {
        if (lineLength == 7U)
        {
            return ESP_RequireRecovery(ESP_RESULT_FAILURE);
        }
        for (i = 7U; i < lineLength; ++i)
        {
            if ((line[i] < (u8)'0') || (line[i] > (u8)'9'))
            {
                return ESP_RequireRecovery(ESP_RESULT_FAILURE);
            }
        }
    }
    else if (ESP_LineEquals(ESP_TOKEN_OK) != 0U)
    {
        if (ESP_CommandResponseWindow() == 0U)
        {
            return ESP_RequireRecovery(ESP_RESULT_FAILURE);
        }
        if (status.operation == ESP_OPERATION_SEND)
        {
            if (sendAccepted != 0U)
            {
                return ESP_RequireRecovery(ESP_RESULT_FAILURE);
            }
            sendAccepted = 1U;
        }
        else
        {
            connectBeforeOK = (u8)((connectSeen != 0U) && (status.tcpState == ESP_TCP_CONNECTED));
            return ESP_LatchReply(ESP_REPLY_OK);
        }
    }
    else if (ESP_LineEquals(ESP_TOKEN_ERROR) != 0U)
    {
        if ((ESP_CommandResponseWindow() == 0U) && (ESP_SendResultWindow() == 0U))
        {
            return ESP_RequireRecovery(ESP_RESULT_FAILURE);
        }
        return ESP_LatchReply(ESP_REPLY_ERROR);
    }
    else if ((ESP_LineEquals(ESP_TOKEN_SEND_OK) != 0U) ||
             (ESP_LineEquals(ESP_TOKEN_SEND_FAIL) != 0U))
    {
        if (ESP_SendResultWindow() == 0U)
        {
            return ESP_RequireRecovery(ESP_RESULT_FAILURE);
        }
        return ESP_LatchReply((ESP_LineEquals(ESP_TOKEN_SEND_OK) != 0U) ?
                              ESP_REPLY_SEND_OK : ESP_REPLY_SEND_FAIL);
    }
    else if (ESP_LineStarts(ESP_IPD_PREFIX, 4U) != 0U)
    {
        return ESP_RequireRecovery(ESP_RESULT_FAILURE);
    }
    return ERROR_STATUS_OK;
}

/* Parses checked decimal framing and switches to exact-length raw-byte discard. */
static ErrorStatus_t ESP_ParseIPD(u8 byte)
{
    u32 digit;
    if ((byte >= (u8)'0') && (byte <= (u8)'9'))
    {
        digit = (u32)(byte - (u8)'0');
        if ((ipdDigits >= ESP_IPD_MAX_DIGITS) || (ipdLength > (UINT32_MAX - digit) / 10U))
        {
            return ESP_RequireRecovery(ESP_RESULT_FAILURE);
        }
        ipdLength = ipdLength * 10U + digit;
        ++ipdDigits;
        return ERROR_STATUS_OK;
    }
    if ((byte != (u8)':') || (ipdDigits == 0U))
    {
        return ESP_RequireRecovery(ESP_RESULT_FAILURE);
    }
    discardRemaining = ipdLength;
    parserMode = (discardRemaining == 0U) ? ESP_PARSE_TEXT : ESP_PARSE_IPD_PAYLOAD;
    return ERROR_STATUS_OK;
}

/* Feeds one RX byte without accessing USART or allowing payload bytes into token matching. */
ErrorStatus_t ESP_ProcessByte(u8 byte)
{
    ErrorStatus_t result;
    if ((initialized == 0U) || (status.recoveryRequired != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    if (parserMode == ESP_PARSE_IPD_PAYLOAD)
    {
        --discardRemaining;
        if (discardRemaining == 0U)
        {
            parserMode = ESP_PARSE_TEXT;
        }
        return ERROR_STATUS_OK;
    }
    if (parserMode == ESP_PARSE_IPD_HEADER)
    {
        return ESP_ParseIPD(byte);
    }
    if (promptSpace != 0U)
    {
        promptSpace = 0U;
        if (byte == (u8)' ')
        {
            return ERROR_STATUS_OK;
        }
    }
    if (sawCR != 0U)
    {
        if (byte != ESP_LF)
        {
            return ESP_RequireRecovery(ESP_RESULT_FAILURE);
        }
        sawCR = 0U;
        result = ESP_ParseLine();
        ESP_ClearBytes(line, lineLength);
        lineLength = 0U;
        return result;
    }
    if (byte == ESP_CR)
    {
        sawCR = 1U;
        return ERROR_STATUS_OK;
    }
    /* A submitted CIPSEND owns its prompt even before its TC state is consumed. */
    if ((byte == (u8)'>') && (lineLength == 0U) &&
        (status.operation == ESP_OPERATION_SEND) && (ESP_CommandResponseWindow() != 0U))
    {
        if ((promptSeen != 0U) || (sendAccepted == 0U) || (reply != ESP_REPLY_NONE))
        {
            return ESP_RequireRecovery(ESP_RESULT_FAILURE);
        }
        promptSeen = 1U;
        promptSpace = 1U;
        return ERROR_STATUS_OK;
    }
    if ((byte < 0x20U) || (byte > 0x7EU) || (lineLength >= ESP_LINE_CAPACITY))
    {
        return ESP_RequireRecovery(ESP_RESULT_FAILURE);
    }
    line[lineLength++] = byte;
    if ((lineLength == 5U) && (ESP_LineStarts(ESP_IPD_PREFIX, 4U) != 0U))
    {
        if (byte != (u8)',')
        {
            return ESP_RequireRecovery(ESP_RESULT_FAILURE);
        }
        lineLength = 0U;
        ipdLength = 0U;
        ipdDigits = 0U;
        parserMode = ESP_PARSE_IPD_HEADER;
    }
    return ERROR_STATUS_OK;
}

/* Submits at most one byte, treating not-ready as pending rather than a fault. */
static ErrorStatus_t ESP_TransmitStep(u8 byte, u16 *index, u16 length,
                                     ESP_ResponsePhase_t completedPhase)
{
    u8 ready;
    if (USART_GetFlag(ESP_USART_INSTANCE, USART_FLAG_TX_EMPTY, &ready) != ERROR_STATUS_OK)
    {
        return ESP_RequireRecovery(ESP_RESULT_FAILURE);
    }
    if (ready != 0U)
    {
        if (USART_Transmit(ESP_USART_INSTANCE, byte, 0U) != ERROR_STATUS_OK)
        {
            return ESP_RequireRecovery(ESP_RESULT_FAILURE);
        }
        ++(*index);
        if (*index == length)
        {
            responsePhase = completedPhase;
        }
    }
    return ERROR_STATUS_OK;
}

/* Confirms final wire completion without an internal polling loop. */
static ErrorStatus_t ESP_TCStep(ESP_TransactionState_t next)
{
    u8 ready;
    if (USART_GetFlag(ESP_USART_INSTANCE, USART_FLAG_TRANSMISSION_COMPLETE, &ready) != ERROR_STATUS_OK)
    {
        return ESP_RequireRecovery(ESP_RESULT_FAILURE);
    }
    if (ready != 0U)
    {
        if (USART_WaitTransmissionComplete(ESP_USART_INSTANCE, 0U) != ERROR_STATUS_OK)
        {
            return ESP_RequireRecovery(ESP_RESULT_FAILURE);
        }
        transaction = next;
    }
    return ERROR_STATUS_OK;
}

/* Consumes a command terminal only after TC and complete incoming framing. */
static ErrorStatus_t ESP_ResponseStep(void)
{
    if (reply == ESP_REPLY_NONE)
    {
        return ERROR_STATUS_OK;
    }
    if (reply == ESP_REPLY_ERROR)
    {
        ESP_Finish(0U);
    }
    else if (reply == ESP_REPLY_OK)
    {
        if ((status.operation == ESP_OPERATION_PREPARE) && (prepareStep + 1U < ESP_PREPARE_COUNT))
        {
            ++prepareStep;
            ESP_ClearObservations();
            transaction = ESP_TX_LOAD_PREPARE;
        }
        else if (status.operation == ESP_OPERATION_OPEN_TCP)
        {
            ESP_Finish(connectBeforeOK);
        }
        else
        {
            ESP_Finish(1U);
        }
    }
    else
    {
        return ESP_RequireRecovery(ESP_RESULT_FAILURE);
    }
    return ERROR_STATUS_OK;
}

/* Advances one action after applying the mandatory whole-operation deadline. */
ErrorStatus_t ESP_Process(u32 elapsedMilliseconds)
{
    if ((initialized == 0U) || (status.recoveryRequired != 0U))
    {
        return ERROR_STATUS_ERROR;
    }
    if (status.result != ESP_RESULT_PENDING)
    {
        return ERROR_STATUS_OK;
    }
    age = (elapsedMilliseconds > UINT32_MAX - age) ? UINT32_MAX : age + elapsedMilliseconds;
    if (age >= deadline)
    {
        return ESP_RequireRecovery(ESP_RESULT_TIMEOUT);
    }
    if ((transaction == ESP_TX_PAYLOAD) && (payloadIndex < payloadLength) && (linkLost != 0U))
    {
        return ESP_RequireRecovery(ESP_RESULT_FAILURE);
    }
    switch (transaction)
    {
        case ESP_TX_COMMAND:
            if (commandIndex < command.length)
            {
                return ESP_TransmitStep(command.bytes[commandIndex], &commandIndex,
                                        command.length, ESP_RESPONSE_COMMAND);
            }
            transaction = ESP_TX_COMMAND_TC;
            return ERROR_STATUS_OK;
        case ESP_TX_COMMAND_TC:
            return ESP_TCStep((status.operation == ESP_OPERATION_SEND) ?
                              ESP_TX_WAIT_PROMPT : ESP_TX_WAIT_RESPONSE);
        case ESP_TX_PAYLOAD:
            if (payloadIndex < payloadLength)
            {
                return ESP_TransmitStep(payload[payloadIndex], &payloadIndex,
                                        payloadLength, ESP_RESPONSE_SEND_RESULT);
            }
            transaction = ESP_TX_PAYLOAD_TC;
            return ERROR_STATUS_OK;
        case ESP_TX_PAYLOAD_TC:
            return ESP_TCStep(ESP_TX_WAIT_SEND_RESULT);
        default:
            break;
    }
    /* Never advance a command boundary through an incomplete incoming frame/line. */
    if ((parserMode != ESP_PARSE_TEXT) || (lineLength != 0U) || (sawCR != 0U))
    {
        return ERROR_STATUS_OK;
    }
    switch (transaction)
    {
        case ESP_TX_WAIT_RESPONSE:
            return ESP_ResponseStep();
        case ESP_TX_LOAD_PREPARE:
            ESP_ClearBytes(command.bytes, command.length);
            if (ESP_BuildFixed(&command, prepareCommands[prepareStep]) != ERROR_STATUS_OK)
            {
                return ESP_RequireRecovery(ESP_RESULT_FAILURE);
            }
            commandIndex = 0U;
            transaction = ESP_TX_COMMAND;
            return ERROR_STATUS_OK;
        case ESP_TX_WAIT_PROMPT:
            if (reply == ESP_REPLY_ERROR)
            {
                if (promptSeen != 0U)
                {
                    return ESP_RequireRecovery(ESP_RESULT_FAILURE);
                }
                ESP_Finish(0U);
            }
            else if (promptSeen != 0U)
            {
                if (linkLost != 0U)
                {
                    return ESP_RequireRecovery(ESP_RESULT_FAILURE);
                }
                /* Consume command acceptance/prompt; retain RX framing and link observations. */
                responsePhase = ESP_RESPONSE_NONE;
                sendAccepted = 0U;
                promptSeen = 0U;
                transaction = ESP_TX_PAYLOAD;
            }
            return ERROR_STATUS_OK;
        case ESP_TX_WAIT_SEND_RESULT:
            if (reply == ESP_REPLY_SEND_OK)
            {
                ESP_Finish(1U);
            }
            else if ((reply == ESP_REPLY_SEND_FAIL) || (reply == ESP_REPLY_ERROR))
            {
                ESP_Finish(0U);
            }
            return ERROR_STATUS_OK;
        default:
            return ESP_RequireRecovery(ESP_RESULT_FAILURE);
    }
}

/* Copies semantic state under the caller's serialization contract. */
ErrorStatus_t ESP_GetStatus(ESP_Status_t *output)
{
    if ((initialized == 0U) || (output == NULL))
    {
        return ERROR_STATUS_ERROR;
    }
    *output = status;
    return ERROR_STATUS_OK;
}

/* Invalidates synchronization after application-detected receive loss or corruption. */
ErrorStatus_t ESP_NotifyTransportError(void)
{
    if (initialized == 0U)
    {
        return ERROR_STATUS_ERROR;
    }
    return ESP_RequireRecovery(ESP_RESULT_FAILURE);
}
