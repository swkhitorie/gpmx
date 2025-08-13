GPM is a microcontroller project template that integrates open-source components

## directory Structure

| Directory | Description                                                                   |
| --------- | ----------------------------------------------------------------------------- |
| boards    | Hardware Abstract Layer Re-Encapsulation based on the manufacturer's library |
| include   | Header files of components, Kernel                                            |
| libs      | Sources files of components, (fatfs, usb, posix...)                          |
| make      | Project makefiles and compile srcipts makefile                                |
| kernel    | Rtos Kernel                                                                   |
| px4       | Important components ported from PX4, such as uorb, workqueue, hrt.          |

## toolchain

Edit Toolchain path in ./make/toolchain.sh:

```bash
armgcc_path=
openocd_path=
armcc_path=
armclang_path=
```

compile:

```bash
./make/build.sh app_template -j2 -r
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
