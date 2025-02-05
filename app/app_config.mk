
#
# application configuration file
#
PROJ_NAME  :=  test

#
# Toolchain used in this project
#
PROJ_TC  :=  gae

TARGET_POSTBUILD := ${TARGET_DEST_FILENAME_BIN}

#####################################
# bsp configuration
#####################################

BSP_ROOT           := app/../
BSP_ABSROOTDIR     := ${SDK_ROOTDIR}/${BSP_ROOT}

CONFIG_PX4_HRTIMER := y

#include ${BSP_ABSROOTDIR}/stm32/stm32h743_eval/make.mk
#include ${BSP_ABSROOTDIR}/stm32/stm32h743_fmuv6/make.mk
#include ${BSP_ABSROOTDIR}/stm32/stm32f103_eval/make.mk
#include ${BSP_ABSROOTDIR}/stm32/stm32f407_eval/make.mk
include ${BSP_ABSROOTDIR}/stm32/stm32f427_fmuv2/make.mk

PROJ_CDEFS        += ${BSP_CDEFS}

CSOURCES          += ${BSP_CSRCS}

PROJ_CINCDIRS     += ${BSP_CINCDIRS}

ASMSOURCES        := ${BSP_ASMSOURCES}

SCF_FILE          := ${BSP_LNK_FILE}

PROJ_ENTRY_POINT  := ${BSP_BOARD_ENTRY_POINT}

PROJ_OPENOCD_LOAD_ADDR := 0x08000000

MOD_ARCH  =  m7


#####################################
# app configuration
#####################################

PROJ_CDEFS += BSP_COM_PRINTF

PROJ_CINCDIRS += app

CPPSOURCES += app/stm32_eval/app_main.cpp
CSOURCES += app/stm32_eval/app_syscall.c
