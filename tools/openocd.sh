#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"
mk_os=$1
openocd_path=$2

if [ ${mk_os} != "Linux" ]
then
    openocd_ver=$3
    openocd_version=$(echo "${openocd_ver}" | grep -o "Open On-Chip Debugger .*" | awk -F"Debugger " '{print $2}')
else
    openocd_bin=$openocd_path/bin/openocd
    if [ ! -f $openocd_bin ]; then
        echo "$openocd_bin not exist"
        exit 1
    fi
    openocd_version=$($openocd_bin -v 2>&1 | grep -o "Open On-Chip Debugger .*" | awk -F"Debugger " '{print $2}')
fi

version=$(echo "$openocd_version" | sed -n 's/^\([0-9]\+\.[0-9]\+\.[0-9]\+\).*$/\1/p')

if [ -z "$version" ]; then
    echo "Error, cannot parse openocd version"
    exit 1
fi

#echo "Openocd version $version"

if [ $version = "0.11.0" ]; then
    openocd_interface_path=$openocd_path/scripts/interface
    openocd_target_path=$openocd_path/scripts/target
fi

if [ $version = "0.12.0" ]; then
    openocd_interface_path=$openocd_path/openocd/scripts/interface
    openocd_target_path=$openocd_path/openocd/scripts/target
fi

if [ -z "$openocd_interface_path" ] || [ -z "$openocd_target_path" ]; then
    echo "Invalid openocd interface/target path"
    exit 1
fi

echo $openocd_interface_path#$openocd_target_path

exit 0
