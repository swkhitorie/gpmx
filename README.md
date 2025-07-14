Gprtos is a microcontroller project template that integrates open-source components

| Directory | Description                                                                   |
| --------- | ----------------------------------------------------------------------------- |
| boards    | Hardware Abstract Layer Re-Encapsulation based on the manufacturer's library |
| include   | Header files of components                                                    |
| libs      | Sources files of components, (fatfs, freertos-posix...)                       |
| make      | Project makefiles and compile srcipts makefile                                |
| mm        | Memory manage of freertos kernel                                              |
| shed      | Freertos kernel                                                               |
| usb       | Cherryusb component, and common USB component                                |
| px4       | Important components ported from PX4, such as uorb, workqueue, hrt.          |

Edit Toolchain path in ./make/toolchain.sh:

```bash
if [ ${makefile_os} == "Linux" ]
then
    armgcc_path=~/__toolchain/gcc-arm-none-eabi-10.3-2021.10
    openocd_path=~/__toolchain/xpack-openocd-0.12.0-6/
    armcc_path=~/
    armclang_path=~/
fi
```

compile:

```bash
./make/build.sh bsp_test/fmuv2_test -j2 -r
```
