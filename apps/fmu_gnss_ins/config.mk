
PROJ_NAME  :=  fmu_gnss_ins
PROJ_TC    :=  gae
APP_PROJ_DIR = apps/fmu_gnss_ins

# compile config
CONFIG_LINK_PRINTF_FLOAT:=n
CONFIG_LINK_SCANF_FLOAT:=n
CONFIG_COMPILE_OPTIMIZE:=-O1 #-flto
CONFIG_C_STANDARD:=gnu11
CONFIG_CXX_STANDARD:=gnu++14
CONFIG_LIB_USE_NANO:=y
TARGET_POSTBUILD := ${TARGET_DEST_FILENAME_BIN}

# os and library config
MK_RTOS=frtos
MK_RTOS_PLATFORM=gcc
MK_RTOS_MEM_METHOD=4
MK_USE_HARDFAULTINFO=y

MK_USE_MODULE_RTXMEM=n
MK_USE_MODULE_KPRINTF=y
MK_USE_MODULE_GPMSHELL=y
MK_USE_KERNEL_CPP=y
MK_USE_KERNEL_POSIX_QUEUE=y
MK_USE_KERNEL_POSIX_MQUEUE=n
MK_USE_KERNEL_POSIX_PTHREAD=y
MK_USE_KERNEL_POSIX_SEMAPHORE=y
MK_USE_KERNEL_POSIX_TIMER=n
MK_USE_KERNEL_POSIX_TIME=y
MK_USE_KERNEL_WORKQUEUE=y
MK_USE_KERNEL_HRT=y
MK_USE_KERNEL_PERF=y

MK_USE_FS_FATFS=y
MK_USE_FS_LITTLEFS=n
MK_USE_FS_ROMFS=n
MK_USE_NET_LWIP_CORE=n
MK_USE_CRUSB=y
MK_USE_CRUSB_CLASS=cdc_acm
MK_USE_CRUSB_IP=dwc2_st

include ${SDK_ROOTDIR}/apps/board_selection.mk

PROJ_CDEFS += configAPPLICATION_ALLOCATED_HEAP=0

# mavlink including 
PROJ_CINCDIRS += $(subst ${SDK_ROOTDIR},,$(subst ;, ,$(MAVLINK_INCLUDING)))     
PROJ_CINCDIRS += ${APP_PROJ_DIR}/mavlink

# uorb include and source
UORB_CINCDIRSS = $(subst ;,,$(UORB_INCLUDING))
PROJ_CINCDIRS += $(subst ${SDK_ROOTDIR},,$(subst ;, ,$(UORB_INCLUDING)))
CPPSOURCES += $(subst ${SDK_ROOTDIR},,$(wildcard ${UORB_CINCDIRSS}/msg/topics_sources/*cpp))

PROJ_CINCDIRS += ${APP_PROJ_DIR}
CPPSOURCES += ${APP_PROJ_DIR}/app_main.cpp
CPPSOURCES += ${APP_PROJ_DIR}/gsh_main.cpp

PROJ_CINCDIRS += apps
PROJ_CINCDIRS += apps/libs
PROJ_CINCDIRS += apps/libs/matrix

CPPSOURCES += ${APP_PROJ_DIR}/drivers/imu/mpu6050/MPU6050.cpp

# PROJ_CINCDIRS += boards/stm32/pxboard_ebfv2/driver
# CPPSOURCES += boards/stm32/pxboard_ebfv2/driver/mpu6050.cpp

CPPSOURCES += apps/libs/conversion/rotation.cpp
CPPSOURCES += apps/libs/drivers/accelerometer/xAccelerometer.cpp
CPPSOURCES += apps/libs/drivers/gyroscope/xGyroscope.cpp

CPPSOURCES += apps/libs/drivers/device/i2cbus.cpp
CPPSOURCES += apps/libs/drivers/device/spibus.cpp

