#!/bin/bash


script_dir="$(cd "$(dirname "$0")" && pwd)"

source ${script_dir}/toolchain.sh
make_thread=$1
make_rebuild=$2

cd ${script_dir}/../

if [ ${make_rebuild} ]
then
    if [ ${make_rebuild} == "-r" ]
    then
        make distclean
        make clean
    fi
fi

# make program PROJ_OPENOCD_DEBUG=stlink PROJ_OPENOCD_CHIP=stm32h7x -j4

make program ${make_thread} \
        OS=${makefile_os} \
        PROJ_OPENOCD_DEBUG=stlink \
        PROJ_OPENOCD_CHIP=stm32h7x \
        PROJ_OPENOCD_LOAD_ADDR=0x08000000

exit 0
