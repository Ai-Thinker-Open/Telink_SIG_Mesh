# 天猫精灵插座

本示例实现了天猫精灵插座的功能，编译烧录后烧录插座三元组与天猫精灵配对即可控制。

## 操作说明
本示例使用了三个GPIO，分别用作继电器控制，指示灯，按钮。

短按按钮可改变继电器的状态(即开关插座)，长按按键(直到指示灯快速闪烁)，松开后将会恢复出厂设置，同时指示灯慢闪数次。


## 控制指令
与天猫精灵成功连接后可使用语音进行控制，常用指令如下：

    “天猫精灵，打开插座”

    “天猫精灵，关闭插座”

    “天猫精灵，两分钟后关闭插座”

## 三元组

下面是我自己创建的插座产品申请的三元组，可以用来测试。

|product ID|Product Secret|MAC地址|
|:----------:|:------------:|:-----:|
|10697|c6470deb30e21b447973de37a99d8c36|f8a76387a87d
|10697|11478a63b541e25b6cf75b66a4ed1f90|f8a76387a87e
|10697|9c3990f6670479fe19866f9fa4eecc39|f8a76387a87f
|10697|66c663c23a447d9076f3eba0a5f0b00c|f8a76387a880
|10697|7c4fdfbebc52e85cbd88ca495a18efaa|f8a76387a881
|10697|11ee16e6fcf85ff50b43fb33896b4723|f8a76387a882
|10697|d88894d5377b79419e8cc96be7cdfb33|f8a76387a883
|10697|538c9006567c54ea410791783b5b2ce3|f8a76387a884
|10697|e548f591bed5c57700e3e6118dcceac1|f8a76387a885
|10697|822341ac90909e54e0f50b0b588f3649|f8a76387a886

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