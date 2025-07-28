#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"
source ${script_dir}/toolchain.sh
app_subpath=$1

cd ${script_dir}/../

make clean distclean \
    APP_SUBPATH=${app_subpath} \
    OS=${makefile_os} \
    TC_PATH_INST_GCC=${armgcc_path} \
    TC_PATH_INST_ARMCC=${armcc_path} \
    TC_PATH_INST_ARMCLANG=${armclang_path}\
    TC_OPENOCD_PATH=${openocd_path} \
    MAKE_TARGET_CLEANS=y

# make/clean.sh test/app_bsp_test

exit 0
