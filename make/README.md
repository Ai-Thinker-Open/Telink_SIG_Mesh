# 编译及烧录脚本
tc32-elf-gcc -ffunction-sections -fdata-sections -I"I:\sig_mesh_sdk\firmware" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\libs" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\mijia_ble_api" -D__PROJECT_MESH__=1 -D__telink__ -DCHIP_TYPE=CHIP_TYPE_8258 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"vendor/common/mesh_common.o" "../vendor/common/mesh_common.c"
Finished building: ../vendor/common/lighting_model_LC.c
Finished building: ../vendor/common/lighting_model_HSL.c
 
Finished building: ../vendor/common/lighting_model_xyl.c
 
 
Building file: ../vendor/common/mesh_fn.c
Building file: ../vendor/common/mesh_lpn.c
Invoking: TC32 Compiler
tc32-elf-gcc -ffunction-sections -fdata-sections -I"I:\sig_mesh_sdk\firmware" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\libs" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\mijia_ble_api" -D__PROJECT_MESH__=1 -D__telink__ -DCHIP_TYPE=CHIP_TYPE_8258 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"vendor/common/mesh_fn.o" "../vendor/common/mesh_fn.c"
Invoking: TC32 Compiler
tc32-elf-gcc -ffunction-sections -fdata-sections -I"I:\sig_mesh_sdk\firmware" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\libs" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\mijia_ble_api" -D__PROJECT_MESH__=1 -D__telink__ -DCHIP_TYPE=CHIP_TYPE_8258 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"vendor/common/mesh_lpn.o" "../vendor/common/mesh_lpn.c"
Building file: ../vendor/common/mesh_node.c
Invoking: TC32 Compiler
tc32-elf-gcc -ffunction-sections -fdata-sections -I"I:\sig_mesh_sdk\firmware" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\libs" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\mijia_ble_api" -D__PROJECT_MESH__=1 -D__telink__ -DCHIP_TYPE=CHIP_TYPE_8258 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"vendor/common/mesh_node.o" "../vendor/common/mesh_node.c"
Finished building: ../vendor/common/mesh_common.c
 
Building file: ../vendor/common/mesh_property.c
Invoking: TC32 Compiler
tc32-elf-gcc -ffunction-sections -fdata-sections -I"I:\sig_mesh_sdk\firmware" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\libs" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\mijia_ble_api" -D__PROJECT_MESH__=1 -D__telink__ -DCHIP_TYPE=CHIP_TYPE_8258 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"vendor/common/mesh_property.o" "../vendor/common/mesh_property.c"
Finished building: ../vendor/common/mesh_lpn.c
 
Finished building: ../vendor/common/mesh_fn.c
Building file: ../vendor/common/mesh_test_cmd.c
 
Invoking: TC32 Compiler
tc32-elf-gcc -ffunction-sections -fdata-sections -I"I:\sig_mesh_sdk\firmware" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\libs" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\mijia_ble_api" -D__PROJECT_MESH__=1 -D__telink__ -DCHIP_TYPE=CHIP_TYPE_8258 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"vendor/common/mesh_test_cmd.o" "../vendor/common/mesh_test_cmd.c"
Building file: ../vendor/common/myprintf.c
Invoking: TC32 Compiler
tc32-elf-gcc -ffunction-sections -fdata-sections -I"I:\sig_mesh_sdk\firmware" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\libs" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\mijia_ble_api" -D__PROJECT_MESH__=1 -D__telink__ -DCHIP_TYPE=CHIP_TYPE_8258 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"vendor/common/myprintf.o" "../vendor/common/myprintf.c"
Finished building: ../vendor/common/mesh_property.c
 
Building file: ../vendor/common/ota_fw_ow.c
Finished building: ../vendor/common/mesh_test_cmd.c
Invoking: TC32 Compiler
tc32-elf-gcc -ffunction-sections -fdata-sections -I"I:\sig_mesh_sdk\firmware" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\libs" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\mijia_ble_api" -D__PROJECT_MESH__=1 -D__telink__ -DCHIP_TYPE=CHIP_TYPE_8258 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"vendor/common/ota_fw_ow.o" "../vendor/common/ota_fw_ow.c"
 
Finished building: ../vendor/common/myprintf.c
Finished building: ../vendor/common/mesh_node.c
 
 
Building file: ../vendor/common/scene.c
Finished building: ../vendor/common/ota_fw_ow.c
Invoking: TC32 Compiler
tc32-elf-gcc -ffunction-sections -fdata-sections -I"I:\sig_mesh_sdk\firmware" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\libs" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\mijia_ble_api" -D__PROJECT_MESH__=1 -D__telink__ -DCHIP_TYPE=CHIP_TYPE_8258 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"vendor/common/scene.o" "../vendor/common/scene.c"
 
Building file: ../vendor/common/scheduler.c
Building file: ../vendor/common/sensors_model.c
Invoking: TC32 Compiler
tc32-elf-gcc -ffunction-sections -fdata-sections -I"I:\sig_mesh_sdk\firmware" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\libs" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\mijia_ble_api" -D__PROJECT_MESH__=1 -D__telink__ -DCHIP_TYPE=CHIP_TYPE_8258 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"vendor/common/scheduler.o" "../vendor/common/scheduler.c"
Invoking: TC32 Compiler
tc32-elf-gcc -ffunction-sections -fdata-sections -I"I:\sig_mesh_sdk\firmware" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\libs" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\mijia_ble_api" -D__PROJECT_MESH__=1 -D__telink__ -DCHIP_TYPE=CHIP_TYPE_8258 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"vendor/common/sensors_model.o" "../vendor/common/sensors_model.c"
Finished building: ../vendor/common/scene.c
Building file: ../vendor/common/system_time.c
 
Invoking: TC32 Compiler
tc32-elf-gcc -ffunction-sections -fdata-sections -I"I:\sig_mesh_sdk\firmware" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\libs" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\mijia_ble_api" -D__PROJECT_MESH__=1 -D__telink__ -DCHIP_TYPE=CHIP_TYPE_8258 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"vendor/common/system_time.o" "../vendor/common/system_time.c"
Building file: ../vendor/common/time_model.c
Finished building: ../vendor/common/scheduler.c
Invoking: TC32 Compiler
Finished building: ../vendor/common/sensors_model.c
tc32-elf-gcc -ffunction-sections -fdata-sections -I"I:\sig_mesh_sdk\firmware" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\libs" -I"I:\sig_mesh_sdk\firmware\vendor\common\mi_api\mijia_ble_api" -D__PROJECT_MESH__=1 -D__telink__ -DCHIP_TYPE=CHIP_TYPE_8258 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"vendor/common/time_model.o" "../vendor/common/time_model.c"
Finished building: ../vendor/common/system_time.c