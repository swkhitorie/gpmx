#
# Check configuration
#

define newline


endef

_MAKE_MAJOR := $(word 1,$(subst ., ,$(MAKE_VERSION)))
# If major version is 0, 1, 2, or 3 -> too old
ifneq ($(filter $(_MAKE_MAJOR),0 1 2 3),)
    $(error GNU Make $(MAKE_VERSION) is too old. Version 4.0 or later is required. Please upgrade.)
endif

ifeq (${SDK_ROOTDIR},)
$(error Invalid empty ROOTDIR)
endif

ifeq (${MAKEFILES_ROOTDIR},)
$(error Invalid empty makefiles path)
endif

ifeq (${TC_NAME},armcc)
ifeq (${TC_PATH_INST_ARMCC}|$(filter build,$(MAKECMDGOALS)),build)
$(error Invalid empty armcc compiler path)
endif
endif

ifeq (${TC_NAME},armclang)
ifeq (${TC_PATH_INST_ARMCLANG}|$(filter build,$(MAKECMDGOALS)),build)
$(error Invalid empty armclang compiler path)
endif
endif

ifeq (${TC_NAME},gae)
ifeq (${TC_PATH_INST_GCC}|$(filter build,$(MAKECMDGOALS)),build)
$(error Invalid empty arm gcc compiler path)
endif
endif

ifeq (${TC_USE_PROGRAM},yes)
ifeq (${TC_OPENOCD_PATH}|$(filter program,$(MAKECMDGOALS)),program)
$(error Invalid empty openocd path)
endif

ifeq (${TC_OPENOCD_DEBUG_CFG_PATH}|$(filter program,$(MAKECMDGOALS)),program)
$(error Invalid empty openocd interface path)
endif

ifeq (${TC_OPENOCD_CHIP_CFG_PATH}|$(filter program,$(MAKECMDGOALS)),program)
$(error Invalid empty openocd target path)
endif
endif

ifneq ($(BUILD_TYPE),app)
ifeq (${TC_FPU_OPTION}|$(filter build,$(MAKECMDGOALS)),build)
$(error empty fpu select)
endif
endif
