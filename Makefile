.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

TOOLCHAIN_PREFIX ?= arm-none-eabi-
CC := $(TOOLCHAIN_PREFIX)gcc
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
OBJDUMP := $(TOOLCHAIN_PREFIX)objdump
SIZE := $(TOOLCHAIN_PREFIX)size

TARGET := firmware
override BUILD_DIR := BUILD
LINKER_SCRIPT := STARTUP/STM32F103C6.ld
CPU_FLAGS := -mcpu=cortex-m3 -mthumb
WARNINGS := -Wall -Wextra -Werror -pedantic -Wconversion -Wshadow -Wenum-conversion

C_SOURCES := \
    02_MCAL/01_RCC/RCC_program.c \
    02_MCAL/02_DIO/DIO_program.c \
    02_MCAL/03_NVIC/NVIC_program.c \
    02_MCAL/04_EXTI/EXTI_program.c \
    02_MCAL/05_AFIO/AFIO_program.c \
    02_MCAL/06_STK/STK_program.c \
    02_MCAL/07_DMA/DMA_program.c \
    02_MCAL/08_SPI/SPI_program.c \
    02_MCAL/09_USART/USART_program.c \
    02_MCAL/10_FPEC/FPEC_program.c \
    02_MCAL/11_ADC/ADC_program.c \
    03_HAL/01_HLEDMAT/HLEDMAT_program.c \
    03_HAL/02_IR_Communication/IR_Communication_program.c \
    03_HAL/03_DAC/DAC_program.c \
    03_HAL/04_STP/STP_program.c \
    03_HAL/05_TFT/TFT_program.c \
    03_HAL/06_ESP/ESP_program.c \
    APP/main.c
ASM_SOURCE := STARTUP/startup_stm32f103c6.s

# OS_program.h is compiled only through APP/main.c.
INCLUDE_DIRS := 01_LIB $(sort $(dir $(C_SOURCES))) 04_SERVICES
CPPFLAGS := $(addprefix -I,$(INCLUDE_DIRS))
CFLAGS := $(CPU_FLAGS) -std=c99 $(WARNINGS) -Os -g \
    -ffreestanding -ffunction-sections -fdata-sections -MMD -MP
ASFLAGS := $(CPU_FLAGS) -g -Wa,--fatal-warnings
# Supply compiler helpers explicitly without hosted startup or libraries.
LDFLAGS := $(CPU_FLAGS) -nostdlib -T$(LINKER_SCRIPT) \
    -Wl,--gc-sections,--fatal-warnings,--print-memory-usage \
    -Wl,-Map=$(BUILD_DIR)/$(TARGET).map
LDLIBS := -lgcc

C_OBJECTS := $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
ASM_OBJECT := $(BUILD_DIR)/$(ASM_SOURCE:.s=.o)
OBJECTS := $(C_OBJECTS) $(ASM_OBJECT)
ELF := $(BUILD_DIR)/$(TARGET).elf
HEX := $(BUILD_DIR)/$(TARGET).hex
BIN := $(BUILD_DIR)/$(TARGET).bin

ifeq ($(OS),Windows_NT)
SHELL := cmd.exe
.SHELLFLAGS := /d /c
MAKE_DIR = if not exist "$(subst /,\,$(@D))" mkdir "$(subst /,\,$(@D))"
else
MAKE_DIR = mkdir -p "$(@D)"
endif

.PHONY: all clean size disasm
all: $(ELF) $(HEX) $(BIN) size

$(BUILD_DIR)/%.o: %.c Makefile
	@$(MAKE_DIR)
	"$(CC)" $(CPPFLAGS) $(CFLAGS) -c "$<" -o "$@"

$(ASM_OBJECT): $(ASM_SOURCE) Makefile
	@$(MAKE_DIR)
	"$(CC)" $(ASFLAGS) -c "$<" -o "$@"

$(ELF): $(OBJECTS) $(LINKER_SCRIPT) Makefile
	"$(CC)" $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o "$@"

$(HEX): $(ELF)
	"$(OBJCOPY)" -O ihex "$<" "$@"

$(BIN): $(ELF)
	"$(OBJCOPY)" -O binary "$<" "$@"

size: $(ELF)
	"$(SIZE)" -B "$<"

disasm: $(ELF)
	"$(OBJDUMP)" -d -S "$<" > "$(BUILD_DIR)/$(TARGET).lst"

clean:
ifeq ($(OS),Windows_NT)
	powershell.exe -NoProfile -NonInteractive -Command "$$ErrorActionPreference = 'Stop'; $$target = Join-Path (Get-Location).Path 'BUILD'; if (Test-Path -LiteralPath $$target) { $$item = Get-Item -LiteralPath $$target; if ((($$item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) -or ((Resolve-Path -LiteralPath $$target).Path -ne $$target)) { throw 'Refusing unsafe BUILD cleanup' }; Remove-Item -LiteralPath $$target -Recurse -Force }"
else
	rm -rf -- ./BUILD
endif

-include $(C_OBJECTS:.o=.d)
