
PROJ_NAME  :=  test

PROJ_TC    :=  gae

TARGET_POSTBUILD := ${TARGET_DEST_FILENAME_BIN}

# bsp configuration
include ${SDK_ROOTDIR}/bsp/make.mk

# os and library configuration
CONFIG_FR_ARCH=${MOD_ARCH}
CONFIG_FR_TOOLCHAIN=gcc
CONFIG_FR_MEM_METHOD=4
CONFIG_FR_LIB_CPP=y
CONFIG_FR_LIB_PX4_SUPPORT=n
CONFIG_FR_LIB_POSIX=y
CONFIG_FR_FAT_FATFS=n
CONFIG_CRUSB=n
CONFIG_USE_DRV_HRT_INTERNAL=y
CONFIG_FR_LIB_UORB=y

include ${SDK_ROOTDIR}/sched/make.mk
include ${SDK_ROOTDIR}/mm/make.mk
include ${SDK_ROOTDIR}/libs/make.mk
include ${SDK_ROOTDIR}/include/make.mk
include ${SDK_ROOTDIR}/usb/make.mk
CSOURCES      += ${FR_CSOURCES}
CPPSOURCES    += ${FR_CPPSOURCES}
PROJ_CINCDIRS += ${FR_CINCDIRS}

#####################################
# app configuration
#####################################
PROJ_CDEFS += CONFIG_BOARD_COM_STDINOUT
PROJ_CDEFS += CONFIG_BOARD_FREERTOS_ENABLE
# PROJ_CDEFS += CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE
# PROJ_CDEFS += CONFIG_BOARD_CRUSB_CDC_ACM_STDINOUT
PROJ_CDEFS += CONFIG_FR_MALLOC_FAILED_HANDLE
PROJ_CDEFS += CONFIG_FR_IDLE_TIMER_TASKCREATE_HANDLE

PROJ_CINCDIRS += apps
#CPPSOURCES += apps/app_debug/app_main.cpp
#CPPSOURCES += apps/app_debug/app_posix_debug.cpp
#CPPSOURCES += apps/app_bsp_eval/app_main.cpp





# CPPSOURCES += apps/app_fmuv2_test/app_main.cpp
# CSOURCES += libs/gcl/drivers/imu/l3gd20/l3gd20_test.c

# CPPSOURCES += apps/app_fmuv6c_test/app_main.cpp
# CPPSOURCES += libs/gcl/drivers/magnetometers/ist8310/IST8310.cpp
# CSOURCES += libs/gcl/drivers/imu/icm42688p/icm42688_test.c

CONFIG_LINK_PRINTF_FLOAT:=y
CONFIG_LINK_SCANF_FLOAT:=n
CONFIG_COMPILE_OPTIMIZE:=O1
