# Makefile for STM32F746G-DISCO Project

# Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
AS = $(PREFIX)gcc -x assembler-with-cpp
LD = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size
GDB = $(PREFIX)gdb

# Project name
PROJECT = stm32f746_camera_ov5640

# Build directory
BUILD_DIR = build

# Target MCU
MCU = cortex-m7
FPU = fpv5-sp-d16
FLOAT-ABI = hard

# CMSIS and HAL paths
CMSIS_DIR = Drivers/CMSIS
HAL_DIR = Drivers/STM32F7xx_HAL_Driver

# Startup file (corrected path)
STARTUP_ASM = ./Core/Startup/startup_stm32f746nghx.s

# Source files
C_SOURCES = \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_adc.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_adc_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_crc.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_crc_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dcmi.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dcmi_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma2d.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dsi.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_eth.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_exti.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_hcd.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ltdc.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ltdc_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_qspi.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rtc.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rtc_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sai.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sai_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sd.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sdram.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_spdifrx.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_spi.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_spi_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart_ex.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_fmc.c \
	./Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_sdmmc.c \
	./Core/Src/font12.c \
	./Core/Src/font16.c \
	./Core/Src/font20.c \
	./Core/Src/font24.c \
	./Core/Src/font8.c \
	./Core/Src/main.c \
	./Core/Src/ov5640.c \
	./Core/Src/stm32746g_discovery.c \
	./Core/Src/stm32746g_discovery_sdram.c \
	./Core/Src/stm32f7xx_hal_msp.c \
	./Core/Src/stm32f7xx_it.c \
	./Core/Src/syscalls.c \
	./Core/Src/sysmem.c \
	./Core/Src/system_stm32f7xx.c

# Include paths
C_INCLUDES = \
	-I./Core/Inc \
	-I./Drivers/STM32F7xx_HAL_Driver/Inc \
	-I./Drivers/STM32F7xx_HAL_Driver/Inc/Legacy \
	-I./Drivers/CMSIS/Device/ST/STM32F7xx/Include \
	-I./Drivers/CMSIS/Include

# Compiler flags
CFLAGS = -mcpu=$(MCU) -mthumb -mfpu=$(FPU) -mfloat-abi=$(FLOAT-ABI)
CFLAGS += -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F746xx -c
CFLAGS += -Wall -fdata-sections -ffunction-sections -fstack-usage -MMD -MP
CFLAGS += -g -O0
CFLAGS += $(C_INCLUDES)

# Linker flags
LDFLAGS = -mcpu=$(MCU) -mthumb -mfpu=$(FPU) -mfloat-abi=$(FLOAT-ABI) 
LDFLAGS += -specs=nano.specs -specs=nosys.specs
LDFLAGS += -TSTM32F746NGHX_FLASH.ld
LDFLAGS += -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map
# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# Object files
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

# ASM objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(STARTUP_ASM:.s=.o)))
vpath %.s $(sort $(dir $(STARTUP_ASM)))

# Build targets
all: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).hex $(BUILD_DIR)/$(PROJECT).bin

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS) Makefile
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(OBJCOPY) -O binary -S $< $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	-rm -fR $(BUILD_DIR)

# Flash using OpenOCD
flash: $(BUILD_DIR)/$(PROJECT).elf
	openocd -f board/stm32f7discovery.cfg \
	-c "program $< verify reset exit"

# Debug with GDB
debug: $(BUILD_DIR)/$(PROJECT).elf
	$(GDB) -ex "target extended-remote :3333" \
	-ex "monitor reset halt" \
	-ex "load" \
	-ex "monitor reset init" \
	$<

# Flash using st-flash
write: $(BUILD_DIR)/$(PROJECT).bin
	st-flash write  $(BUILD_DIR)/$(PROJECT).bin 0x08000000

# Include dependency files
-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean flash debug