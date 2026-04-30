
PROJ_NAME  :=  rtklib_test
PROJ_TC    :=  gae
APP_PROJ_DIR = apps/rtklib_test

# compile config
CONFIG_LINK_PRINTF_FLOAT:=y
CONFIG_LINK_SCANF_FLOAT:=n
CONFIG_COMPILE_OPTIMIZE:=-O1 #-flto
CONFIG_C_STANDARD:=gnu11
CONFIG_CXX_STANDARD:=gnu++11
CONFIG_LIB_USE_NANO:=y
TARGET_POSTBUILD := ${TARGET_DEST_FILENAME_BIN}

# os and library config
MK_RTOS=frtos
MK_RTOS_PLATFORM=gcc
MK_RTOS_MEM_METHOD=4
MK_USE_HARDFAULTINFO=y

MK_USE_MODULE_RTXMEM=y
MK_USE_MODULE_KPRINTF=y
MK_USE_MODULE_GPMSHELL=y
MK_USE_KERNEL_CPP=n
MK_USE_KERNEL_POSIX_QUEUE=n
MK_USE_KERNEL_POSIX_MQUEUE=n
MK_USE_KERNEL_POSIX_PTHREAD=n
MK_USE_KERNEL_POSIX_SEMAPHORE=n
MK_USE_KERNEL_POSIX_TIMER=n
MK_USE_KERNEL_POSIX_TIME=n
MK_USE_KERNEL_WORKQUEUE=n
MK_USE_KERNEL_HRT=y
MK_USE_KERNEL_PERF=n

MK_USE_FS_FATFS=y
MK_USE_FS_LITTLEFS=n
MK_USE_FS_ROMFS=n
MK_USE_CRUSB=y
MK_USE_CRUSB_IP=dwc2_st
MK_USE_CRUSB_CLASS+=cdc_acm
MK_USE_NET_LWIP_CORE=n

include ${SDK_ROOTDIR}/apps/board_selection.mk

PROJ_CDEFS += configAPPLICATION_ALLOCATED_HEAP=1
PROJ_CDEFS += GSH_THREAD_STACK_SIZE=4096

# run board pxboard_ebfv2
PROJ_CDEFS += SERIAL3_CONFIG
PROJ_CDEFS += SERIAL3_DMA_RXBUFFER_LEN=1024*16
PROJ_CDEFS += SERIAL3_DMA_TXBUFFER_LEN=256
PROJ_CDEFS += SERIAL3_TXBUFFER_LEN=256
PROJ_CDEFS += SERIAL3_RXBUFFER_LEN=1024*8
PROJ_CDEFS += SERIAL3_BAUDRATE=460800

PROJ_CDEFS += CONFIG_RTKLIB_USR_PLATFORM
PROJ_CDEFS += CONFIG_RTKLIB_INPUT_STREAM=2
PROJ_CDEFS += CONFIG_RTKLIB_OUTPUT_STREAM=1
PROJ_CDEFS += NOCALLOC
PROJ_CDEFS += NFREQ=3
PROJ_CDEFS += ENACMP
PROJ_CDEFS += ENAGAL

PROJ_CINCDIRS += ${APP_PROJ_DIR}/
CPPSOURCES += ${APP_PROJ_DIR}/app_main.cpp
CPPSOURCES += ${APP_PROJ_DIR}/gsh_main.cpp

PROJ_CINCDIRS += ${APP_PROJ_DIR}/rtkplatform/
CSOURCES += ${APP_PROJ_DIR}/rtkplatform/stream.c
CSOURCES += ${APP_PROJ_DIR}/rtkplatform/rtkrcv.c

PROJ_CINCDIRS += apps/libs/rtklib
CSOURCES += apps/libs/rtklib/datum.c
CSOURCES += apps/libs/rtklib/ephemeris.c
CSOURCES += apps/libs/rtklib/geoid.c
CSOURCES += apps/libs/rtklib/gis.c
CSOURCES += apps/libs/rtklib/ionex.c
CSOURCES += apps/libs/rtklib/lambda.c
CSOURCES += apps/libs/rtklib/options.c
CSOURCES += apps/libs/rtklib/pntpos.c
CSOURCES += apps/libs/rtklib/postpos.c
CSOURCES += apps/libs/rtklib/ppp_ar.c
CSOURCES += apps/libs/rtklib/ppp.c
CSOURCES += apps/libs/rtklib/preceph.c
CSOURCES += apps/libs/rtklib/rtcm.c
CSOURCES += apps/libs/rtklib/rtcm3.c
CSOURCES += apps/libs/rtklib/rtcm3e.c
CSOURCES += apps/libs/rtklib/rtkcmn.c
CSOURCES += apps/libs/rtklib/rtkpos.c
CSOURCES += apps/libs/rtklib/rtksvr.c
CSOURCES += apps/libs/rtklib/sbas.c
CSOURCES += apps/libs/rtklib/solution.c
# CSOURCES += apps/libs/rtklib/stream.c
CSOURCES += apps/libs/rtklib/tides.c
CSOURCES += apps/libs/rtklib/tle.c

