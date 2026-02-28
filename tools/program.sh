#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"
source ${script_dir}/toolchain.sh

if [ ${makefile_os} != "Linux" ]
then
    armcc_path=$1
    armclang_path=$2
    armgcc_path=$3
    openocd_path=$4
    openocd_ver=$5
    shift 5
fi

OPTS=$(getopt -o j:ra:f:i:t: --long jobs:,rebuild,app_path:,format_uorb:,interface_prog:,target_prog:, -- "$@") || exit 1
eval set -- "$OPTS"

jobs_count=1
rebuild_flag=0
app_subpath=""
format_uorb=""
prog_interface=""
prog_target=""
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
        -i | --interface_prog)
            prog_interface="$2"
            shift 2
            ;;
        -t | --target_prog)
            prog_target="$2"
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
# echo "interface (i): $prog_interface"
# echo "kernel (k): $prog_target"
# echo "other: $@"

if [ "$format_uorb" ]; then
    sh ${script_dir}/uorb_gen/uorb_msg_generate.sh ${script_dir}/../msg/ ${script_dir}/../build $format_uorb
    exit_code=$?
    if [ ${exit_code} != 0 ]
    then
        echo "uorb_msg_generate.sh exec failed"
        exit 1
    fi
fi

if [ ${makefile_os} != "Linux" ]
then
result=$(${script_dir}/openocd.sh ${makefile_os} ${openocd_path} "${openocd_ver}")
else
result=$(${script_dir}/openocd.sh ${makefile_os} ${openocd_path})
fi
exit_code=$?
if [ ${exit_code} != 0 ]
then
    echo "openocd.sh exec failed"
    exit 1
fi
openocd_interface_path=$(echo "$result" | cut -d'#' -f1)
openocd_target_path=$(echo "$result" | cut -d'#' -f2)
# echo ${openocd_interface_path}
# echo ${openocd_target_path}

echo "Path[gcc-arm ]:" ${armgcc_path}
echo "Path[armcc   ]:" ${armcc_path}
echo "Path[armclang]:" ${armclang_path}
echo "Path[openocd ]:" ${openocd_path}

target_addr=0x08000000
target_type1=$(echo stm32 | grep "${prog_target}")

if [ ${target_type1} ]
then
target_addr=0x08000000
fi

cd ${script_dir}/../

current_dir=$(pwd)
find_sub_path=$current_dir/apps/$app_subpath
if [ ! -d ${find_sub_path} ];then
    echo "APP directory:"${find_sub_path}"/ is not exist"
    exit 1
fi

if [ $rebuild_flag -eq 1 ];then
    ${script_dir}/clean.sh $app_subpath "$@"
    echo "Rebuilding..."
else
    echo "Building..."
fi

make all -j${jobs_count} \
    APP_SUBPATH=${app_subpath} \
    OS=${makefile_os} \
    MAKE_TARGET_CLEANS=n \
    TC_PATH_INST_GCC=${armgcc_path} \
    TC_PATH_INST_ARMCC=${armcc_path} \
    TC_PATH_INST_ARMCLANG=${armclang_path} \
    MK_USE_UORB_FORMAT=${format_uorb} \
    "$@"

make program -j${jobs_count} \
    APP_SUBPATH=${app_subpath} \
    OS=${makefile_os} \
    MAKE_TARGET_CLEANS=n \
    TC_PATH_INST_GCC=${armgcc_path} \
    TC_PATH_INST_ARMCC=${armcc_path} \
    TC_PATH_INST_ARMCLANG=${armclang_path} \
    TC_USE_PROGRAM=yes\
    TC_OPENOCD_PATH=${openocd_path} \
    TC_OPENOCD_DEBUG_CFG_PATH=${openocd_interface_path} \
    TC_OPENOCD_CHIP_CFG_PATH=${openocd_target_path} \
    PROJ_OPENOCD_DEBUG=${prog_interface} \
    PROJ_OPENOCD_CHIP=${prog_target} \
    PROJ_OPENOCD_LOAD_ADDR=${target_addr} \
    MK_USE_UORB_FORMAT=${format_uorb} \
    "$@"

# program.sh -j2 -r -a boards_test -i stlink -t stm32f4x 
# program.sh -j2 -r -a boards_test -i stlink -t stm32h7x

exit 0
