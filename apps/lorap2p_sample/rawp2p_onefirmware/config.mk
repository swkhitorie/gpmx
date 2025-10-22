
PROJ_NAME  :=  rawlorap2p_sample
PROJ_TC    :=  gae
APP_PROJ_DIR = apps/lorap2p_sample/rawp2p_onefirmware

CONFIG_LINK_PRINTF_FLOAT:=n
CONFIG_LINK_SCANF_FLOAT:=n
CONFIG_COMPILE_OPTIMIZE:=-O1
CONFIG_C_STANDARD:=gnu11
CONFIG_CXX_STANDARD:=gnu++11
CONFIG_LIB_USE_NANO:=y
TARGET_POSTBUILD := ${TARGET_DEST_FILENAME_BIN}

# board configuration
# ebyte_e77_900mbl
# nucleo_wl55jc
include ${SDK_ROOTDIR}/boards/stm32/ebyte_e77_900mbl/make.mk

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

# stm32wle5 load addr
PROJ_SCF_DEFS += -DLR_CODE_BASE=0x08000000
PROJ_SCF_DEFS += -DLR_CODE_SIZE=255K

PROJ_SCF_DEFS += -DNVM_DATA_BASE=0x0803FC00
PROJ_SCF_DEFS += -DNVM_DATA_SIZE=1K

PROJ_CDEFS += CONFIG_BOARD_COM_STDINOUT
#PROJ_CDEFS += CONFIG_BOARD_COM_STDOUT_DMA
PROJ_CDEFS += P2P_REGION_EU868
PROJ_CDEFS += RADIO_BOARD_ROLE=1

PROJ_CINCDIRS += ${APP_PROJ_DIR}
CPPSOURCES += ${APP_PROJ_DIR}/app_main.cpp

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



