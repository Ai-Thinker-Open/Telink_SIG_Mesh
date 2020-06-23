# 示例程序

## AT_Ali_Mesh
可通过蓝牙Mesh连接天猫精灵的AT固件，可对接除灯和插座开关外的任何产品，需外接MCU

## CW_Ali_Mesh
可通过蓝牙Mesh连接天猫精灵的冷暖灯固件,已配置好，直接编译烧录即可

## Plug_Ali_Mesh
可通过蓝牙Mesh连接天猫精灵的插座固件,已配置好，直接编译烧录即可

## RGBCW_Ali_Mesh
可通过蓝牙Mesh连接天猫精灵的RGB冷暖,已配置好，直接编译烧录即可

## SFS_Ali_Mesh
可通过蓝牙Mesh连接天猫精灵的单火线开关,已配置好，直接编译烧录即可

## 打开与关闭调试信息打印

#### 日志开关及串口引脚

在工程目录 ```app/app_config_8258.h```文件中，有如下与日志打印相关的宏定义：

    #define HCI_LOG_FW_EN   1
    #if HCI_LOG_FW_EN
    #define DEBUG_INFO_TX_PIN           		GPIO_PB1
    #define PRINT_DEBUG_INFO                    1
    #endif

其中```HCI_LOG_FW_EN```控住日志开关，```DEBUG_INFO_TX_PIN```设置日志串口引脚，```PRINT_DEBUG_INFO```

#### 串口波特率

在工程目录下```mesh/myprintf.h```文件中，宏定义```BAUD_USE```决定了波特率，用户可将其改为其他值。

#### 日志级别

在SDK目录下```proj_lib/sig_mesh/app_mesh.h```中，宏定义```TL_LOG_LEVEL```决定了日志级别，用户可修改这个宏定义过滤一些日志。
