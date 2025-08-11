#!/bin/bash

script_dir="$(cd "$(dirname "$0")" && pwd)"
source ${script_dir}/toolchain.sh

openocd_bin=$openocd_path/bin/openocd

if [ ! -f $openocd_bin ]; then
    echo "$openocd_bin not exist"
    exit 1
fi

openocd_version=$($openocd_bin -v 2>&1 | grep -o "Open On-Chip Debugger .*" | awk -F"Debugger " '{print $2}')
openocd_version2=$($openocd_bin -v 2>&1 | grep -oP 'Open On-Chip Debugger \K[0-9]+\.[0-9]+\.[0-9]+')

if [[ $openocd_version =~ ([0-9]+\.[0-9]+\.[0-9]+) ]]; then
    version="${BASH_REMATCH[1]}"
    echo "Openocd version $version"
else
    echo "Error, cannot parse openocd version"
    exit 1
fi

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

exit 0
