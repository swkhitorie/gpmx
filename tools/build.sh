#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"
source ${script_dir}/toolchain.sh
param_num=$#

if [ ${makefile_os} != "Linux" ]
then
    armcc_path=$1
    armclang_path=$2
    armgcc_path=$3
    app_subpath=$4
    make_thread=$5
    make_rebuild=$6
    shift 6
else
    app_subpath=$1
    make_thread=$2
    make_rebuild=$3
    shift 3
fi

echo "Path[gcc-arm ]:" ${armgcc_path}
echo "Path[armcc   ]:" ${armcc_path}
echo "Path[armclang]:" ${armclang_path}

if [ ${param_num} -lt 1 ]; then
    echo "Params error, usage: build.sh <app subpath> <make thread> <-r>"
    exit 1
fi

cd ${script_dir}/../

current_dir=$(pwd)
find_sub_path=$current_dir/apps/$app_subpath
find_app_config=$current_dir/apps/$app_subpath/config.mk
if [ ! -d ${find_sub_path} ];then
    echo "APP directory:"${find_sub_path}"/ is not exist"
    exit 1
fi

if [ ! -f ${find_app_config} ];then
    echo "APP config file:"${find_app_config}"/ is not exist"
    exit 1
fi

if [ ${make_rebuild} ]
then
    if [ ${make_rebuild} == "-r" ]
    then
        ${script_dir}/clean.sh $app_subpath "$@"
        echo "Rebuilding..."
    else
        echo "Building..."
    fi
else
    echo "Building..."
fi

make all ${make_thread} \
    APP_SUBPATH=${app_subpath} \
    OS=${makefile_os} \
    MAKE_TARGET_CLEANS=n \
    TC_PATH_INST_GCC=${armgcc_path} \
    TC_PATH_INST_ARMCC=${armcc_path} \
    TC_PATH_INST_ARMCLANG=${armclang_path} \
    "$@"

# build.sh test/app_bsp_test -j2 -r 

exit 0
