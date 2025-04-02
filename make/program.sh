#!/bin/bash


script_dir="$(cd "$(dirname "$0")" && pwd)"

source ${script_dir}/toolchain.sh
board_selection=$1
make_thread=$2
make_rebuild=$3
prog_interface=$4
prog_target=$5
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
        make BOARD_SELECTION=${board_selection} distclean
        make BOARD_SELECTION=${board_selection} clean
    fi
fi

# make/program.sh f407_eval -j2 -r stlink stm32f4x
# make/program.sh f407_eval -j2 -r stlink stm32h7x

make program ${make_thread} \
        OS=${makefile_os} \
        BOARD_SELECTION=${board_selection} \
        PROJ_OPENOCD_DEBUG=${prog_interface} \
        PROJ_OPENOCD_CHIP=${prog_target} \
        PROJ_OPENOCD_LOAD_ADDR=${target_addr}

exit 0
