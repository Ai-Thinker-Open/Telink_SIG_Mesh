################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /proj/app

OBJS += \
$(OUT_PATH)/proj/app/usbaud.o \
$(OUT_PATH)/proj/app/usbcdc.o \
$(OUT_PATH)/proj/app/usbkb.o \
$(OUT_PATH)/proj/app/usbmouse.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/proj/app/%.o: $(TEL_PATH)/components/proj/app/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /proj/common

OBJS += \
$(OUT_PATH)/proj/common/breakpoint.o \
$(OUT_PATH)/proj/common/compatibility.o \
$(OUT_PATH)/proj/common/list.o \
$(OUT_PATH)/proj/common/log.o \
$(OUT_PATH)/proj/common/mempool.o \
$(OUT_PATH)/proj/common/mmem.o \
$(OUT_PATH)/proj/common/printf.o \
$(OUT_PATH)/proj/common/qsort.o \
$(OUT_PATH)/proj/common/selection_sort.o \
$(OUT_PATH)/proj/common/tstring.o \
$(OUT_PATH)/proj/common/tutility.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/proj/common/%.o: $(TEL_PATH)/components/proj/common/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /proj/drivers

OBJS += \
$(OUT_PATH)/proj/drivers/rf_pa.o \
$(OUT_PATH)/proj/drivers/usb.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/proj/drivers/%.o: $(TEL_PATH)/components/proj/drivers/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /proj/mcu

OBJS += \
$(OUT_PATH)/proj/mcu/putchar.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/proj/mcu/%.o: $(TEL_PATH)/components/proj/mcu/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# OUT_DIR += /proj/mcu_spec

# OBJS += \
# $(OUT_PATH)/proj/mcu_spec/adc_8263.o \
# $(OUT_PATH)/proj/mcu_spec/adc_8266.o \
# $(OUT_PATH)/proj/mcu_spec/adc_8267.o \
# $(OUT_PATH)/proj/mcu_spec/cstartup_8261.o \
# $(OUT_PATH)/proj/mcu_spec/cstartup_8263.o \
# $(OUT_PATH)/proj/mcu_spec/cstartup_8263_ram.o \
# $(OUT_PATH)/proj/mcu_spec/cstartup_8266.o \
# $(OUT_PATH)/proj/mcu_spec/cstartup_8266_ram.o \
# $(OUT_PATH)/proj/mcu_spec/cstartup_8267.o \
# $(OUT_PATH)/proj/mcu_spec/cstartup_8267_ram.o \
# $(OUT_PATH)/proj/mcu_spec/cstartup_8269.o \
# $(OUT_PATH)/proj/mcu_spec/cstartup_8269_ram.o \
# $(OUT_PATH)/proj/mcu_spec/gpio_8263.o \
# $(OUT_PATH)/proj/mcu_spec/gpio_8266.o \
# $(OUT_PATH)/proj/mcu_spec/gpio_8267.o \
# $(OUT_PATH)/proj/mcu_spec/pwm_8263.o \
# $(OUT_PATH)/proj/mcu_spec/pwm_8266.o \
# $(OUT_PATH)/proj/mcu_spec/pwm_8267.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/proj/mcu_spec/%.o: $(TEL_PATH)/components/proj/mcu_spec/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"

proj/mcu_spec/%.o: ../proj/mcu_spec/%.S
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


