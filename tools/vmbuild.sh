#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"
source ${script_dir}/toolchain.sh
app_subpath=$1
make_thread=$2
make_rebuild=$3
param_num=$#
isvm=$(systemd-detect-virt)

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
        ${script_dir}/clean.sh $1
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
    TC_PATH_INST_ARMCLANG=${armclang_path}

build_status=$?
if [ $build_status -ne 0 ]; then
    echo "Build failed with exit code: $build_status"
    exit $build_status
fi

proj_name=$(grep '^PROJ_NAME\s*:=' "$find_app_config" | awk -F':=' '{print $2}' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
proj_tc=$(grep '^PROJ_TC\s*:=' "$find_app_config" | awk -F':=' '{print $2}' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
proj_bin=${current_dir}/bin/${proj_name}_${proj_tc}_.bin
proj_hex=${current_dir}/bin/${proj_name}_${proj_tc}_.hex

if [ ! -f ${proj_bin} ] || [ ! -f ${proj_hex} ];then
    echo "proj file not exist"
    exit 1
fi

if [ ${isvm} == "vmware" ]
then
    shared_path=/mnt/hgfs/
elif [ ${isvm} == "virualbox" ]
then
    shared_path=/mnt/media/
else
    echo "unsupported vm platform ${isvm}"
    exit 1
fi

sub_shared_path=("$shared_path"/*/)

# default 
target_shared_path=${sub_shared_path[0]}

if [ ! -d ${shared_path} ] || [ ! -d ${target_shared_path} ];then
    echo "vm shared path not existed"
    exit 1
fi

echo "copy to shared path ${target_shared_path}"
cp ${proj_bin} ${target_shared_path}
cp ${proj_hex} ${target_shared_path}

exit 0
