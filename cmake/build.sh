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

make all ${make_thread} \
        OS=${makefile_os}

exit 0


# program example:
# make program PROJ_OPENOCD_DEBUG=stlink PROJ_OPENOCD_CHIP=stm32h7x -j4