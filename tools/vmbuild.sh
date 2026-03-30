#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"
source ${script_dir}/toolchain.sh
param_num=$#

if [ ${makefile_os} != "Linux" ]
then
    armcc_path=$1
    armclang_path=$2
    armgcc_path=$3
    shift 3
fi

OPTS=$(getopt -o j:ra:f: --long jobs:,rebuild,app_path:,format_uorb: -- "$@") || exit 1
eval set -- "$OPTS"

jobs_count=1
rebuild_flag=0
app_subpath=""
format_uorb=""
while true; do
    case "$1" in
        -j | --jobs)
            jobs_count="$2"
            shift 2
            ;;
        -r | --rebuild)
            rebuild_flag=1
            shift 1
            ;;
        -a | --app_path)
            app_subpath="$2"
            shift 2
            ;;
        -f | --format_uorb)
            format_uorb="$2"
            shift 2
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "internal error" >&2
            exit 1
            ;;
    esac
done

# echo "job count (j): $jobs_count"
# echo "rebuild_flag (r): $rebuild_flag"
# echo "output_file (u): $app_subpath"
# echo "format_uorb (f): $format_uorb"
# echo "other: $@"

if [ "$format_uorb" ]; then
    bash ${script_dir}/uorb_gen/uorb_msg_generate.sh ${script_dir}/../msg/ ${script_dir}/../build $format_uorb
    exit_code=$?
    if [ ${exit_code} != 0 ]
    then
        echo "uorb_msg_generate.sh exec failed"
        exit 1
    fi
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

if [ $rebuild_flag -eq 1 ];then
    ${script_dir}/clean.sh $app_subpath "$@"
    echo "Rebuilding..."
else
    echo "Building..."
fi

build_time=$(date +"%Y%m%d_%H%M%S")

make all -j${jobs_count} \
    APP_SUBPATH=${app_subpath} \
    OS=${makefile_os} \
    MAKE_TARGET_CLEANS=n \
    TC_PATH_INST_GCC=${armgcc_path} \
    TC_PATH_INST_ARMCC=${armcc_path} \
    TC_PATH_INST_ARMCLANG=${armclang_path} \
    MK_USE_KERNEL_UORB=${format_uorb} \
    "$@"

build_status=$?
if [ $build_status -ne 0 ]; then
    echo "Build failed with exit code: $build_status"
    exit $build_status
fi

proj_name=$(grep '^PROJ_NAME\s*:=' "$find_app_config" | awk -F':=' '{print $2}' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
proj_tc=$(grep '^PROJ_TC\s*:=' "$find_app_config" | awk -F':=' '{print $2}' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
proj_bin=${current_dir}/bin/${proj_name}_${proj_tc}_0.bin
proj_hex=${current_dir}/bin/${proj_name}_${proj_tc}_0.hex
proj_elf=${current_dir}/bin/${proj_name}_${proj_tc}_0.elf

if [ ! -f ${proj_bin} ] || [ ! -f ${proj_hex} ] || [ ! -f ${proj_elf} ];then
    echo "proj file not exist"
    exit 1
fi

isvm=$(systemd-detect-virt)
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
cp ${proj_elf} ${target_shared_path}

exit 0
