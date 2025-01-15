
# Path to root dir
SDK_DIR_MK = $(dir $(realpath $(lastword ${MAKEFILE_LIST})))
SDK_DIR_CUR = $(CURRENT_DIR)
SDK_ROOTDIR := ${SDK_DIR_MK}

# Absolute path to makefiles
MAKEFILES_ROOTDIR := ${SDK_ROOTDIR}/make

CONFIG_FILE := ${SDK_ROOTDIR}/app/app_config.mk
include ${CONFIG_FILE}

TC_PATH_INST_ARMCC := C:\Keil_v5\ARM\ARMCC
TC_PATH_INST_ARMCLANG := C:\Keil_v5\ARM\ARMCLANG
TC_PATH_INST_GCC:= D:\envir\gcc-arm-none-eabi
TC_OPENOCD_PATH := D:\envir\openocd\xpack-openocd-0.11.0-1

# including common build makefile
include $(MAKEFILES_ROOTDIR)/common/build.mk

#PROJ_OPENOCD_DEBUG := stlink
#PROJ_OPENOCD_CHIP := stm32h7x
#PROJ_OPENOCD_LOAD_ADDR := 0x08000000

# example:
# make program PROJ_OPENOCD_DEBUG=stlink PROJ_OPENOCD_CHIP=stm32h7x -j4

all: prebuild build postbuild

