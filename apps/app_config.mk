
PROJ_NAME  :=  test

PROJ_TC    :=  gae

TARGET_POSTBUILD := ${TARGET_DEST_FILENAME_BIN}

# bsp configuration
include ${SDK_ROOTDIR}/bsp/make.mk

# os and library configuration
CONFIG_FR_ARCH=${MOD_ARCH}
CONFIG_FR_TOOLCHAIN=gcc
CONFIG_FR_MEM_METHOD=4
CONFIG_FR_LIB_CPP=n
CONFIG_FR_LIB_PX4_SUPPORT=n
CONFIG_FR_LIB_POSIX=y
CONFIG_FR_FAT_FATFS=n
CONFIG_CRUSB=n
CONFIG_USE_DRV_HRT_INTERNAL=y

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
# PROJ_CDEFS += CONFIG_FR_MALLOC_FAILED_HANDLE
# PROJ_CDEFS += CONFIG_FR_IDLE_TIMER_TASKCREATE_HANDLE

# PROJ_CINCDIRS += libs/drivers/imu/icm42688p
# CSOURCES += libs/drivers/imu/icm42688p/icm42688_test.c

# PROJ_CINCDIRS += libs/drivers/imu/l3gd20
# CSOURCES += libs/drivers/imu/l3gd20/l3gd20_test.c

PROJ_CINCDIRS += libs/gcl/drivers/imu/mpu6000
CSOURCES += libs/gcl/drivers/imu/mpu6000/mpu6050_test.c

# PROJ_CINCDIRS += libs/drivers/magnetometers/ist8310
# CSOURCES += libs/drivers/magnetometers/ist8310/ist8310_test.c


CPPSOURCES += libs/gcl/drivers/imu/mpu6050/mpu6050.cpp
CPPSOURCES += libs/gcl/drivers/magnetometers/hmc5883/hmc5883l.cpp

PROJ_CINCDIRS += apps
CPPSOURCES += apps/app_bsp_eval/app_main.cpp


CONFIG_LINK_PRINTF_FLOAT:=y

CONFIG_LINK_SCANF_FLOAT:=n

CONFIG_COMPILE_OPTIMIZE:=O1


