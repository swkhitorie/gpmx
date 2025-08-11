
PROJ_NAME  :=  lora_p2p_wlxx
PROJ_TC    :=  gae
APP_PROJ_DIR = apps/demo/wlxx_lora_p2p

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

PROJ_CINCDIRS += ${APP_PROJ_DIR}
PROJ_CINCDIRS += ${APP_PROJ_DIR}/loraP2P/
PROJ_CINCDIRS += ${APP_PROJ_DIR}/rtklib/

CPPSOURCES += ${APP_PROJ_DIR}/app_main.cpp
CPPSOURCES += ${APP_PROJ_DIR}/ringbuffer.cpp
CPPSOURCES += ${APP_PROJ_DIR}/p2p_proto.cpp
CPPSOURCES += ${APP_PROJ_DIR}/rand_lcg_series.cpp

CPPSOURCES += ${APP_PROJ_DIR}/rtklib/rtcm3.cpp
CPPSOURCES += ${APP_PROJ_DIR}/rtklib/rtkcmn.cpp
CPPSOURCES += ${APP_PROJ_DIR}/rtklib/rtk_basemsg.cpp

CPPSOURCES += ${APP_PROJ_DIR}/loraP2P/region_EU868.cpp
CPPSOURCES += ${APP_PROJ_DIR}/loraP2P/region_US915.cpp
CPPSOURCES += ${APP_PROJ_DIR}/loraP2P/p2p_common.cpp
CPPSOURCES += ${APP_PROJ_DIR}/loraP2P/p2p_if.cpp

CPPSOURCES += ${APP_PROJ_DIR}/loraP2P/p2p_state.cpp
CPPSOURCES += ${APP_PROJ_DIR}/loraP2P/p2p_linkfind.cpp
CPPSOURCES += ${APP_PROJ_DIR}/loraP2P/p2p_raw.cpp
CPPSOURCES += ${APP_PROJ_DIR}/loraP2P/p2p_rawack.cpp
CPPSOURCES += ${APP_PROJ_DIR}/loraP2P/p2p_rawack_fhss.cpp



