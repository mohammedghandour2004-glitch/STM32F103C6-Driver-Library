/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< IR_COMMUNICATION_PRIVATE.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : IR_COMMUNICATION_PRIVATE
 * Description : This file contains the NEC infrared communication decoding functionality.
 */

#ifndef IR_COMMUNICATION_PRIVATE_H
#define IR_COMMUNICATION_PRIVATE_H

typedef enum
{
    IR_STATE_IDLE,
    IR_STATE_LEADER_LOW,
    IR_STATE_LEADER_HIGH,
    IR_STATE_DATA_LOW,
    IR_STATE_DATA_HIGH,
    IR_STATE_FINAL_LOW
} IR_Communication_State_t;

#endif
