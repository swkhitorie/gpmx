
run command example:
    ./tools/build.sh -j8 -r -a boards_test -b pxboard_ebfv2 -e freertos_status_test -v
    ./tools/build.sh -j8 -r -a boards_test -b pxboard_ebfv2 -e klibc_pthread_test -v
    ./tools/program.sh -j8 -r -a boards_test -i stlink -t stm32f4x -b pxboard_ebfv2 -e freertos_status_test

