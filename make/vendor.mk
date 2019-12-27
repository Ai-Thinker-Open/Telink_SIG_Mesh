################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /vendor/common

OBJS += \
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
$(OUT_PATH)/vendor/common/mesh_ota.o \
$(OUT_PATH)/vendor/common/mesh_property.o \
$(OUT_PATH)/vendor/common/mesh_test_cmd.o \
$(OUT_PATH)/vendor/common/myprintf.o \
$(OUT_PATH)/vendor/common/ota_fw_ow.o \
$(OUT_PATH)/vendor/common/remote_prov.o \
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
$(OUT_PATH)/vendor/common/%.o: $(TEL_PATH)/components/vendor/common/%.c
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
$(OUT_PATH)/vendor/common/nl_api/%.o: $(TEL_PATH)/components/vendor/common/nl_api/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OUT_DIR += /vendor/common/mi_api

OBJS += \
$(OUT_PATH)/vendor/common/mi_api/mible_api.o \
$(OUT_PATH)/vendor/common/mi_api/mijia_pub_proc.o \
$(OUT_PATH)/vendor/common/mi_api/telink_sdk_mible_api.o 


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)vendor/common/mi_api/%.o: $(TEL_PATH)/components/vendor/common/mi_api/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################
OUT_DIR += /vendor/common/mi_api/certi
OBJS += $(OUT_PATH)/vendor/common/mi_api/certi/mi_config.o 

################################################################################

OUT_DIR += /vendor/common/mi_api/certi/common

OBJS += \
$(OUT_PATH)/vendor/common/mi_api/certi/common/crc32.o \
$(OUT_PATH)/vendor/common/mi_api/certi/common/mible_beacon.o \
$(OUT_PATH)/vendor/common/mi_api/certi/common/mible_crypto.o \
$(OUT_PATH)/vendor/common/mi_api/certi/common/mible_rxfer.o \
$(OUT_PATH)/vendor/common/mi_api/certi/common/queue.o \
$(OUT_PATH)/vendor/common/mi_api/certi/common/tlv.o 

################################################################################

OUT_DIR += /vendor/common/mi_api/certi/cryptography

OBJS += \
$(OUT_PATH)/vendor/common/mi_api/certi/cryptography/mi_crypto.o \
$(OUT_PATH)/vendor/common/mi_api/certi/cryptography/mi_crypto_backend_mbedtls.o \
$(OUT_PATH)/vendor/common/mi_api/certi/cryptography/mi_crypto_backend_msc.o \
$(OUT_PATH)/vendor/common/mi_api/certi/cryptography/mi_crypto_backend_uECC.o \
$(OUT_PATH)/vendor/common/mi_api/certi/cryptography/mi_mesh_otp.o 

################################################################################

OUT_DIR += /vendor/common/mi_api/certi/gatt_dfu

OBJS += \
$(OUT_PATH)/vendor/common/mi_api/certi/gatt_dfu/mible_dfu_auth.o \
$(OUT_PATH)/vendor/common/mi_api/certi/gatt_dfu/mible_dfu_flash.o \
$(OUT_PATH)/vendor/common/mi_api/certi/gatt_dfu/mible_dfu_main.o 

################################################################################

OUT_DIR += /vendor/common/mi_api/certi/mesh_auth

OBJS += \
$(OUT_PATH)/vendor/common/mi_api/certi/mesh_auth/mible_mesh_auth.o 

################################################################################

OUT_DIR += /vendor/common/mi_api/certi/mijia_profiles

OBJS += \
$(OUT_PATH)/vendor/common/mi_api/certi//mijia_profiles/mi_service_server.o

################################################################################

OUT_DIR += /vendor/common/mi_api/certi/third_party/mbedtls

OBJS += \
$(OUT_PATH)/vendor/common/mi_api/certi/third_party/mbedtls/asn1parse.o \
$(OUT_PATH)/vendor/common/mi_api/certi/third_party/mbedtls/ccm.o \
$(OUT_PATH)/vendor/common/mi_api/certi/third_party/mbedtls/sha256_hkdf.o 

################################################################################

OUT_DIR += /vendor/common/mi_api/certi/third_party/micro-ecc

OBJS += \
$(OUT_PATH)/vendor/common/mi_api/certi/third_party/micro-ecc/uECC.o 

################################################################################

OUT_DIR += /vendor/common/mi_api/certi/third_party/pt

OBJS += \
$(OUT_PATH)/vendor/common/mi_api/certi/third_party/pt/pt_misc.o 

# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)vendor/common/mi_api/certi/%.o: $(TEL_PATH)/components/vendor/common/mi_api/certi/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"


################################################################################
# Automatically-generated file. Do not edit!
################################################################################
OUT_DIR += /vendor/common/mi_api/mi_vendor
OBJS += $(OUT_PATH)/vendor/common/mi_api/mi_vendor/vendor_model_mi.o

# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)vendor/common/mi_api/mi_vendor/%.o: $(TEL_PATH)/components/vendor/common/mi_api/mi_vendor/%.c
	@echo 'Building file: $<'
	@tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"
