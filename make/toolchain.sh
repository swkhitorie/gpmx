#!/bin/bash

compiler_os=$(uname)
makefile_os=${compiler_os}

if [ ${makefile_os} == "Linux" ]
then
    armgcc_path=~/__toolchain/gcc-arm-none-eabi-10.3-2021.10
    openocd_path=~/__toolchain/xpack-openocd-0.12.0-6/
    armcc_path=~/
    armclang_path=~/
else
    armgcc_path=
    openocd_path=
    armcc_path=
    armclang_path=
fi
