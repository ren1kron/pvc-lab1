# Native tests use the host C compiler; firmware uses Arm GNU (or CubeIDE's GCC).
MCU ?= F407
CONFIG ?= Debug
CROSS_COMPILE ?= arm-none-eabi-
HOST_CC ?= cc
PYTHON ?= python3

ifeq ($(filter $(MCU),F407 F427),)
$(error MCU must be F407 or F427)
endif
ifeq ($(filter $(CONFIG),Debug Release),)
$(error CONFIG must be Debug or Release)
endif
ifeq ($(MCU),F407)
PROJECT_DIR := firmware
PART := STM32F407VGTX
else
PROJECT_DIR := firmware-f427
PART := STM32F427VITX
endif

OUT := build/$(MCU)/$(CONFIG)
TARGET := $(OUT)/Lab1_$(MCU)
CPU := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
INCLUDES := -Ifirmware/Core/Inc -Ifirmware/Drivers/STM32F4xx_HAL_Driver/Inc \
 -Ifirmware/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy \
 -Ifirmware/Drivers/CMSIS/Device/ST/STM32F4xx/Include -Ifirmware/Drivers/CMSIS/Include
OPT := $(if $(filter Debug,$(CONFIG)),-Og,-Os)
CFLAGS := $(CPU) $(INCLUDES) -DSTM32$(MCU)xx -DUSE_HAL_DRIVER -std=c11 \
 $(OPT) -g3 -Wall -Wextra -Werror -ffunction-sections -fdata-sections -fno-common
SOURCES := $(wildcard firmware/Core/Src/*.c) $(wildcard firmware/Drivers/STM32F4xx_HAL_Driver/Src/*.c)
STARTUP := $(wildcard $(PROJECT_DIR)/Startup/*.s)
OBJECTS := $(addprefix $(OUT)/,$(SOURCES:.c=.o)) $(OUT)/startup.o
LDFLAGS := $(CPU) -T$(PROJECT_DIR)/$(PART)_FLASH.ld --specs=nano.specs \
 --specs=nosys.specs -Wl,--gc-sections,-Map=$(TARGET).map,--print-memory-usage

.PHONY: all firmware test test-model test-sanitize test-capacity64 simulate audit clean
all: firmware
firmware: $(TARGET).elf $(TARGET).bin $(TARGET).hex

$(TARGET).elf: $(OBJECTS) $(PROJECT_DIR)/$(PART)_FLASH.ld
	$(CROSS_COMPILE)gcc $(OBJECTS) $(LDFLAGS) -o $@
	$(CROSS_COMPILE)size $@
$(OUT)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CROSS_COMPILE)gcc $(CFLAGS) $(if $(findstring /Drivers/,$<),-Wno-unused-parameter,) -MMD -MP -c $< -o $@
$(OUT)/startup.o: $(STARTUP)
	@mkdir -p $(dir $@)
	$(CROSS_COMPILE)gcc $(CPU) -g3 -x assembler-with-cpp -c $< -o $@
$(TARGET).bin: $(TARGET).elf
	$(CROSS_COMPILE)objcopy -O binary $< $@
$(TARGET).hex: $(TARGET).elf
	$(CROSS_COMPILE)objcopy -O ihex $< $@

HOST_FLAGS := -std=c11 -g -O1 -Wall -Wextra -Werror -Wpedantic \
 -Itests/fake_hal -Itests -Ifirmware/Core/Inc -Isimulator
LOGIC := firmware/Core/Src/led.c firmware/Core/Src/button.c firmware/Core/Src/morse.c
HOST_SOURCES := tests/test_main.c tests/fake_hal/fake_hal.c $(LOGIC)
test: test-model
	@mkdir -p build/host
	$(HOST_CC) $(HOST_FLAGS) $(HOST_SOURCES) -o build/host/tests
	./build/host/tests
test-model:
	@mkdir -p build/host
	$(HOST_CC) $(HOST_FLAGS) tests/test_gpio_model.c simulator/gpio_model.c -o build/host/test-model
	./build/host/test-model
test-sanitize:
	@mkdir -p build/host
	$(HOST_CC) $(HOST_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer $(HOST_SOURCES) -o build/host/tests-sanitize
	./build/host/tests-sanitize
	$(HOST_CC) $(HOST_FLAGS) -fsanitize=address,undefined tests/test_gpio_model.c simulator/gpio_model.c -o build/host/model-sanitize
	./build/host/model-sanitize
test-capacity64:
	@mkdir -p build/host
	$(HOST_CC) $(HOST_FLAGS) -DMORSE_CAPACITY=64 $(HOST_SOURCES) -o build/host/tests64
	./build/host/tests64
simulate:
	@mkdir -p build/host
	$(HOST_CC) $(HOST_FLAGS) -DGPIO_REGISTER_MODEL simulator/morse_demo.c simulator/gpio_model.c $(LOGIC) -o build/host/gpio-demo
	./build/host/gpio-demo > build/gpio_trace.csv
	@echo "Trace: build/gpio_trace.csv"
audit:
	$(PYTHON) scripts/audit.py
clean:
	rm -rf build

-include $(OBJECTS:.o=.d)
