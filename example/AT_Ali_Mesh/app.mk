################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OBJS += \
$(OUT_PATH)/app.o \
$(OUT_PATH)/app_att.o \
$(OUT_PATH)/app_uart.o \
$(OUT_PATH)/at_cmd.o \
$(OUT_PATH)/at_cmdHandle.o \
$(OUT_PATH)/app_model.o \
$(OUT_PATH)/main.o

# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/%.o: app/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"