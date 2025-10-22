
PROJ_NAME  :=  rawlorap2p_ota_slave
PROJ_TC    :=  gae
APP_PROJ_DIR = apps/lorap2p_sample/rawp2p_sv_slave

CONFIG_LINK_PRINTF_FLOAT:=n
CONFIG_LINK_SCANF_FLOAT:=n
CONFIG_COMPILE_OPTIMIZE:=-O1
CONFIG_C_STANDARD:=gnu11
CONFIG_CXX_STANDARD:=gnu++11
CONFIG_LIB_USE_NANO:=y
TARGET_POSTBUILD := ${TARGET_DEST_FILENAME_BIN}

include ${SDK_ROOTDIR}/boards/stm32/nucleo_wl55jc/make.mk

# os and library config
MK_RTOS=none
MK_RTOS_PLATFORM=gcc
MK_RTOS_MEM_METHOD=4
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

PROJ_SCF_DEFS += -DLR_CODE_BASE=0x08000000
PROJ_SCF_DEFS += -DLR_CODE_SIZE=254K

PROJ_SCF_DEFS += -DNVM_DATA_BASE=0x0803f800
PROJ_SCF_DEFS += -DNVM_DATA_SIZE=2K

PROJ_CDEFS += CONFIG_BOARD_COM_STDINOUT
#PROJ_CDEFS += CONFIG_BOARD_COM_STDOUT_DMA

PROJ_CDEFS += RADIO_BOARD_ROLE=2
PROJ_CDEFS += LORAP2P_SAVE
PROJ_CDEFS += P2P_REGION_EU868
PROJ_CDEFS += P2P_ROLE_SLAVE
PROJ_CDEFS += P2P_MODE_RAWACK_FHSS

PROJ_CINCDIRS += ${APP_PROJ_DIR}
CPPSOURCES += ${APP_PROJ_DIR}/app_main.cpp

PROJ_CINCDIRS += apps/src/libs/ymodem/
CSOURCES += apps/src/libs/ymodem/ymdm_cmn.c
CSOURCES += apps/src/libs/ymodem/ymdm_rcv.c

PROJ_CINCDIRS += ${APP_PROJ_DIR}/../nvm/
CPPSOURCES += ${APP_PROJ_DIR}/../nvm/nvm_board.cpp

CPPSOURCES += ${APP_PROJ_DIR}/lorap2p.cpp
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



