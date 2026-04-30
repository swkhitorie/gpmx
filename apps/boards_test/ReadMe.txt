
run command example:
    .\tools\build.bat -j8 -r -a boards_test -b pxboard_ebfv2 TEST_ITEM="BOARD_TEST"
    .\tools\build.bat -j8 -r -a boards_test -b pxboard_ebfv2 TEST_ITEM="BOARD_TEST_ITEM=freertos_status_test"
    .\tools\build.bat -j8 -r -a boards_test -b pxboard_ebfv2 TEST_ITEM="BOARD_TEST_ITEM=klibc_pthread_test"
    .\tools\build.bat -j8 -r -a boards_test -b pixhawk_fmuv2 TEST_ITEM="BOARD_TEST_ITEM=freertos_status_test"
    .\tools\program.bat -j8 -r -a boards_test -i stlink -t stm32f4x -b pxboard_ebfv2 TEST_ITEM="BOARD_TEST_ITEM=freertos_status_test"

