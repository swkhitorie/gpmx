
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

PROJ_CINCDIRS += ${APP_PROJ_DIR}
CPPSOURCES += ${APP_PROJ_DIR}/app_main.cpp
CPPSOURCES += ${APP_PROJ_DIR}/gsh_main.cpp

include ${SDK_ROOTDIR}/apps/libs/config.mk
CPPSOURCES += apps/libs/drivers/imu/mpu6050/MPU6050.cpp

# PROJ_CINCDIRS += ${APP_PROJ_DIR}/mavlink
# ALL_EXT_GROUPS := mavlink
# mavlink_OPT := -Wno-address-of-packed-member -Wno-cast-align
# # mavlink_SRC += mavlink_command_sender.cpp
# # mavlink_SRC += mavlink_events.cpp
# # mavlink_SRC += mavlink_ftp.cpp
# # mavlink_SRC += mavlink_log_handler.cpp
# # mavlink_SRC += mavlink_main.cpp
# mavlink_SRC += mavlink_messages.cpp
# # mavlink_SRC += mavlink_mission.cpp
# # mavlink_SRC += mavlink_parameters.cpp
# mavlink_SRC += mavlink_rate_limiter.cpp
# mavlink_SRC += mavlink_receiver.cpp
# # mavlink_SRC += mavlink_shell.cpp
# mavlink_SRC += mavlink_simple_analyzer.cpp
# mavlink_SRC += mavlink_stream.cpp
# mavlink_SRC += mavlink_timesync.cpp
# mavlink_SRC += mavlink_ulog.cpp
# mavlink_SRC += MavlinkStatustestHandler.cpp
# mavlink_SRC += mavlink.cpp
# CPPSOURCES += $(foreach src,${mavlink_SRC},${APP_PROJ_DIR}/mavlink/$(src))
