################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /vendor/common

OBJS += \
$(OUT_PATH)/vendor/common/config_model.o \
$(OUT_PATH)/vendor/common/app_beacon.o \
$(OUT_PATH)/vendor/common/app_health.o \
$(OUT_PATH)/vendor/common/app_heartbeat.o \
$(OUT_PATH)/vendor/common/app_provison.o \
$(OUT_PATH)/vendor/common/app_proxy.o \
$(OUT_PATH)/vendor/common/ble_ll_ota.o \
$(OUT_PATH)/vendor/common/blt_led.o \
$(OUT_PATH)/vendor/common/blt_soft_timer.o \
$(OUT_PATH)/vendor/common/cmd_interface.o \
$(OUT_PATH)/vendor/common/dual_mode_adapt.o \
$(OUT_PATH)/vendor/common/battery_check.o \
$(OUT_PATH)/vendor/common/ev.o \
$(OUT_PATH)/vendor/common/factory_reset.o \
$(OUT_PATH)/vendor/common/fast_provision_model.o \
$(OUT_PATH)/vendor/common/generic_model.o \
$(OUT_PATH)/vendor/common/light.o \
$(OUT_PATH)/vendor/common/lighting_model.o \
$(OUT_PATH)/vendor/common/lighting_model_HSL.o \
$(OUT_PATH)/vendor/common/lighting_model_LC.o \
$(OUT_PATH)/vendor/common/lighting_model_xyl.o \
$(OUT_PATH)/vendor/common/mesh_common.o \
$(OUT_PATH)/vendor/common/mesh_fn.o \
$(OUT_PATH)/vendor/common/mesh_lpn.o \
$(OUT_PATH)/vendor/common/mesh_node.o \
$(OUT_PATH)/vendor/common/mesh_property.o \
$(OUT_PATH)/vendor/common/mesh_test_cmd.o \
$(OUT_PATH)/vendor/common/myprintf.o \
$(OUT_PATH)/vendor/common/ota_fw_ow.o \
$(OUT_PATH)/vendor/common/scene.o \
$(OUT_PATH)/vendor/common/scheduler.o \
$(OUT_PATH)/vendor/common/sensors_model.o \
$(OUT_PATH)/vendor/common/system_time.o \
$(OUT_PATH)/vendor/common/time_model.o \
$(OUT_PATH)/vendor/common/user_ali.o \
$(OUT_PATH)/vendor/common/user_ali_time.o \
$(OUT_PATH)/vendor/common/user_proc.o \
$(OUT_PATH)/vendor/common/vendor_model.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/vendor/common/%.o: $(TEL_PATH)/vendor/common/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"



################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /vendor/common/nl_api

OBJS += \
$(OUT_PATH)/vendor/common/nl_api/nl_common.o \
$(OUT_PATH)/vendor/common/nl_api/nl_model_level.o \
$(OUT_PATH)/vendor/common/nl_api/nl_model_schedules.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/vendor/common/nl_api/%.o: mesh/nl_api/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
##############################################
##################################
OUT_DIR += /vendor/common/mi_api/mijia_ble_api

OBJS += \
$(OUT_PATH)/vendor/common/mi_api/mijia_ble_api/mible_api.o 

$(OUT_PATH)/vendor/common/mi_api/mijia_ble_api%.o: $(TEL_PATH)/vendor/common/mi_api/mijia_ble_api/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"

OUT_DIR += /vendor/common/mi_api

OBJS += \
$(OUT_PATH)/vendor/common/mi_api/mijia_pub_proc.o \
$(OUT_PATH)/vendor/common/mi_api/telink_sdk_mible_api.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/vendor/common/mi_api%.o: $(TEL_PATH)/vendor/common/mi_api/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################
OUT_DIR += /vendor/common/mi_api/libs
OBJS += $(OUT_PATH)/vendor/common/mi_api/libs/mi_config.o 

################################################################################

################################################################################

OUT_DIR += /vendor/common/mi_api/libs/third_party/micro-ecc


OBJS += $(OUT_PATH)/vendor/common/mi_api/libs/third_party/micro-ecc/uECC.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)vendor/common/mi_api/libs/third_party/micro-ecc%.o: $(TEL_PATH)/vendor/common/mi_api/libs/third_party/micro-ecc/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################
OUT_DIR += /vendor/common/mi_api/mi_vendor
OBJS += $(OUT_PATH)/vendor/common/mi_api/mi_vendor/vendor_model_mi.o

# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)vendor/common/mi_api/mi_vendor/%.o: $(TEL_PATH)/vendor/common/mi_api/mi_vendor/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"
