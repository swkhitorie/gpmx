
ifneq ($(OS), Linux)
TC_PATH_INST_GCC := $(shell where arm-none-eabi-gcc)/../../
TC_PATH_INST_ARMCC := $(shell where armcc)/../../
TC_PATH_INST_ARMCLANG := $(shell where armclang)/../../
TC_OPENOCD_PATH := $(shell where openocd)/../../
endif

# Path to root dir
SDK_DIR_MK = $(dir $(realpath $(lastword ${MAKEFILE_LIST})))
SDK_DIR_CUR = $(CURRENT_DIR)
SDK_ROOTDIR := ${SDK_DIR_MK}

# Absolute path to makefiles
MAKEFILES_ROOTDIR := ${SDK_ROOTDIR}/make

MAKE_TARGET_CLEANS := y
ifeq (,$(findstring clean,$(MAKECMDGOALS))$(findstring distclean,$(MAKECMDGOALS)))
MAKE_TARGET_CLEANS := n
endif

CONFIG_FILE := ${SDK_ROOTDIR}/apps/app_config.mk
include ${CONFIG_FILE}

# including common build makefile
include $(MAKEFILES_ROOTDIR)/make/build.mk
all: prebuild build postbuild
