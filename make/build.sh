#!/bin/bash


script_dir="$(cd "$(dirname "$0")" && pwd)"

source ${script_dir}/toolchain.sh
make_thread=$1
make_rebuild=$2
app_subpath=$3

cd ${script_dir}/../

if [ ${make_rebuild} ]
then
    if [ ${make_rebuild} == "-r" ]
    then
        make APP_SUBPATH=${app_subpath} distclean
        make APP_SUBPATH=${app_subpath} clean
    fi
fi

# make/build.sh -j2 -r test/app_bsp_test

make all ${make_thread} \
        OS=${makefile_os} \
        APP_SUBPATH=${app_subpath} \

exit 0
