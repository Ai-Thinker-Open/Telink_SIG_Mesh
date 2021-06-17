# mijia BLE Low level API
## 主分支
mijia BLE 底层通用 API 定义。mible_api.c 中函数为弱定义实现。
## 芯片厂分支
各芯片厂提供兼容层适配：
保持原有主分支文件不变，增加各自平台上的适配文件，以芯片型号命名 xxxx_api.c，
如 nRF5_api.c 是 Nordic nRF5 平台对 mible_api.c 内函数的实现。
此文件中还可以增加其他文件，用于支持适配。
芯片厂分支内的主分支文件，除 mible_port.h 外不能更改，mible_port.h 用于定义与平台相关的 printf、hexdump、malloc 等。
至少支持 256 bytes HEAP