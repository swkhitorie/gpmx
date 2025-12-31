GPM is a microcontroller project template that integrates open-source components

## directory Structure

| Directory | Description                                            |
| --------- | ------------------------------------------------------ |
| boards    | chip library and boards support pack                   |
| kernel    | rtos kernel, drivers interface, and lowlevel component |
| libs      | components souces files                                |
| make      | build makefiles                                        |
| tools     | build scripts                                          |
| px4       | px4 sdk layer, and modules                             |

## component list

- FreeRTOS v10.2.1
- RT-Thread Nano v4.1.1
- FatFS r0.15a
- LwIP v2.1.2
- CherryUSB
- Cmbacktrace

## toolchain

Edit Toolchain path in ./make/toolchain.sh:

```bash
linux:
edit ./tools/toolchain.sh
armgcc_path=
openocd_path=
armcc_path=
armclang_path=

win:
add toolchain to windows system variables
```

compile:

```bash
linux:
./tools/build.sh <app_path> -j2 -r

win:
./tools/build.bat <app_path> -j2 -r
```

## integrate Peripheral Driver Support

STM32 :

|           | F1           | F4 | H7 | WL |
| --------- | ------------ | -- | -- | -- |
| GPIO/EXTI | **√** | √ | √ | √ |
| RTC       | √           | √ | √ | × |
| USART     | √           | √ | √ | √ |
| I2C       | √           | √ | √ | × |
| SPI       | √           | √ | √ | × |
| CAN       | √           | √ | × | × |
| ETH       | √           | √ | × | × |
