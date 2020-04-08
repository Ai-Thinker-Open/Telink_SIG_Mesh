################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /boot/8258

OBJS += \
$(OUT_PATH)/boot/8258/cstartup_8258.o \
$(OUT_PATH)/boot/8258/cstartup_8258_RET_16K.o \
$(OUT_PATH)/boot/8258/cstartup_8258_RET_32K.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/boot/8258/%.o: $(TEL_PATH)/SDK_3.1.5/boot/8258/%.S
	@echo 'Building file: $<'
	@tc32-elf-gcc -DMCU_STARTUP_8258_RET_16K -D__PROJECT_MESH__=1 -DCHIP_TYPE=CHIP_TYPE_8258 $(INCLUDE_PATHS) -c -o"$@" "$<"



