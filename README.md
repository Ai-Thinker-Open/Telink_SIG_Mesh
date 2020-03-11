# Telink SIG Mesh SDK

Telink 泰凌微 蓝牙Mesh芯片软件开发套件

# 使用方法
---------------------------------------------------

### 获取TC32编译工具链
>目前仅测试了Mac OS和Linux和操作系统

>Mac OS版交叉编译工具下载地址 ```https://sinoprobe.dscloud.me:8443/telink/tc32_for_macOS.zip```

>Linux版交叉编译工具下载地址 ```https://sinoprobe.dscloud.me:8443/telink/tc32_gcc_v2.0.tar.bz2```


###MacOSX编译环境设置
或直接shell console输入：

wget https://sinoprobe.dscloud.me:8443/telink/tc32_for_macOS.zip

下载完成后，解压到 tc32/ 目录，将此目录拷贝到合适的目录<your_path>，并在~/.bash_profile中加入:

export PATH=$PATH:<your_path>/bin

console 输入: gcc -v 查看本机是否安装gcc编译器，xcode自带，输出如下:

Configured with: --prefix=/Applications/Xcode.app/Contents/Developer/usr --with-gxx-include-dir=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/c++/4.2.1
Apple clang version 11.0.0 (clang-1100.0.33.16)
Target: x86_64-apple-darwin19.3.0
Thread model: posix
InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin

console 输入: tc32-elf-gcc -v 输出同linux系统，验证交叉编译器工作正常。

编译环境python3必不可少！




###linux版本获取编译工具链

    wget https://sinoprobe.dscloud.me:8443/telink/tc32_gcc_v2.0.tar.bz2

解压到opt文件夹 *(也可解压到其他文件夹)*

    sudo tar -xvjf　tc32_gcc_v2.0.tar.bz2　-C /opt/

添加工具链到环境变量(以解压到/opt为例)

    export PATH=$PATH:/opt/tc32/bin

测试是否搭建成功

    tc32-elf-gcc -v

如果搭建成功将打印如下信息:

    Using built-in specs.
    COLLECT_GCC=tc32-elf-gcc
    COLLECT_LTO_WRAPPER=/opt/tc32/lib/gcc/tc32-elf/4.5.1.tc32-elf-1.5/lto-wrapper
    Target: tc32-elf
    Configured with: ../../gcc-4.5.1/configure --program-prefix=tc32-elf- --target=tc32-elf --prefix=/opt/tc32 --enable-languages=c --libexecdir=/opt/tc32/lib --with-gnu-as --with-gnu-ld --without-headers --disable-decimal-float --disable-nls --disable-mathvec --with-pkgversion='Telink TC32 version 2.0 build' --without-docdir --without-fp --without-tls --disable-shared --disable-threads --disable-libffi --disable-libquadmath --disable-libstdcxx-pch --disable-libmudflap --disable-libgomp --disable-libssp -v --without-docdir --enable-soft-float --with-newlib --with-gcc --with-gnu- --with-gmp=/opt/tc32/addontools --with-mpc=/opt/tc32/addontools --with-mpfr=/opt/tc32/addontools
    Thread model: single
    gcc version 4.5.1.tc32-elf-1.5 (Telink TC32 version 2.0 build)

### 获取SDK

    git clone https://github.com/sinoprobe/Telink_SIG_Mesh.git


### 编译
进入examples/8258_mesh工程目录

    cd Telink_SIG_Mesh/examples/8258_mesh

执行下列编译指令：

    make all

输出类似如下信息说明编译成功：

    Invoking: Print Size
    tc32-elf-size -t / ... /Telink_SIG_Mesh/examples/8258_mesh/out/8258_mesh.elf
    text    data     bss     dec     hex filename
    134720    2724   14160  151604   25034 / ... /Telink_SIG_Mesh/examples/8258_mesh/out/8258_mesh.elf
    134720    2724   14160  151604   25034 (TOTALS)
    Finished building: sizedummy

### 烧录程序到芯片

串口烧录接线方式如下：

|串口|模块|
|----|---|
|VCC|3V3|
|GND|GND|
|TX|RX|
RX|TX|
|RTS|RST|
|DTR|PA1|

注意：PA1为boot选择引脚，为低电平进入下载模式，为高电平进入运行模式

烧录指令：

    make flash
其他指令：

    make erase_fw //擦除固件
    make erase_key //擦除Mesh相关数据
    make monitor //打开串口监控
    python3 ../../make/Telink_Tools.py -p /dev/ttyUSB0 burn_triad 1345 78da07fa44a7 221746e805ac0e6269bd4d3e55f1145c //烧录三元组
