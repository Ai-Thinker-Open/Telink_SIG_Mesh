# 天猫精灵AT固件

本示例实现了天猫精灵AT传输功能，编译烧录后烧录任意三元组与天猫精灵配对即可实现AT交互。

## 操作说明

本示例基于TB-02_Kit开发板，使用PB1与PB7作为串口引脚，默认波特率为115200。

短按按钮可改变继电器的状态(即开关插座)，长按按键(直到指示灯快速闪烁)，松开后将会恢复出厂设置，同时指示灯慢闪数次。

## AT指令

|   指令   |       功能      |      备注     |
|----------|-----------------|--------------|
|AT        |测试AT
|AT+GMR    |查询版本号
|AT+RST    |复位模块
|AT+RESTORE|恢复出厂设置
|AT+STATE  |查询配网状态
|AT+SLEEP  |深度睡眠         |暂未实现|
|AT+NETNAME|查询NetKey       |
|AT+PASWORD|查询AppKey       |
|AT+ADDR   |查询模块地址
|AT+SEND   |
|+DATA:    |收到数据

## 语音控制指令
与天猫精灵成功连接后可使用语音进行控制，常用指令如下：

    “天猫精灵，打开插座”

    “天猫精灵，关闭插座”

    “天猫精灵，两分钟后关闭插座”

## 三元组

下面是我自己创建的一些产品申请的三元组，可以用来测试，不保证每个都能够着正常使用

|product ID|Product Secret|MAC地址|
|:----------:|:------------:|:-----:|
|空调|
|10586|d82cacf2993cfa46b42f6f25100c7655|f8a76385b9ea
|10586|efbe4f1cba3b01705fc2463f5bc5374d|f8a76385b9eb
|10586|1e7f37d99909b43835bf7b84ff2abfd3|f8a76385b9ec
|10586|af7f8f10f649f3f0b975f031420c583f|f8a76385b9ed
|10586|349fca807e7ba6b2485e03ea39249941|f8a76385b9ee
|风扇|
|11159|b58427faca6c0233be26294c2cdc8ed2|f8a76393a433
|11159|276f5393c517900c15df45fa8591ce11|f8a76393a434
|11159|1c0c2905805ba98e8c7c25a1cae9f092|f8a76393a435
|11159|5ae11e6b81593cf0848bbaffeabbdd91|f8a76393a436
|11159|1c02ec81e70a03928cf77c40df3d7169|f8a76393a437

## 更改记录

本工程是在远程SDK上修改来的，主要修改如下：

在```/app/app.c```中，添加了user_gpio_init函数，修改了process_ui函数，实现按键长短按功能。

在main_loop函数中，注释factory_reset_cnt_check();函数，关闭通过上电次数恢复出厂的功能。

在```/mesh/light.c```文件中，修改set_on_power_up_onoff函数中的light_par_save(0)为light_par_save(1)，立即保存开关状态。

修改 set_on_power_up_last 函数，实现断电记忆功能

修改 light_par_save_proc 函数，实现未配网也要保存开关状态数据

修改 light_dim_set_hw 函数，只保留开关功能

修改  light_g_level_set_idx_with_trans 函数，将渐变时间强制设为20ms

修改 proc_led 函数，改变指示灯GPIO

在```/mesh/mesh_config.h```文件中，将宏定义LIGHT_TYPE_SEL设为LIGHT_TYPE_NONE，灯的类型修改为None

将宏定义ALI_MD_TIME_EN设为1，打开阿里天猫精灵定时功能。

在 ```app/app_config_8258.h```中打开了调试功能，使之GPIO为PB6，波特率500000

在```mesh/mesh_common.c```文件中，修改 tl_log_msg 函数，强制输出调试信息

在```mesh/generic_model```文件中，将G_ONOFF_GET 回调修改为 mesh_cmd_at_data

在```mesh/vendor_model.c```文件中，将一些opcode回调函数更改为 mesh_cmd_at_data
## 用户其他可修改项

用户可根据自己的需求在这个Demo上修改，实现自己想要的功能：

### 修改按键，指示灯及继电器控制引脚

这些引脚可在app/app_config_8258.h文件中通过宏定义指定，客户可自行查看此文件修改之。

默认定义如下：

    #define BTN_GPIO GPIO_PD2 //按键GPIO
    #define FACTORY_RESTORE  3 //长按恢复出厂设置时间
    #define SWITCH_ENABLE_1 0 //按键优先电平

    #define PWM_R       GPIO_PB4		//继电器GPIO

### 修改按键长按时间
修改上述宏定义即可。

### 修改配网成功指示灯闪烁次数
在```mesh/light.h  ```中，修改宏定义 LED_EVENT_PROVISION_SUCCESS 可修改配网成功指示灯灯闪次数及频率

### 修改出厂设置指示灯闪烁次数
在```mesh/light.h  ```中，修改宏定义 LED_EVENT_FACTORY_SUCCESS 可修改恢复出厂设置后指示灯灯闪次数及频率