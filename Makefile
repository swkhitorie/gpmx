# if gpmx is located in a subdirectories under the user project directory
# copy the Makefile to your root dir
# remove SDK_ROOTDIR := ${SDK_DIR_MK}
#        CONFIG_FILE := ${SDK_ROOTDIR}/apps/app_config.mk
#
# add    SDK_ROOTDIR := ${SDK_DIR_MK}/your_gpmx_path/gpmx
#        USR_ROOTDIR := ${SDK_DIR_MK}
#        CONFIG_FILE := ${USR_ROOTDIR}/your_config_path/app_config.mk
# file(src/inc) mk variable our of gpmx should change prefix to USR_ 
# use USCF_FILE=your_scatter_file if you want to use your own bsp link file
# copy compile script ../gpmx/make/, and use it

# Path to root dir
SDK_DIR_MK = $(dir $(realpath $(lastword ${MAKEFILE_LIST})))
SDK_DIR_CUR = $(CURRENT_DIR)

SDK_ROOTDIR := ${SDK_DIR_MK}
CONFIG_FILE := ${SDK_ROOTDIR}/apps/app_config.mk
include ${CONFIG_FILE}

# Absolute path to makefiles
MAKEFILES_ROOTDIR := ${SDK_ROOTDIR}/make

# including common build makefile
include $(MAKEFILES_ROOTDIR)/make/build.mk
all: prebuild build postbuild
