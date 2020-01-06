# 蓝牙Mesh智能灯

## 可配置项介绍
修改工程目录下的 ```app_config.h```文件，可配置为不同的功能

## PWM引脚
修改宏定义```WM_R```，```PWM_G```，```PWM_B```，```PWM_W```可设置PWM引脚

    #define PWM_R       GPIO_PB4
    #define PWM_G       GPIO_PB5
    #define PWM_B       GPIO_PC2
    #define PWM_W       GPIO_PB7

## 对接天猫精灵
修改宏定义```CONTRON_MODE```可设置是否对接天猫精灵

    /*********************************************
    * 对接音箱模式
    * 可能值：  MESH_NORMAL_MODE		 正常模式
                MESH_SPIRIT_ENABLE		对接天猫精灵
                MESH_MI_ENABLE          对接小爱同学

    * 默认值：MESH_NORMAL_MODE
    * ********************************************/
    #define CONTRON_MODE MESH_SPIRIT_ENABLE


## 打开串口调试

    /********************************************
    * 串口调试功能配置
    * ******************************************/
    #define HCI_LOG_FW_EN   1 //串口打印调试使能
    #if HCI_LOG_FW_EN
        #define PRINT_DEBUG_INFO                    1
        #define DEBUG_INFO_TX_PIN           		GPIO_PB1
        #define PB1_OUTPUT_ENABLE         			1
        #define PB1_DATA_OUT                        1 //must
        #define PULL_WAKEUP_SRC_PB1         		PM_PIN_PULLUP_10K
    #endif