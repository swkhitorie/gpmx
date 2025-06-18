
PROJ_NAME  :=  nucleo_wl55jc_p2p
PROJ_TC    :=  gae

CONFIG_LINK_PRINTF_FLOAT:=n
CONFIG_LINK_SCANF_FLOAT:=n
CONFIG_COMPILE_OPTIMIZE:=O1

TARGET_POSTBUILD := ${TARGET_DEST_FILENAME_BIN}

# board configuration
MOD_ARCH = m4
include ${SDK_ROOTDIR}/boards/stm32/stm32wl55_nucleo_wl55jc/make.mk
#stm32wl55_nucleo_wl55jc
#stm32wle5_e77_900mbl

# os and library configuration
CONFIG_MK_CHIP_ARCH=${MOD_ARCH}
CONFIG_MK_COMPILER=gcc
CONFIG_MK_USE_FREERTOS=n
CONFIG_MK_USE_FRMEM_METHOD=4
CONFIG_MK_USE_FR_POSIX=n
CONFIG_MK_USE_FS_FATFS=n
CONFIG_MK_USE_LIB_CPP=n
CONFIG_MK_USE_UORB=n
CONFIG_MK_USE_HRT=n

CONFIG_MK_USE_CRUSB=n
CONFIG_MK_USE_CRUSB_CDC=n
CONFIG_MK_CRUSB_IP=none

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
# macro list: 
# bsp: 
#     CONFIG_BOARD_COM_STDINOUT
#     CONFIG_BOARD_FREERTOS_ENABLE
#     CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE
#     CONFIG_BOARD_MTD_QSPIFLASH_ENABLE
#     CONFIG_BOARD_MTD_QSPIFLASH_FATFS_SUPPORT
#     CONFIG_BOARD_MTD_QSPIFLASH_RAW_RW_TEST
#     CONFIG_BOARD_MMCSD_ENABLE
#     CONFIG_BOARD_MMCSD_FATFS_SUPPORT
#     CONFIG_BOARD_MMCSD_RAW_RW_TEST
#     CONFIG_BOARD_MMCSD_INFO_CHECK
# rtos:
#     CONFIG_FR_MALLOC_FAILED_HANDLE
#     CONFIG_FR_IDLE_TIMER_TASKCREATE_HANDLE
# usb:
#     CONFIG_CRUSB_CDC_TX_FIFO_ENABLE
#     CONFIG_BOARD_CRUSB_CDC_ACM_STDINOUT
#####################################
PROJ_CDEFS += CONFIG_BOARD_COM_STDINOUT

PROJ_CINCDIRS += apps/test/wlxx_lora_p2p/
PROJ_CINCDIRS += apps/test/wlxx_lora_p2p/loraP2P/
PROJ_CINCDIRS += apps/test/wlxx_lora_p2p/rtklib/

CPPSOURCES += apps/test/wlxx_lora_p2p/app_main.cpp
CSOURCES += apps/test/wlxx_lora_p2p/ringbuffer.c

CPPSOURCES += apps/test/wlxx_lora_p2p/loraP2P/region_EU868.cpp
CPPSOURCES += apps/test/wlxx_lora_p2p/loraP2P/region_US915.cpp
CPPSOURCES += apps/test/wlxx_lora_p2p/loraP2P/lora_common.cpp
CPPSOURCES += apps/test/wlxx_lora_p2p/loraP2P/lora_p2p_proto.cpp
CPPSOURCES += apps/test/wlxx_lora_p2p/loraP2P/rand_lcg_series.cpp
CPPSOURCES += apps/test/wlxx_lora_p2p/loraP2P/p2p_state.cpp
CPPSOURCES += apps/test/wlxx_lora_p2p/loraP2P/p2p_linkfind.cpp
CPPSOURCES += apps/test/wlxx_lora_p2p/loraP2P/p2p_raw.cpp
CPPSOURCES += apps/test/wlxx_lora_p2p/loraP2P/p2p_rawack.cpp
CPPSOURCES += apps/test/wlxx_lora_p2p/loraP2P/p2p_rawack_fhss.cpp

CPPSOURCES += apps/test/wlxx_lora_p2p/rtklib/rtcm3.cpp
CPPSOURCES += apps/test/wlxx_lora_p2p/rtklib/rtkcmn.cpp
