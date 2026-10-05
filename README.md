<div align="center">

# ⚡ STM32F103C6 Driver Library

<img src="https://readme-typing-svg.demolab.com?font=Fira+Code&weight=600&size=18&pause=1000&color=36BCF7&center=true&vCenter=true&width=650&lines=Bare-Metal+Embedded+C;STM32F103C6+%7C+Cortex-M3;MCAL+%2B+HAL+%2B+Cooperative+Scheduler;0+Errors+%7C+0+Warnings" />

<br>

![C](https://img.shields.io/badge/C-C99-blue?style=for-the-badge&logo=c)
![STM32](https://img.shields.io/badge/STM32-F103C6-03234B?style=for-the-badge&logo=stmicroelectronics)
![Bare Metal](https://img.shields.io/badge/Bare--Metal-No_HAL-success?style=for-the-badge)
![Build](https://img.shields.io/badge/Build-Passing-brightgreen?style=for-the-badge)

</div>

---

## 🚀 Overview

A **bare-metal C99 driver library** for the **STM32F103C6 (ARM Cortex-M3)**, built from scratch using direct register-level programming.

No STM32 HAL. No LL. No Arduino.

---

## 🧩 Architecture

```text
01_LIB       → Common types & bit utilities
02_MCAL      → RCC, DIO, NVIC, EXTI, AFIO, STK, DMA, SPI, USART, FPEC, ADC
03_HAL       → LED Matrix, IR, DAC, 74HC595, TFT, ESP
04_SERVICES  → Cooperative Scheduler
APP          → Minimal build harness
STARTUP      → Vector table + Linker Script
