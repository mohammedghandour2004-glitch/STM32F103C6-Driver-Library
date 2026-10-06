/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< IR_COMMUNICATION_CONFIG.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 03_HAL
 * Component   : IR_COMMUNICATION_CONFIG
 * Description : This file contains the NEC infrared communication decoding functionality.
 */

#ifndef IR_COMMUNICATION_CONFIG_H
#define IR_COMMUNICATION_CONFIG_H

/* Inclusive microsecond windows are engineering defaults, not timing guarantees. */

#ifndef IR_LEADER_LOW_MIN_US
#define IR_LEADER_LOW_MIN_US 8000U
#endif

#ifndef IR_LEADER_LOW_MAX_US
#define IR_LEADER_LOW_MAX_US 10000U
#endif

#ifndef IR_FULL_LEADER_HIGH_MIN_US
#define IR_FULL_LEADER_HIGH_MIN_US 4000U
#endif

#ifndef IR_FULL_LEADER_HIGH_MAX_US
#define IR_FULL_LEADER_HIGH_MAX_US 5000U
#endif

#ifndef IR_REPEAT_LEADER_HIGH_MIN_US
#define IR_REPEAT_LEADER_HIGH_MIN_US 2000U
#endif

#ifndef IR_REPEAT_LEADER_HIGH_MAX_US
#define IR_REPEAT_LEADER_HIGH_MAX_US 2500U
#endif

#ifndef IR_DATA_LOW_MIN_US
#define IR_DATA_LOW_MIN_US 400U
#endif

#ifndef IR_DATA_LOW_MAX_US
#define IR_DATA_LOW_MAX_US 750U
#endif

#ifndef IR_ZERO_HIGH_MIN_US
#define IR_ZERO_HIGH_MIN_US 400U
#endif

#ifndef IR_ZERO_HIGH_MAX_US
#define IR_ZERO_HIGH_MAX_US 750U
#endif

#ifndef IR_ONE_HIGH_MIN_US
#define IR_ONE_HIGH_MIN_US 1400U
#endif

#ifndef IR_ONE_HIGH_MAX_US
#define IR_ONE_HIGH_MAX_US 1900U
#endif

#ifndef IR_REPEAT_MAX_AGE_US
#define IR_REPEAT_MAX_AGE_US 150000U
#endif

#endif
