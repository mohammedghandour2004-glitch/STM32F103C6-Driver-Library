<div align="center">

<img src="https://capsule-render.vercel.app/api?type=waving&color=0:0F2027,50:203A43,100:2C5364&height=200&section=header&text=STM32F103C6%20Driver%20Library&fontSize=42&fontColor=FFFFFF&animation=fadeIn&fontAlignY=35&desc=Bare-Metal%20Embedded%20C%20%7C%20ARM%20Cortex-M3&descAlignY=55&descSize=16" />

<img src="https://readme-typing-svg.demolab.com?font=Fira+Code&weight=600&size=19&pause=900&color=00D9FF&center=true&vCenter=true&width=700&lines=Register-Level+STM32+Programming;MCAL+%2B+HAL+%2B+Cooperative+Scheduler;Custom+Startup+%2B+Linker+Script;17+Driver+Implementations;0+Errors+%7C+0+Warnings" />

<br>

![C](https://img.shields.io/badge/C-C99-00599C?style=for-the-badge&logo=c&logoColor=white)
![STM32](https://img.shields.io/badge/STM32-F103C6-03234B?style=for-the-badge&logo=stmicroelectronics&logoColor=white)
![ARM](https://img.shields.io/badge/ARM-Cortex--M3-0091BD?style=for-the-badge&logo=arm&logoColor=white)
![Bare Metal](https://img.shields.io/badge/Bare--Metal-Register_Level-00A86B?style=for-the-badge)
![Build](https://img.shields.io/badge/BUILD-PASSING-39FF14?style=for-the-badge&logo=gnu&logoColor=black)

<br>

`No STM32 HAL` • `No LL` • `No Arduino` • `Pure Register-Level C`

</div>

---

## ⚡ Overview

A reusable **bare-metal C99 driver library** for the **STM32F103C6** based on direct register-level programming.

Built as a clean foundation for future **embedded systems projects, simulations, and hardware experiments**.

---

## 🧩 Architecture

```text
STM32F103C6-Driver-Library
│
├── 01_LIB       → Common types, bit utilities & error status
├── 02_MCAL      → RCC • DIO • NVIC • EXTI • AFIO • STK
│                  DMA • SPI • USART • FPEC • ADC
│
├── 03_HAL       → LED Matrix • IR • DAC • 74HC595 • TFT • ESP
├── 04_SERVICES  → Cooperative Scheduler
├── APP          → Minimal build-validation harness
├── STARTUP      → Vector Table + Reset Handler + Linker Script
└── Makefile     → GNU Arm build system
