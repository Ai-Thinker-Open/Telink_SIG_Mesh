################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /mesh

OBJS += \
$(OUT_PATH)/mesh/app_beacon.o \
$(OUT_PATH)/mesh/app_health.o \
$(OUT_PATH)/mesh/app_heartbeat.o \
$(OUT_PATH)/mesh/app_provison.o \
$(OUT_PATH)/mesh/app_proxy.o \
$(OUT_PATH)/mesh/ble_ll_ota.o \
$(OUT_PATH)/mesh/blt_led.o \
$(OUT_PATH)/mesh/blt_soft_timer.o \
$(OUT_PATH)/mesh/cmd_interface.o \
$(OUT_PATH)/mesh/dual_mode_adapt.o \
$(OUT_PATH)/mesh/ev.o \
$(OUT_PATH)/mesh/factory_reset.o \
$(OUT_PATH)/mesh/fast_provision_model.o \
$(OUT_PATH)/mesh/generic_model.o \
$(OUT_PATH)/mesh/light.o \
$(OUT_PATH)/mesh/lighting_model.o \
$(OUT_PATH)/mesh/lighting_model_HSL.o \
$(OUT_PATH)/mesh/lighting_model_LC.o \
$(OUT_PATH)/mesh/lighting_model_xyl.o \
$(OUT_PATH)/mesh/mesh_common.o \
$(OUT_PATH)/mesh/mesh_fn.o \
$(OUT_PATH)/mesh/mesh_lpn.o \
$(OUT_PATH)/mesh/mesh_node.o \
$(OUT_PATH)/mesh/mesh_ota.o \
$(OUT_PATH)/mesh/mesh_property.o \
$(OUT_PATH)/mesh/mesh_test_cmd.o \
$(OUT_PATH)/mesh/myprintf.o \
$(OUT_PATH)/mesh/ota_fw_ow.o \
$(OUT_PATH)/mesh/remote_prov.o \
$(OUT_PATH)/mesh/scene.o \
$(OUT_PATH)/mesh/scheduler.o \
$(OUT_PATH)/mesh/sensors_model.o \
$(OUT_PATH)/mesh/system_time.o \
$(OUT_PATH)/mesh/time_model.o \
$(OUT_PATH)/mesh/user_ali.o \
$(OUT_PATH)/mesh/user_ali_time.o \
$(OUT_PATH)/mesh/user_proc.o \
$(OUT_PATH)/mesh/vendor_model.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/mesh/%.o: mesh/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"



################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /mesh/nl_api

OBJS += \
$(OUT_PATH)/mesh/nl_api/nl_common.o \
$(OUT_PATH)/mesh/nl_api/nl_model_level.o \
$(OUT_PATH)/mesh/nl_api/nl_model_schedules.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/mesh/nl_api/%.o: mesh/nl_api/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /mesh/mi_api

OBJS += \
$(OUT_PATH)/mesh/mi_api/mible_api.o \
$(OUT_PATH)/mesh/mi_api/mijia_pub_proc.o \
$(OUT_PATH)/mesh/mi_api/telink_sdk_mible_api.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)mesh/mi_api/%.o: mesh/mi_api/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################
OUT_DIR += /mesh/mi_api/certi
OBJS += $(OUT_PATH)/mesh/mi_api/certi/mi_config.o 

################################################################################

# OUT_DIR += /mesh/mi_api/certi/common

# OBJS += \
# $(OUT_PATH)/mesh/mi_api/certi/common/crc32.o \
# $(OUT_PATH)/mesh/mi_api/certi/common/mible_beacon.o \
# $(OUT_PATH)/mesh/mi_api/certi/common/mible_crypto.o \
# $(OUT_PATH)/mesh/mi_api/certi/common/mible_rxfer.o \
# $(OUT_PATH)/mesh/mi_api/certi/common/queue.o \
# $(OUT_PATH)/mesh/mi_api/certi/common/tlv.o 

################################################################################

OUT_DIR += /mesh/mi_api/certi/cryptography

OBJS += $(OUT_PATH)/mesh/mi_api/certi/cryptography/mi_crypto_backend_uECC.o


OUT_DIR += /mesh/mi_api/certi/third_party/micro-ecc

OBJS += $(OUT_PATH)/mesh/mi_api/certi/third_party/micro-ecc/uECC.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)mesh/mi_api/certi/%.o: mesh/mi_api/certi/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################
# OUT_DIR += /mesh/mi_api/mi_vendor
# OBJS += $(OUT_PATH)/mesh/mi_api/mi_vendor/vendor_model_mi.o

# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)mesh/mi_api/mi_vendor/%.o: mesh/mi_api/mi_vendor/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"
