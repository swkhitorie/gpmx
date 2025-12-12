#
# Check configuration
#

ifeq (${SDK_ROOTDIR},)
$(error Invalid empty ROOTDIR)
endif

ifeq (${MAKEFILES_ROOTDIR},)
$(error Invalid empty makefiles path)
endif

ifeq (${TC_NAME},armcc)
ifeq (${TC_PATH_INST_ARMCC},)
$(error Invalid empty armcc compiler path)
endif
endif

ifeq (${TC_NAME},armclang)
ifeq (${TC_PATH_INST_ARMCLANG},)
$(error Invalid empty armclang compiler path)
endif
endif

ifeq (${TC_NAME},gae)
ifeq (${TC_PATH_INST_GCC},)
$(error Invalid empty arm gcc compiler path)
endif
endif

ifeq (${TC_USE_PROGRAM},yes)
ifeq (${TC_OPENOCD_PATH},)
$(error Invalid empty openocd path)
endif

ifeq (${TC_OPENOCD_DEBUG_CFG_PATH},)
$(error Invalid empty openocd interface path)
endif

ifeq (${TC_OPENOCD_CHIP_CFG_PATH},)
$(error Invalid empty openocd target path)
endif
endif

ifeq (${TC_FPU_OPTION},)
$(error empty fpu select)
endif
