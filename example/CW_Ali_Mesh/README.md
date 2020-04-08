# 天猫精灵冷暖灯

本示例实现了天猫精灵冷暖的功能，编译烧录后烧录插座三元组与天猫精灵配对即可控制。

## 操作说明

本示例使用了两路PWM分别控制冷光灯和暖光灯，上电默认是冷光灯。

快速上电5次，每次上电时间在3S以内，在第六次上电时灯将会恢复出厂设置，恢复出厂设置时灯闪烁数次。

与天猫精灵配对成功后，灯也会闪烁数次。


## 控制指令
与天猫精灵成功连接后可使用语音进行控制，常用指令如下：

    “天猫精灵，开灯”

    “天猫精灵，将灯的亮度调为80”

    “天猫精灵，将灯的色温调为30”

## 三元组

下面是我自己创建的插座产品申请的三元组，可以用来测试。

|product ID|Product Secret|MAC地址|
|:----------:|:------------:|:-----:|
|11083|62aac4f6cf738007f4b4a3b88b739c92|f8a763929176
|11083|2cfed0966cafd412e4c93fee8650836d|f8a763929177
|11083|2e4481ef43fcc2d715b88090d81cfc3b|f8a763929178
|11083|f8a27997161169f7de247dc7541e41b9|f8a763929179
|11083|65a2f7dc96ce075c18c8b5a49f4ea822|f8a76392917a
|11083|cf51b09c66106706cb1b861a86fdcfe3|f8a76392917b
|11083|4b12d213d665bed3c451713ef7d9da22|f8a76392917c
|11083|1c77a3b3eaf7d872c39c1054447aedea|f8a76392917d
|11083|0d7080190ee8d9c15b38e9cc64b43c2f|f8a76392917e
|11083|0cfcf3b376d1ec3643e73562d26aa3df|f8a76392917f


## 更改记录

本工程是在远程SDK上修改来的，主要修改如下：

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