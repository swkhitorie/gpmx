#!/bin/bash


script_dir="$(cd "$(dirname "$0")" && pwd)"

source ${script_dir}/toolchain.sh
make_thread=$1
make_rebuild=$2
prog_interface=$3
prog_target=$4
app_subpath=$5

target_addr=0x08000000
target_type1=$(echo stm32 | grep "${prog_target}")

if [ ${target_type1} ]
then
target_addr=0x08000000
fi

cd ${script_dir}/../

if [ ${make_rebuild} ]
then
    if [ ${make_rebuild} == "-r" ]
    then
        make APP_SUBPATH=${app_subpath} distclean
        make APP_SUBPATH=${app_subpath} clean
    fi
fi

# make/program.sh -j2 -r stlink stm32f4x test/app_bsp_test
# make/program.sh -j2 -r stlink stm32h7x test/app_bsp_test

make program ${make_thread} \
        OS=${makefile_os} \
        APP_SUBPATH=${app_subpath} \
        PROJ_OPENOCD_DEBUG=${prog_interface} \
        PROJ_OPENOCD_CHIP=${prog_target} \
        PROJ_OPENOCD_LOAD_ADDR=${target_addr}

exit 0
