#!/bin/bash

compiler_os=$(uname)
if [ ${compiler_os} = "Linux" ]
then
    makefile_os = ${compiler_os}
fi

armgcc_path=/opt/gcc-arm-none-eabi-9-2020-q2-update/
openocd_path=/home/xpack-openocd-0.12.0-6/

find_armgcc_path() {
    armgcc_path_location="$(whereis arm-none-eabi-gcc)"
    armgcc_path=$(echo /${armgcc_path_location#*/})
    return ${armgcc_path}   
}

echo "armgcc_path:" ${armgcc_path}
echo "openocd_path:" ${openocd_path}
