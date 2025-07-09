#!/bin/bash

compiler_os=$(uname)
makefile_os=${compiler_os}

# find_armgcc_path() {
#     armgcc_path_location="$(whereis arm-none-eabi-gcc)"
#     armgcc_path=$(echo /${armgcc_path_location#*/})
#     return ${armgcc_path}   
# }

if [ ${makefile_os} == "Linux" ]
then
    armgcc_path=~/__toolchain/gcc-arm-none-eabi-10.3-2021.10
    openocd_path=~/__toolchain/xpack-openocd-0.12.0-6/
    armcc_path=~/
    armclang_path=~/
fi
