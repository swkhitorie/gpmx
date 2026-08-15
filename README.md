| Directory | Description                                     |
| --------- | ----------------------------------------------- |
| boards    | chip library and boards support pack            |
| gpm       | rtos kernel, drivers, lib, and other components |
| tools     | toolchains run script                           |

GPMX is a personal mcu project that integrates open-source components

## directory Structure



## component list

- FreeRTOS v10.2.1
- RT-Thread Nano v4.1.1
- FatFS r0.15a
- LwIP v2.1.2
- CherryUSB
- Cmbacktrace

## toolchain

Edit Toolchain path in tools/toolchain.sh:

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

## integrate Peripheral Driver Support

STM32 :

|           | F1  | F4 | H7 | WL |
| --------- | --- | -- | -- | -- |
| GPIO/EXTI | √  | √ | √ | √ |
| RTC       | √  | √ | √ | √ |
| USART     | √  | √ | √ | √ |
| I2C       | √  | √ | √ | × |
| SPI       | √  | √ | √ | × |
| CAN       | √  | √ | × | × |
| ETH       | √  | √ | √ | × |
