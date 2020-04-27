# 天猫精灵RGB冷暖灯

本示例实现了天猫精灵红绿蓝 + 冷暖的功能，编译烧录后烧录插座三元组与天猫精灵配对即可控制。

## 操作说明

本示例使用了五路PWM分别控制红绿蓝冷暖灯，上电默认是冷光灯。

快速上电5次，每次上电时间在3S以内，在第六次上电时灯将会恢复出厂设置，恢复出厂设置时灯闪烁数次。

与天猫精灵配对成功后，灯也会闪烁数次。


## 控制指令
与天猫精灵成功连接后可使用语音进行控制，常用指令如下：

    “天猫精灵，开灯”

    “天猫精灵，将灯的亮度调为80”

    “天猫精灵，将灯的色温调为30”

    “天猫精灵，将灯调为红色”

    “天猫精灵，将灯调为紫色”

## 三元组

下面是我自己创建的插座产品申请的三元组，可以用来测试。

|product ID|Product Secret|MAC地址|
|:----------:|:------------:|:-----:|
|10312|29c4ffeab71d7dc826bdaac17dc933b3|f8a7637c9371|
|10312|05cc9fc8fb8b7023775ec83fe068ba51|f8a7637c9372|
|10312|9d5c8db13cacdf46d05719a5a739cf73|f8a7637c9373|
|10312|6075376add65c06a53ea417293b3d16c|f8a7637c9374|
|10312|99eb04fb938b432b5ffd244092f55847|f8a7637c9375|
|10312|b1bdb975e03243bb2cb9d4387e0a16c3|f8a7637c9376|
|10312|2896d81b3efca00c8bd48f418e1d7b17|f8a7637c9377|
|10312|37d01f60204d3ab9a46c6c7543fcf964|f8a7637c9378|
|10312|a05d3e094a98a253611f51c71cb5bdac|f8a7637c9379|
|10312|e3da0f70300e78855e52597a95b40349|f8a7637c937a|


## 更改记录

本工程是在远程SDK上修改来的，主要修改如下：

配置文件中添加了```PWM_C```这个定义，及其延伸定义

 ```(LIGHT_TYPE_SEL == LIGHT_TYPE_CT_HSL)```状态下```light_res_hw```添加```PWM_C```,修改为5路。


在```/mesh/light.c```文件中将

    light_dim_set_hw(idx, 0, get_pwm_cmp(0xff,(100-ct_100)*lum_100/100));
    light_dim_set_hw(idx, 1, get_pwm_cmp(0xff, ct_100*lum_100/100));
修改为：

    light_dim_set_hw(idx, 4, get_pwm_cmp(0xff,(100-ct_100)*lum_100/100));
    light_dim_set_hw(idx, 3, get_pwm_cmp(0xff, ct_100*lum_100/100));


在```/mesh/light.c```文件中，
修改 proc_led 函数，将指示灯闪烁改为呼吸渐变效果
修改 rf_link_light_event_callback 函数，注释掉了LGT_CMD_SET_SUBSCRIPTION 事件回调。

在```/mesh/light.h```文件中，
添加了 LED_EVENT_PROVISION_SUCCESS	LED_EVENT_UNBIND_SUCCESS LED_EVENT_FACTORY_SUCCESS 宏定义

在```/mesh/mesh_config.h```文件中，将宏定义LIGHT_TYPE_SEL设为LIGHT_TYPE_CT，灯的类型修改为冷暖灯。


## 用户其他可修改项

用户可根据自己的需求在这个Demo上修改，实现自己想要的功能：

### 修改PWM引脚

这些引脚可在app/app_config_8258.h文件中通过宏定义指定，客户可自行查看此文件修改之。

默认定义如下：

    //---------------  LED / PWM
    #define PWM_R       GPIO_PB4		//冷光
    #define PWM_G       GPIO_PB5		//暖光
    #define PWM_B       GPIO_PWM3B0		//blue
    #define PWM_W       GPIO_PWM4B1		//white

### 修改恢复出厂设置的上电次数及上电时长
通过修改```mesh/factory_reset.c```文件中的factory_reset_serials变量可修改上述参数。

默认数据如下，表示上电五次恢复出厂设置，每次上电时间在0-3S
const u8 factory_reset_serials[] = { 0, 3,
                                     0, 3,
                                     0, 3,
                                     0, 3,
                                     0, 3,};


若将数据改为如下，则表示上电六次恢复出厂设置，前四次上电时间在0-3S，后两次上电时间在5-20秒
const u8 factory_reset_serials[] = { 0, 3,
                                     0, 3,
                                     0, 3,
                                     5, 20,
                                     5, 20,};

用户可根据自己的爱好自行修改。

### 修改配网成功指示灯闪烁次数
在```mesh/light.h  ```中，修改宏定义 LED_EVENT_PROVISION_SUCCESS 可修改配网成功指示灯灯闪次数及频率

### 修改出厂设置指示灯闪烁次数
在```mesh/light.h  ```中，修改宏定义 LED_EVENT_FACTORY_SUCCESS 可修改恢复出厂设置后指示灯灯闪次数及频率