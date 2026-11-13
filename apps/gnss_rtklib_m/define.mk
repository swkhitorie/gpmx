
# compile config
BUILD_COMPILE_OPTIMIZE:=-O1 #-flto
BUILD_C_STANDARD:=gnu11
BUILD_CXX_STANDARD:=gnu++11
BUILD_LINK_PRINTF_FLOAT:=y
BUILD_LINK_SCANF_FLOAT:=n
BUILD_LIB_USE_NANO:=y

CONFIG_TEST=n
CONFIG_POSIXRUN_ENABLE=n
CONFIG_FREERTOS_ENABLE=y
CONFIG_RTTNANO_ENABLE=n
CONFIG_RTOS_COMPILER="gcc"
CONFIG_RTOS_MEM_METHOD=4
CONFIG_GMSH=y
CONFIG_FINSH=n

CONFIG_CMBACKTRACE=y
CONFIG_HRT=y
CONFIG_KPRINTF=y

CONFIG_LIBC_QUEUE=y
CONFIG_LIBC_MQUEUE=n
CONFIG_LIBC_PTHREAD=n
CONFIG_LIBC_SEMAPHORE=n
CONFIG_LIBC_SEMAPHORE_INHERIT=n
CONFIG_LIBC_TIMER=n
CONFIG_LIBC_TIME=y
CONFIG_LIBCPP=n
CONFIG_LIBC_VFS=n

CONFIG_FS_FATFS=y

CONFIG_MM_RTXMEM=y

ifeq (${REPLAYMODE},y)
PROJ_CDEFS += CONFIG_RTK_STREAM_REPLAY_MODE
endif

PROJ_CDEFS += configAPPLICATION_ALLOCATED_HEAP=1
PROJ_CDEFS += GSH_THREAD_STACK_SIZE=10*1024

# run board pxboard_ebfv2
PROJ_CDEFS += SERIAL3_CONFIG
PROJ_CDEFS += SERIAL3_DMA_RXBUFFER_LEN=1024*16
PROJ_CDEFS += SERIAL3_DMA_TXBUFFER_LEN=256
PROJ_CDEFS += SERIAL3_TXBUFFER_LEN=256
PROJ_CDEFS += SERIAL3_RXBUFFER_LEN=1024*8
PROJ_CDEFS += SERIAL3_BAUDRATE=460800

PROJ_CINCDIRS += ${APP_PROJ_DIR}
CPPSOURCES += ${APP_PROJ_DIR}/app_main.cpp

CSOURCES += ${APP_PROJ_DIR}/rtk_server.c
CSOURCES += ${APP_PROJ_DIR}/rtk_stream.c

PROJ_CINCDIRS += apps/libs/rtklib
# CSOURCES += apps/libs/rtklib/datum.c
CSOURCES += apps/libs/rtklib/ephemeris.c
CSOURCES += apps/libs/rtklib/geoid.c
# CSOURCES += apps/libs/rtklib/gis.c
# CSOURCES += apps/libs/rtklib/ionex.c
CSOURCES += apps/libs/rtklib/lambda.c
# CSOURCES += apps/libs/rtklib/options.c
CSOURCES += apps/libs/rtklib/pntpos.c
CSOURCES += apps/libs/rtklib/postpos.c
# CSOURCES += apps/libs/rtklib/ppp_ar.c
# CSOURCES += apps/libs/rtklib/ppp.c
# CSOURCES += apps/libs/rtklib/preceph.c
CSOURCES += apps/libs/rtklib/rtcm.c
CSOURCES += apps/libs/rtklib/rtcm3.c
# CSOURCES += apps/libs/rtklib/rtcm3e.c
CSOURCES += apps/libs/rtklib/rtkcmn.c
CSOURCES += apps/libs/rtklib/rtkpos.c
CSOURCES += apps/libs/rtklib/rtksvr.c
# CSOURCES += apps/libs/rtklib/sbas.c
CSOURCES += apps/libs/rtklib/solution.c
# CSOURCES += apps/libs/rtklib/stream.c
# CSOURCES += apps/libs/rtklib/tides.c
# CSOURCES += apps/libs/rtklib/tle.c

