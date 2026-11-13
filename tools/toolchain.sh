#!/bin/bash

compiler_os=$(uname)
makefile_os=${compiler_os}

if [ ${makefile_os} == "Linux" ]; then
    armgcc_path=~/__toolchain/gcc-arm-none-eabi-10.3-2021.10
    openocd_path=~/__toolchain/xpack-openocd-0.12.0-6/
    armcc_path=~/__toolchain/arm_cc_v5_06
    armclang_path=~/__toolchain/arm_clang_6.24/
elif [ ${makefile_os} == "Windows_NT" ]; then
    armcc_path=$(busybox dirname "$(busybox dirname $(busybox which armcc.exe 2>/dev/null | head -n 1))")
    armclang_path=$(busybox dirname "$(busybox dirname $(busybox which armclang.exe 2>/dev/null | head -n 1))")
    armgcc_path=$(busybox dirname "$(busybox dirname $(busybox which arm-none-eabi-gcc.exe 2>/dev/null | head -n 1))")
else
    echo "Unsupport Build Platform ${makefile_os}"
    exit 1
fi

echo "-- $(make --version | head -n 1)"
echo "-- compiler gae:     " ${armgcc_path}
echo "-- compiler armcc:   " ${armcc_path}
echo "-- compiler armclang:" ${armclang_path}
