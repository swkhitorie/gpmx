
PROJ_NAME  :=  LoRaP2P_sample_wlxx
PROJ_TC    :=  gae
APP_PROJ_DIR = apps/LoRaP2P_sample

CONFIG_C_STANDARD:=gnu11
CONFIG_CXX_STANDARD:=gnu++11
CONFIG_LINK_PRINTF_FLOAT:=n
CONFIG_LINK_SCANF_FLOAT:=n
CONFIG_COMPILE_OPTIMIZE:=O1
TARGET_POSTBUILD := ${TARGET_DEST_FILENAME_BIN}

# board configuration
# stm32wl55_nucleo_wl55jc
# stm32wle5_e77_900mbl
include ${SDK_ROOTDIR}/boards/stm32/stm32wl55_nucleo_wl55jc/make.mk

# os and library config
MK_COMPILER=gcc
MK_RTOS=none
MK_MEM_METHOD=4
MK_USE_POSIX=n
MK_USE_LIB_CPP=n

MK_USE_FS_FATFS=n
MK_USE_CRUSB=n
MK_USE_CRUSB_CDC=n
MK_CRUSB_IP=n

MK_USE_PX4_HRT=n
MK_USE_PX4_COMPONENT=n
MK_USE_PX4_UORB=n
MK_USE_PX4_WORKQUEUE=n

PROJ_CDEFS += CONFIG_BOARD_COM_STDINOUT
#PROJ_CDEFS += CONFIG_BOARD_COM_STDOUT_DMA

PROJ_CINCDIRS += ${APP_PROJ_DIR}
CPPSOURCES += ${APP_PROJ_DIR}/app_main.cpp

PROJ_CINCDIRS += apps/src/libs/rtklib
CPPSOURCES += apps/src/libs/rtklib/rtkcmn.cpp
CPPSOURCES += apps/src/libs/rtklib/rtcm3.cpp
CPPSOURCES += apps/src/libs/rtklib/rtcm3_bsmsg.cpp

PROJ_CINCDIRS += apps/src/libs/LoRaP2P/
CSOURCES += apps/src/libs/LoRaP2P/platforms/p2p_platform_wlxx.c
CSOURCES += apps/src/libs/LoRaP2P/proto/p2p_proto.c
CSOURCES += apps/src/libs/LoRaP2P/proto/p2p_rbuffer.c
CSOURCES += apps/src/libs/LoRaP2P/region/region_EU868.c
CSOURCES += apps/src/libs/LoRaP2P/region/region_US915.c
CSOURCES += apps/src/libs/LoRaP2P/region/region_CN470.c
CSOURCES += apps/src/libs/LoRaP2P/rand_lcg_series.c
CSOURCES += apps/src/libs/LoRaP2P/p2p_common.c
CSOURCES += apps/src/libs/LoRaP2P/p2p_state.c
CSOURCES += apps/src/libs/LoRaP2P/p2p_linkfind.c
CSOURCES += apps/src/libs/LoRaP2P/p2p_raw.c
CSOURCES += apps/src/libs/LoRaP2P/p2p_rawack.c
CSOURCES += apps/src/libs/LoRaP2P/p2p_rawack_fhss.c
CSOURCES += apps/src/libs/LoRaP2P/p2p_pconfig.c



