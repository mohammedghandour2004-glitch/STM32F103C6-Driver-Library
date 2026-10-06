/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< BIT_MATH.h >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : 01_LIB
 * Component   : BIT_MATH
 * Description : This file contains 32-bit bit-manipulation macros for the STM32 target.
 */

#ifndef BIT_MATH_H
#define BIT_MATH_H

/* For unsigned 32-bit register/value operations on the STM32 target.
 * Valid bit indices are 0..31; other indices and u64 operands are unsupported.
 * These macros are not a generic u64 bit-manipulation facility. */
#define BIT_MASK(BIT)                 (1U << (BIT))

#define SET_BIT(REG, BIT)             ((REG) |= BIT_MASK(BIT))

#define CLEAR_BIT(REG, BIT)           ((REG) &= ~BIT_MASK(BIT))

#define TOGGLE_BIT(REG, BIT)          ((REG) ^= BIT_MASK(BIT))

#define GET_BIT(REG, BIT)             (((REG) >> (BIT)) & 1U)

#endif /* BIT_MATH_H */
