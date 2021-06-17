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
$(OUT_PATH)/proj/app/%.o: $(TEL_PATH)/proj/app/%.c
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
$(OUT_PATH)/proj/common/%.o: $(TEL_PATH)/proj/common/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /proj/drivers

OBJS += \
$(OUT_PATH)/proj/drivers/rf_pa.o \
$(OUT_PATH)/proj/drivers/keyboard.o \
$(OUT_PATH)/proj/drivers/usb.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/proj/drivers/%.o: $(TEL_PATH)/proj/drivers/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /proj/mcu

OBJS += $(OUT_PATH)/proj/mcu/putchar.o 

# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/proj/mcu/%.o: $(TEL_PATH)/proj/mcu/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"

