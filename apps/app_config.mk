
# application configuration file
PROJ_NAME  :=  test

# Toolchain used in this project
PROJ_TC  :=  gae

TARGET_POSTBUILD := ${TARGET_DEST_FILENAME_BIN}

#####################################
# bsp configuration
#####################################
MOD_ARCH  =  m7
include ${SDK_ROOTDIR}/bsp/stm32/stm32h743_eval/make.mk
#include ${SDK_ROOTDIR}/bsp/stm32/stm32h743_fmuv6/make.mk
#include ${SDK_ROOTDIR}/bsp/stm32/stm32f103_eval/make.mk
#include ${SDK_ROOTDIR}/bsp/stm32/stm32f407_eval/make.mk
#include ${SDK_ROOTDIR}/bsp/stm32/stm32f427_fmuv2/make.mk

PROJ_CDEFS += ${BSP_CDEFS}
CSOURCES += ${BSP_CSRCS}
PROJ_CINCDIRS += ${BSP_CINCDIRS}
ASMSOURCES := ${BSP_ASMSOURCES}
SCF_FILE := ${BSP_LNK_FILE}
PROJ_ENTRY_POINT := ${BSP_BOARD_ENTRY_POINT}

#####################################
# platform os configuration
#####################################
CONFIG_FR_ARCH=${MOD_ARCH}
CONFIG_FR_TOOLCHAIN=gcc
CONFIG_FR_MEM_METHOD=4
CONFIG_FR_LIB_CXX=n
CONFIG_FR_LIB_PX4_SUPPORT=n
CONFIG_FR_LIB_POSIX=y
CONFIG_FR_FAT_FATFS=y
CONFIG_CRUSB=y

include ${SDK_ROOTDIR}/sched/make.mk
include ${SDK_ROOTDIR}/mm/make.mk
include ${SDK_ROOTDIR}/libs/make.mk
include ${SDK_ROOTDIR}/include/make.mk
include ${SDK_ROOTDIR}/fs/make.mk
include ${SDK_ROOTDIR}/usb/make.mk

CSOURCES += ${FR_CSOURCES}
CPPSOURCES += ${FR_CPPSOURCES}
PROJ_CINCDIRS += ${FR_CINCDIRS}

#####################################
# app configuration
#####################################
PROJ_OPENOCD_LOAD_ADDR := 0x08000000

PROJ_CDEFS += BSP_COM_PRINTF
PROJ_CDEFS += BSP_MODULE_FR
# PROJ_CDEFS += BSP_MODULE_USB_CHERRY
# PROJ_CDEFS += CRUSB_STD_INOUT_ENABLE

PROJ_CINCDIRS += apps
CPPSOURCES += apps/app_bsp_eval/app_main.cpp
