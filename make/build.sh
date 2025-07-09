#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"
source ${script_dir}/toolchain.sh
app_subpath=$1
make_thread=$2
make_rebuild=$3
param_num=$#

if [ ${param_num} -lt 1 ]; then
    echo "params error, usage: ./make/build.sh <app subpath> <make thread> <-r>"
    exit 1
fi

cd ${script_dir}/../

if [ ${make_rebuild} ]
then
    if [ ${make_rebuild} == "-r" ]
    then
        ./make/clean.sh $1
        echo "rebuilding..."
    else
        echo "building..."
    fi
else
    echo "building..."
fi

if [ ${makefile_os} == "Linux" ]
then
    echo "path[gcc-arm ]:" ${armgcc_path}
    echo "path[openocd ]:" ${openocd_path}
    echo "path[armcc   ]:" ${armcc_path}
    echo "path[armclang]:" ${armclang_path}
    make all ${make_thread} \
        APP_SUBPATH=${app_subpath} \
        OS=${makefile_os} \
        TC_PATH_INST_GCC=${armgcc_path} \
        TC_PATH_INST_ARMCC=${armcc_path} \
        TC_PATH_INST_ARMCLANG=${armclang_path}\
        TC_OPENOCD_PATH=${openocd_path}
else
    make all ${make_thread} \
        APP_SUBPATH=${app_subpath} \
        OS=${makefile_os}
fi

# make/build.sh test/app_bsp_test -j2 -r 

exit 0
