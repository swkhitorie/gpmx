# GPThread

gpthread是一个集成了各种开源组件的嵌入式软件工程

目录摘要:

| 目录名  | 描述                                                         |
| ------- | ------------------------------------------------------------ |
| bsp     | 板级支持包, 并提供基于芯片厂商提供的sdk进行再封装的软件层    |
| include | 大部分组件的头文件目录, 包括设备驱动接口dev, freertos头文件, 以及freertos-posix头文件... |
| libs    | 组件源文件目录, 包括设备驱动接口dev, freertos-posix, uorb, fatfs, px4-hrt... |
| make    | makefile工具文件, 编译脚本, 以及windows下的编译工具          |
| mm      | 内存管理目录, 使用freertos提供的方法                         |
| shed    | freertos内核目录                                             |
| usb     | cherryusb组件, 以及常用的功能封装                            |

bsp支持的芯片列表:

- stm32
  - stm32f103: f1系列的普通外设驱动例程
  - stm32f407, f427: f4系列的普通外设驱动例程
  - stm32h743, h7b0: h7系列的普通外设驱动例程
  - stm32wl55, e5: wl系列的串口单项透传例程, 支持nucleo-wl55jc
  - fmuv2, fmuv6c: 支持v2以及v6c飞控的基础例程

编译:

windows:

```bash
./make/buildtools_win/busybox.exe bash ./make/build.sh fmu_v6c -j2 -r
./make/buildtools_win/busybox.exe bash ./make/program.sh h743_eval -j2 -r stlink stm32h7x
#fmu_v6c: 支持的开发板, 可参考bsp/make.mk
```

linux:

```bash
./make/build.sh fmu_v6c -j2 -r
./make/program.sh h743_eval -j2 -r stlink stm32h7x
```

例程修改:

在apps/app_config.mk里添加修改:

```makefile
PROJ_NAME  :=  test                               #工程名
PROJ_TC    :=  gae                                #选择编译器, 目前只针对gcc进行了编写
# ....
CONFIG_FR_ARCH=${MOD_ARCH}
CONFIG_FR_TOOLCHAIN=gcc
CONFIG_FR_MEM_METHOD=4                            #选择heap_x.c
CONFIG_FR_LIB_CPP=n                               #是否添加cpp new delete到内存管理
CONFIG_FR_LIB_PX4_SUPPORT=n                       
CONFIG_FR_LIB_POSIX=n                             #是否使用freertos-posix
CONFIG_FR_FAT_FATFS=n                             #是否使用fatfs
CONFIG_CRUSB=n                                   
CONFIG_USE_DRV_HRT_INTERNAL=y
CONFIG_FR_LIB_UORB=n
CONFIG_USE_RTKLIB=y

PROJ_CDEFS += CONFIG_BOARD_COM_STDINOUT               #条件编译宏
PROJ_CINCDIRS += apps                                 #头文件目录
CPPSOURCES += apps/app_freertos_test/app_main.cpp     #源文件
```





