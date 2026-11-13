#!/bin/bash
set -e

script_dir="$(cd "$(dirname "$0")" && pwd)"
source ${script_dir}/toolchain.sh
param_num=$#
if [ ${param_num} -lt 1 ]; then
    echo "Params error, usage: build.sh <app subpath> <make thread> <-r>"
    exit 1
fi

jobs_count=1
rebuild_flag=0
app_subpath=""
uorb_generate=0
uorb_including=""
mavlink_generate=0
mavlink_including=""
build_board=""
test_entry="null_test_item"
virtual_environment=0
while getopts "a:j:rumb:e:v" opt; do
    case $opt in
        a) app_subpath="$OPTARG" ;;
        j) jobs_count="$OPTARG" ;;
        r) rebuild_flag=1 ;;
        u) uorb_generate=1 ;;
        m) mavlink_generate=1 ;;
        b) build_board="$OPTARG" ;;
        e) test_entry="$OPTARG" ;;
        v) virtual_environment=1 ;;
        \?) echo "Usage: $0 -a <app_subpath> [-j jobs] [-r]" >&2; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

if [ ${jobs_count} -eq 0 ];then
    jobs_count=""
fi

if [ $rebuild_flag -eq 1 ];then
    bash ${script_dir}/clean.sh $app_subpath APP_BUILT_BOARD=${build_board} "$@"
fi

if [ $uorb_generate -eq 1 ]; then
    uorb_generate_path=${script_dir}/../build/${app_subpath}
    uorb_msg_path=${script_dir}/../apps/${app_subpath}/msg/
    bash ${script_dir}/../gpm/tools/msg/uorb_msg_generate.sh ${uorb_msg_path} ${uorb_generate_path} ${uorb_generate}
    exit_code=$?
    if [ ${exit_code} == 0 ]; then
        uorb_including="${uorb_generate_path}/;"
    else
        echo "[uorb] exec failed ${uorb_generate}"
        uorb_including=
    fi
fi

if [ $mavlink_generate -eq 1 ]; then
    mavlink_generate_path=${script_dir}/../build/${app_subpath}/mavlink
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

BUILD_TIME=$(date +"%Y%m%d_%H%M%S")

BEAR_COMMAND=bear
if command -v bear >/dev/null 2>&1; then
    :
else
    echo "[bear] not found"
    BEAR_COMMAND=""
fi

echo "[make] building..."

${BEAR_COMMAND} make prebuild build postbuild -j${jobs_count} \
    APP_SUBPATH=${app_subpath} \
    OS=${makefile_os} \
    TC_PATH_INST_GCC=${armgcc_path} \
    TC_PATH_INST_ARMCC=${armcc_path} \
    TC_PATH_INST_ARMCLANG=${armclang_path} \
    APP_BUILT_BOARD=${build_board} \
    MAVLINK_INCLUDING=${mavlink_including} \
    UORB_INCLUDING=${uorb_including} \
    TEST_ENTRY=${test_entry} \
    "$@"

build_status=$?
if [ $build_status -eq 0 ] && [ $virtual_environment -eq 1 ]; then
    bash ${script_dir}/vmhelptool.sh $find_app_config
fi
