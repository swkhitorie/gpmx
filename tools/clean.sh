#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"
compiler_os=$(uname)
makefile_os=${compiler_os}
app_subpath=$1
cd ${script_dir}/../

current_dir=$(pwd)
find_sub_path=$current_dir/apps/$app_subpath
if [ ! -d ${find_sub_path} ];then
    echo "APP directory:"${find_sub_path}"/ is not exist"
    exit 1
fi

shift 1
echo "[make] cleaning..."
make clean distclean APP_SUBPATH=${app_subpath} OS=${makefile_os} "$@"
exit 0
