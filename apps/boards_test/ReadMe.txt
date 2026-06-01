
run command example:
    .\tools\build.bat -j8 -r -a boards_test -b pxboard_ebfv2 -e freertos_status_test
    .\tools\build.bat -j8 -r -a boards_test -b pxboard_ebfv2 -e klibc_pthread_test
    .\tools\program.bat -j8 -r -a boards_test -i stlink -t stm32f4x -b pxboard_ebfv2 -e freertos_status_test

