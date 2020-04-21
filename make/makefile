################################################################################
# Automatically-generated file. Do not edit!
################################################################################

OBJ_SRCS := 
S_SRCS := 
ASM_SRCS := 
C_SRCS := 
S_UPPER_SRCS := 
O_SRCS := 
FLASH_IMAGE := 
ELFS := 
OBJS := 
LST := 
SIZEDUMMY := 
OUT_DIR :=

# ################################################################################
# # Automatically-generated file. Do not edit! subdir.mk
# ################################################################################

# # Add inputs and outputs from these tool invocations to the build variables 
# S_UPPER_SRCS += \
# ../div_mod.S 

# OBJS += $(OUT_PATH)/div_mod.o 

# # Each subdirectory must supply rules for building sources it contributes
# $(OUT_PATH)/%.o: ../../%.S
# 	@echo 'Building file: $<'
# 	@echo 'Invoking: TC32 CC/Assembler'
# 	tc32-elf-gcc -DMCU_CORE_8269 -c -o"$@" "$<"
# 	@echo 'Finished building: $<'
# 	@echo ' '

# ################################################################################
# # Automatically-generated file. Do not edit! object.mk
# ################################################################################

GCC_FLAGS := \
-ffunction-sections \
-fdata-sections \
-Wall \
-O2 \
-fpack-struct \
-fshort-enums \
-finline-small-functions \
-std=gnu99 \
-fshort-wchar \
-fms-extensions 

INCLUDE_PATHS := -I$(TEL_PATH)/SDK_3.1.5 -I$(PROJECT_PATH)

GCC_FLAGS += $(TEL_CHIP)

LS_FLAGS := $(TEL_PATH)/SDK_3.1.5/boot.link

#include SDK makefile
-include $(TEL_PATH)/make/boot.mk
-include $(TEL_PATH)/make/driver.mk
-include $(TEL_PATH)/make/proj.mk

-include $(PROJECT_PATH)/mesh/mesh.mk
-include $(PROJECT_PATH)/app.mk

# Add inputs and outputs from these tool invocations to the build variables 
LST_FILE := $(OUT_PATH)/$(PROJECT_NAME).lst
BIN_FILE := $(OUT_PATH)/$(PROJECT_NAME).bin
ELF_FILE := $(OUT_PATH)/$(PROJECT_NAME).elf

SIZEDUMMY := sizedummy

# All Target
all: pre-build main-build

# Main-build Target
main-build: $(ELF_FILE) secondary-outputs

# Tool invocations
$(ELF_FILE): $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@tc32-elf-ld --gc-sections -L $(TEL_PATH)/SDK_3.1.5/proj_lib -T $(LS_FLAGS) -o $(ELF_FILE) $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

$(LST_FILE): $(ELF_FILE)
	@echo 'Invoking: TC32 Create Extended Listing'
	tc32-elf-objdump -x -D -l -S  $(ELF_FILE)  > $(LST_FILE)
	@echo 'Finished building: $@'
	@echo ' '

$(BIN_FILE): $(ELF_FILE)
	@echo 'Create Flash image (binary format)'
	tc32-elf-objcopy -v -O binary $(ELF_FILE)  $(BIN_FILE)
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: $(ELF_FILE)
	@echo 'Invoking: Print Size'
	tc32-elf-size -t $(ELF_FILE)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(LST)$(SIZEDUMMY) $(ELF_FILE) $(BIN_FILE) $(LST_FILE)
	-@echo ' '

pre-build:
	@mkdir -p $(foreach s,$(OUT_DIR),$(OUT_PATH)$(s))
#-"D:\tel\ble_sdk_release/getver.sh"
	-@echo ' '

flash:
	python $(TEL_PATH)/make/Telink_Tools.py -p $(DOWNLOAD_PORT) burn $(BIN_FILE) 

erase_fw:
	python $(TEL_PATH)/make/Telink_Tools.py -p $(DOWNLOAD_PORT) erase_flash 0x4000 44

erase_all:
	python $(TEL_PATH)/make/Telink_Tools.py -p $(DOWNLOAD_PORT) erase_flash 0x4000 124

erase_key:
	python $(TEL_PATH)/make/Telink_Tools.py -p $(DOWNLOAD_PORT) erase_flash 0x30000 16

monitor:
	python $(TEL_PATH)/make/monitor.py --port $(DOWNLOAD_PORT) --baud 115200 $(ELF_FILE)

secondary-outputs: $(BIN_FILE) $(LST_FILE) $(SIZEDUMMY)

.PHONY: all clean dependents flash erase_fw erase_key monitor pre-build erase_all 
.SECONDARY: main-build 