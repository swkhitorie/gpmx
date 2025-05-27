# GPThread

gpthread是一个集成了各种开源组件的嵌入式软件工程模板

| 目录名  | 描述                                                                                     |
| ------- | ---------------------------------------------------------------------------------------- |
| boards  | 板级支持包, 并提供基于芯片厂商提供的sdk进行再封装的软件层                                |
| include | 大部分组件的头文件目录, 包括设备驱动接口dev, freertos头文件, 以及freertos-posix头文件... |
| libs    | 组件源文件目录, 包括设备驱动接口dev, freertos-posix, uorb, fatfs, px4-hrt...             |
| make    | makefile工具文件, 编译脚本, 以及windows下的编译工具                                      |
| mm      | 内存管理目录, 使用freertos提供的方法                                                     |
| shed    | freertos内核目录                                                                         |
| usb     | cherryusb组件, 以及常用的功能封装                                                        |
| tools   | 脚本工具,包括uorb生成脚本, px4-fmu固件烧录脚本                                           |

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
busybox.exe bash .\make\build.sh -j2 -r test/bsp_test
busybox.exe bash .\make\program.sh -j2 -r stlink stm32h7x test/bsp_test
#test/bsp_test: app代码子目录
```

linux:

```bash
./make/build.sh -j2 -r test/bsp_test
./make/program.sh -j2 -r stlink stm32h7x test/bsp_test
```
