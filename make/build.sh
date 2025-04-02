#!/bin/bash


script_dir="$(cd "$(dirname "$0")" && pwd)"

source ${script_dir}/toolchain.sh
board_selection=$1
make_thread=$2
make_rebuild=$3

cd ${script_dir}/../

if [ ${make_rebuild} ]
then
    if [ ${make_rebuild} == "-r" ]
    then
        make BOARD_SELECTION=${board_selection} distclean
        make BOARD_SELECTION=${board_selection} clean
    fi
fi

# make/build.sh f407_eval -j2 -r

make all ${make_thread} \
        OS=${makefile_os} \
        BOARD_SELECTION=${board_selection} \

exit 0
