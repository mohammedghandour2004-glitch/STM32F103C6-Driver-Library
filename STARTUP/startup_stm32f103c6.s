/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< startup_stm32f103c6.s >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : STARTUP
 * Component   : STARTUP
 * Description : This file contains the Cortex-M3 vector table and reset startup routine.
 */

.syntax unified
.cpu cortex-m3
.thumb

.extern main
.extern _estack
.extern _sidata
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss

.section .isr_vector, "a", %progbits
.balign 512
.global g_pfnVectors
.type g_pfnVectors, %object
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    .word WWDG_IRQHandler
    .word PVD_IRQHandler
    .word TAMPER_IRQHandler
    .word RTC_IRQHandler
    .word FLASH_IRQHandler
    .word RCC_IRQHandler
    .word EXTI0_IRQHandler
    .word EXTI1_IRQHandler
    .word EXTI2_IRQHandler
    .word EXTI3_IRQHandler
    .word EXTI4_IRQHandler
    .word DMA1_Channel1_IRQHandler
    .word DMA1_Channel2_IRQHandler
    .word DMA1_Channel3_IRQHandler
    .word DMA1_Channel4_IRQHandler
    .word DMA1_Channel5_IRQHandler
    .word DMA1_Channel6_IRQHandler
    .word DMA1_Channel7_IRQHandler
    .word ADC1_2_IRQHandler
    .word USB_HP_CAN1_TX_IRQHandler
    .word USB_LP_CAN1_RX0_IRQHandler
    .word CAN1_RX1_IRQHandler
    .word CAN1_SCE_IRQHandler
    .word EXTI9_5_IRQHandler
    .word TIM1_BRK_IRQHandler
    .word TIM1_UP_IRQHandler
    .word TIM1_TRG_COM_IRQHandler
    .word TIM1_CC_IRQHandler
    .word TIM2_IRQHandler
    .word TIM3_IRQHandler
    .word 0    /* Reserved IRQ30. */
    .word I2C1_EV_IRQHandler
    .word I2C1_ER_IRQHandler
    .word 0    /* Reserved IRQ33. */
    .word 0    /* Reserved IRQ34. */
    .word SPI1_IRQHandler
    .word 0    /* Reserved IRQ36. */
    .word USART1_IRQHandler
    .word USART2_IRQHandler
    .word 0    /* Reserved IRQ39. */
    .word EXTI15_10_IRQHandler
    .word RTCAlarm_IRQHandler
    .word USBWakeUp_IRQHandler
.size g_pfnVectors, . - g_pfnVectors

.section .text.Reset_Handler, "ax", %progbits
.balign 4
.global Reset_Handler
.type Reset_Handler, %function
.thumb_func
/* Initializes C data and enters the application. */
Reset_Handler:
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
.Lcopy_data:
    cmp r1, r2
    bhs .Lzero_setup
    ldr r3, [r0], #4
    str r3, [r1], #4
    b .Lcopy_data

.Lzero_setup:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0
.Lzero_bss:
    cmp r1, r2
    bhs .Lenter_main
    str r3, [r1], #4
    b .Lzero_bss

.Lenter_main:
    bl main
.Lmain_returned:
    b .Lmain_returned
.size Reset_Handler, . - Reset_Handler
.ltorg

.section .text.Default_Handler, "ax", %progbits
.balign 4
.global Default_Handler
.type Default_Handler, %function
.thumb_func
/* Traps unimplemented exceptions and interrupts. */
Default_Handler:
    b Default_Handler
.size Default_Handler, . - Default_Handler

/* Allows application handlers to override the default Thumb target. */
.macro WEAK_HANDLER name
    .weak \name
    .thumb_set \name, Default_Handler
.endm

WEAK_HANDLER NMI_Handler
WEAK_HANDLER HardFault_Handler
WEAK_HANDLER MemManage_Handler
WEAK_HANDLER BusFault_Handler
WEAK_HANDLER UsageFault_Handler
WEAK_HANDLER SVC_Handler
WEAK_HANDLER DebugMon_Handler
WEAK_HANDLER PendSV_Handler
WEAK_HANDLER SysTick_Handler

WEAK_HANDLER WWDG_IRQHandler
WEAK_HANDLER PVD_IRQHandler
WEAK_HANDLER TAMPER_IRQHandler
WEAK_HANDLER RTC_IRQHandler
WEAK_HANDLER FLASH_IRQHandler
WEAK_HANDLER RCC_IRQHandler
WEAK_HANDLER EXTI0_IRQHandler
WEAK_HANDLER EXTI1_IRQHandler
WEAK_HANDLER EXTI2_IRQHandler
WEAK_HANDLER EXTI3_IRQHandler
WEAK_HANDLER EXTI4_IRQHandler
WEAK_HANDLER DMA1_Channel1_IRQHandler
WEAK_HANDLER DMA1_Channel2_IRQHandler
WEAK_HANDLER DMA1_Channel3_IRQHandler
WEAK_HANDLER DMA1_Channel4_IRQHandler
WEAK_HANDLER DMA1_Channel5_IRQHandler
WEAK_HANDLER DMA1_Channel6_IRQHandler
WEAK_HANDLER DMA1_Channel7_IRQHandler
WEAK_HANDLER ADC1_2_IRQHandler
WEAK_HANDLER USB_HP_CAN1_TX_IRQHandler
WEAK_HANDLER USB_LP_CAN1_RX0_IRQHandler
WEAK_HANDLER CAN1_RX1_IRQHandler
WEAK_HANDLER CAN1_SCE_IRQHandler
WEAK_HANDLER EXTI9_5_IRQHandler
WEAK_HANDLER TIM1_BRK_IRQHandler
WEAK_HANDLER TIM1_UP_IRQHandler
WEAK_HANDLER TIM1_TRG_COM_IRQHandler
WEAK_HANDLER TIM1_CC_IRQHandler
WEAK_HANDLER TIM2_IRQHandler
WEAK_HANDLER TIM3_IRQHandler
WEAK_HANDLER I2C1_EV_IRQHandler
WEAK_HANDLER I2C1_ER_IRQHandler
WEAK_HANDLER SPI1_IRQHandler
WEAK_HANDLER USART1_IRQHandler
WEAK_HANDLER USART2_IRQHandler
WEAK_HANDLER EXTI15_10_IRQHandler
WEAK_HANDLER RTCAlarm_IRQHandler
WEAK_HANDLER USBWakeUp_IRQHandler
