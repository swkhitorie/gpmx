GPM is a microcontroller project template that integrates open-source components

| Directory | Description                                                                   |
| --------- | ----------------------------------------------------------------------------- |
| boards    | Hardware Abstract Layer Re-Encapsulation based on the manufacturer's library |
| include   | Header files of components, Kernel                                            |
| libs      | Sources files of components, (fatfs, usb, posix...)                          |
| make      | Project makefiles and compile srcipts makefile                                |
| kernel    | Rtos Kernel                                                                   |
| px4       | Important components ported from PX4, such as uorb, workqueue, hrt.          |

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

support chip driver:

    stm32f1xx: usart, i2c, spi

    stm32f4xx: usart, i2c, spi

    stm32h7xx: usart, i2c, spi

    stm32wlxx: usart
