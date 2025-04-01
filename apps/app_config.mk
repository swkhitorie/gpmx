
# application configuration file
PROJ_NAME  :=  test

# Toolchain used in this project
PROJ_TC  :=  gae

TARGET_POSTBUILD := ${TARGET_DEST_FILENAME_BIN}

#####################################
# bsp configuration
#####################################
MOD_ARCH = m4
#include ${SDK_ROOTDIR}/bsp/stm32/stm32h743_eval/make.mk
#include ${SDK_ROOTDIR}/bsp/stm32/stm32h743_fmuv6/make.mk
#include ${SDK_ROOTDIR}/bsp/stm32/stm32f103_eval/make.mk
include ${SDK_ROOTDIR}/bsp/stm32/stm32f407_eval/make.mk
#include ${SDK_ROOTDIR}/bsp/stm32/stm32f427_fmuv2/make.mk
#include ${SDK_ROOTDIR}/bsp/stm32/stm32wl55_eval/make.mk
#include ${SDK_ROOTDIR}/bsp/stm32/stm32wle5_eval/make.mk

PROJ_CDEFS += ${BSP_CDEFS}
CSOURCES += ${BSP_CSRCS}
PROJ_CINCDIRS += ${BSP_CINCDIRS}
ASMSOURCES := ${BSP_ASMSOURCES}
SCF_FILE := ${BSP_LNK_FILE}
PROJ_ENTRY_POINT := ${BSP_BOARD_ENTRY_POINT}

#####################################
# platform os configuration
#####################################
CONFIG_FR_ARCH=${MOD_ARCH}
CONFIG_FR_TOOLCHAIN=gcc
CONFIG_FR_MEM_METHOD=4
CONFIG_FR_LIB_CPP=n
CONFIG_FR_LIB_PX4_SUPPORT=n
CONFIG_FR_LIB_POSIX=n
CONFIG_FR_FAT_FATFS=n
CONFIG_CRUSB=n
CONFIG_USE_DRV_HRT_INTERNAL=n

include ${SDK_ROOTDIR}/sched/make.mk
include ${SDK_ROOTDIR}/mm/make.mk
include ${SDK_ROOTDIR}/libs/make.mk
include ${SDK_ROOTDIR}/include/make.mk
include ${SDK_ROOTDIR}/usb/make.mk

CSOURCES += ${FR_CSOURCES}
CPPSOURCES += ${FR_CPPSOURCES}
PROJ_CINCDIRS += ${FR_CINCDIRS}

#####################################
# app configuration
#####################################
PROJ_CDEFS += CONFIG_BOARD_COM_STDINOUT
PROJ_CDEFS += CONFIG_BOARD_FREERTOS_ENABLE
# PROJ_CDEFS += CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE
# PROJ_CDEFS += CONFIG_BOARD_CRUSB_CDC_ACM_STDINOUT
# PROJ_CDEFS += CONFIG_FR_MALLOC_FAILED_HANDLE
# PROJ_CDEFS += CONFIG_FR_IDLE_TIMER_TASKCREATE_HANDLE
ifeq (${CONFIG_USE_DRV_HRT_INTERNAL},y)
PROJ_CDEFS += CONFIG_USE_DRV_HRT_INTERNAL
endif

# PROJ_CINCDIRS += libs/drivers/imu/icm42688p
# CSOURCES += libs/drivers/imu/icm42688p/icm42688_test.c

# PROJ_CINCDIRS += libs/drivers/imu/l3gd20
# CSOURCES += libs/drivers/imu/l3gd20/l3gd20_test.c

PROJ_CINCDIRS += libs/gcl/drivers/imu/mpu6000
CSOURCES += libs/gcl/drivers/imu/mpu6000/mpu6050_test.c

# PROJ_CINCDIRS += libs/drivers/magnetometers/ist8310
# CSOURCES += libs/drivers/magnetometers/ist8310/ist8310_test.c

PROJ_CINCDIRS += libs/gcl/drivers/imu/mpu6050/
CPPSOURCES += libs/gcl/drivers/imu/mpu6050/mpu6050.cpp

PROJ_CINCDIRS += apps
CPPSOURCES += apps/app_bsp_eval/app_main.cpp
