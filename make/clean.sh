#!/bin/bash


script_dir="$(cd "$(dirname "$0")" && pwd)"

source ${script_dir}/toolchain.sh
app_subpath=$1

cd ${script_dir}/../

# make/clean.sh test/app_bsp_test
make APP_SUBPATH=${app_subpath} clean distclean

exit 0
