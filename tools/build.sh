#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"
source ${script_dir}/toolchain.sh
param_num=$#

if [ ${makefile_os} != "Linux" ]
then
    armcc_path=$(busybox dirname "$(busybox dirname $(busybox which armcc.exe 2>/dev/null | head -n 1))")
    armclang_path=$(busybox dirname "$(busybox dirname $(busybox which armclang.exe 2>/dev/null | head -n 1))")
    armgcc_path=$(busybox dirname "$(busybox dirname $(busybox which arm-none-eabi-gcc.exe 2>/dev/null | head -n 1))")
fi

echo "-- $(make --version | head -n 1)"
echo "-- compiler gae:     " ${armgcc_path}
echo "-- compiler armcc:   " ${armcc_path}
echo "-- compiler armclang:" ${armclang_path}

OPTS=$(getopt -o j:ra:umb:ve: --long jobs:,rebuild,app_path:,uorb,mavlink,board:,virtualmachine,entry: -- "$@") || exit 1
eval set -- "$OPTS"

jobs_count=1
rebuild_flag=0
app_subpath=""
uorb_generate=0
uorb_enable=n
mavlink_generate=0
mavlink_including=""
build_board=""
virtual_environment=0
test_entry="null_test_item"
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
        -u | --uorb)
            uorb_generate=1
            shift 1
            ;;
        -m | --mavlink)
            mavlink_generate=1
            shift 1
            ;;
        -b | --board)
            build_board="$2"
            shift 2
            ;;
        -v | --virtualmachine)
            virtual_environment=1
            shift 1
            ;;
        -e | --entry)
            test_entry="$2"
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

if [ ${jobs_count} -eq 0 ];then
    jobs_count=""
fi

if [ $rebuild_flag -eq 1 ];then
    bash ${script_dir}/clean.sh $app_subpath $build_board "$@"
fi

uorb_generate_path=${script_dir}/../build/${app_subpath}
uorb_msg_path=${script_dir}/../apps/${app_subpath}/msg/
bash ${script_dir}/../gpm/tools/msg/uorb_msg_generate.sh ${uorb_msg_path} ${uorb_generate_path} ${uorb_generate}
exit_code=$?
if [ ${exit_code} == 0 ]; then
    uorb_including="${uorb_generate_path}/;"
    uorb_enable=y
else
    echo "[uorb] exec failed ${uorb_generate}"
    uorb_including=
    uorb_enable=n
fi

mavlink_generate_path=${script_dir}/../build/${app_subpath}/mavlink
if [ $mavlink_generate -eq 1 ]; then
    bash ${script_dir}/genmavlink.sh ${mavlink_generate_path} common
    bash ${script_dir}/genmavlink.sh ${mavlink_generate_path} development
    mavlink_including+="${mavlink_generate_path};"
    mavlink_including+="${mavlink_generate_path}/common;"
    mavlink_including+="${mavlink_generate_path}/development;"
    exit_code=$?
    if [ ${exit_code} != 0 ]; then
        echo "mavlink/generate.sh exec failed"
        exit 1
    fi
fi

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

build_time=$(date +"%Y%m%d_%H%M%S")

BEAR_COMMAND=bear
if command -v bear >/dev/null 2>&1; then
    :
else
    echo "[bear] not found"
    BEAR_COMMAND=""
fi

echo "[make] building..."

${BEAR_COMMAND} make all -j${jobs_count} \
    APP_SUBPATH=${app_subpath} \
    OS=${makefile_os} \
    MAKE_TARGET_CLEANS=n \
    TC_PATH_INST_GCC=${armgcc_path} \
    TC_PATH_INST_ARMCC=${armcc_path} \
    TC_PATH_INST_ARMCLANG=${armclang_path} \
    BUILD_USR_BOARD=${build_board} \
    CONFIG_MODULE_UORB=${uorb_enable} \
    MAVLINK_INCLUDING=${mavlink_including} \
    UORB_INCLUDING=${uorb_including} \
    TEST_ENTRY=${test_entry} \
    "$@"

build_status=$?
if [ $build_status -ne 0 ]; then
    echo "Build failed with exit code: $build_status"
    exit $build_status
fi

if [ $virtual_environment -ne 1 ]; then
    exit 0
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

echo "[bin] copy to shared path ${target_shared_path}"
cp ${proj_bin} ${target_shared_path}
cp ${proj_hex} ${target_shared_path}
cp ${proj_elf} ${target_shared_path}

exit 0
