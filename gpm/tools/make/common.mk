################################################################################
#
# common.mk
#
# This makefiles defines all variables and macros used by build makefiles
#
################################################################################

include ${MAKEFILES_ROOTDIR}/make/macros.mk

#####################################
# folder and file defs
#####################################

# folder for tools
TOOLS_DIR := ${MAKEFILES_ROOTDIR}

# folder for common makefiles
MAKEFILES_COMMONDIR := ${MAKEFILES_ROOTDIR}/make

# folder for project files
MAKEFILES_PROJDIR := ${MAKEFILES_ROOTDIR}/${PROJ_NAME}

# folder for target files
TARGET_ROOTDIR_RAW := build/${PROJ_NAME}
ifneq (${USR_ROOTDIR},)
TARGET_ROOTDIR := ${USR_ROOTDIR}/build/${PROJ_NAME}
else
TARGET_ROOTDIR := ${SDK_ROOTDIR}/build/${PROJ_NAME}
endif

# folder for binary outputs
COMMON_BINDIR := bin

# objects folder
OBJS_FOLDER := ${TARGET_ROOTDIR}/objs

# Project related defines
TCDEFSINC := ${MAKEFILES_COMMONDIR}/tc_${PROJ_TC}_defs.mk

# Core related defines
COREDEFSINC := ${MAKEFILES_COMMONDIR}/core_${MOD_ARCH}_defs.mk

# List of all incdefs files
DEFSINCLIST := ${TCDEFSINC} ${COREDEFSINC}

#####################################
# include toolchain, core and OS definitions
#####################################
include ${TCDEFSINC}
ifeq (${MAKE_TARGET_CLEANS},n)
include ${COREDEFSINC}
endif

# definitions for binary folder target outputs
ifneq (${USR_ROOTDIR},)
TARGET_DEST_ROOTDIR       := ${USR_ROOTDIR}/${COMMON_BINDIR}
else
TARGET_DEST_ROOTDIR       := ${SDK_ROOTDIR}/${COMMON_BINDIR}
endif
ifeq (${COMPILE_TIME},)
COMPILE_TIME              := 0
endif
TARGET_DEST_FILENAME      := ${PROJ_NAME}_${PROJ_TC}_${COMPILE_TIME}
TARGET_DEST_FILENAME_EXE  := ${TARGET_DEST_ROOTDIR}/${TARGET_DEST_FILENAME}.${TC_SUFFIX}
TARGET_DEST_FILENAME_BIN  := ${TARGET_DEST_ROOTDIR}/${TARGET_DEST_FILENAME}.${MOD_BINEXT}
TARGET_DEST_FILENAME_HEX  := ${TARGET_DEST_ROOTDIR}/${TARGET_DEST_FILENAME}.${MOD_HEXEXT}
TARGET_DEST_FILENAME_LIST := ${TARGET_DEST_ROOTDIR}/${TARGET_DEST_FILENAME}.${MOD_LNKLSTEXT}

#####################################
# define target files
#####################################

# Configuration file
# BUILD_CONFIG_FILE := ${MAKEFILES_PROJDIR}/${PROJ_CFG}.mk

# Assembler options file
ASMOPTS_FILE := ${TARGET_ROOTDIR}/asmopts.via

# C options file
COPTS_FILE := ${TARGET_ROOTDIR}/copts.via

# C include file 
CINC_FILE := ${TARGET_ROOTDIR}/cinc.via

# macros file
MACROS_FILE := ${TARGET_ROOTDIR}/macros.via

# c macros file
GLOBAL_CMACROS_PATH := ${TARGET_ROOTDIR}/gpmx
GLOBAL_CMACROS_FILE := ${GLOBAL_CMACROS_PATH}/config.h

# C options file
LOPTS_FILE := ${TARGET_ROOTDIR}/lopts.via

# Project makefile
PROJ_MAKEFILE := ${SDK_ROOTDIR}/Makefile

# scatter file name
ifneq (${USCF_FILE},)
SCF_TEMPLATE_FILE_NAME := ${USR_ROOTDIR}/${USCF_FILE}
else
SCF_TEMPLATE_FILE_NAME := ${SDK_ROOTDIR}/${SCF_FILE}
endif

SCF_FILE_NAME:=${TARGET_ROOTDIR}/${PROJ_NAME}_gen_link.${TC_SCFEXT}

# definitions for local folder target outputs
TARGET_SRC_FILENAME_EXE    :=${TARGET_ROOTDIR}/${PROJ_NAME}.${TC_SUFFIX}
TARGET_LIBS                :=${TARGET_ROOTDIR}/${PROJ_NAME}.${TC_LIB_SUFFIX}
TARGET_SRC_FILENAME_LIST   :=${TARGET_ROOTDIR}/${PROJ_NAME}.${MOD_LNKLSTEXT}
TARGET_SRC_FILENAME_DISASM :=${TARGET_ROOTDIR}/${PROJ_NAME}.list

# definitions for binary folder target outputs
ifneq ($(BUILD_TYPE),lib)
TARGET_POSTBUILD:=${TARGET_DEST_FILENAME_BIN}
endif

#####################################
# Build sources list
#####################################
PROJ_ASMSOURCES    := ${ASMSOURCES} ${EXT_ASMSOURCES}

PROJ_CSOURCES      := ${CSOURCES} ${EXT_CSOURCES}

PROJ_CPPSOURCES    := ${CPPSOURCES} ${EXT_CPPSOURCES}

PROJ_CARMSOURCES   := ${CARMSOURCES} ${EXT_CARMSOURCES}

PROJ_SOURCES       := ${PROJ_ASMSOURCES} ${PROJ_CSOURCES} ${PROJ_CARMSOURCES}

#####################################
# assembler section
#####################################
ASMOPTS:=$(strip ${TC_ASMOPTS} ${TC_DBG_ASMOPTS} ${CORE_ASMOPTS} ${PROJ_ASMOPTS} ${EXT_ASMOPTS})

# CASMDEFS:=$(foreach def,$(strip ${PROJ_CDEFS}),-D${def})
ASMDEFS:=$(strip ${TC_ASMDEFS} ${PROJ_ASMDEFS} ${CASMDEFS} ${OS_ASMDEFS} ${EXT_ASMDEFS})

ASMINCDIRS:=$(foreach inc,$(strip ${TC_ASMINCDIRS} ${PROJ_ASMINCDIRS} ${PROJ_CINCDIRS} ${OS_ASMINCDIRS} ${EXT_ASMINCDIRS}),-I$(call MK_TC_PATH,${SDK_ROOTDIR}/${inc}))

#####################################
# compiler section
#####################################
COPTS:=$(strip ${TC_COPTS} ${TC_DBG_COPTS} ${CORE_COPTS} ${PROJ_COPTS} ${EXT_COPTS})

CPPOPTS:=$(strip ${TC_CPPOPTS} ${TC_DBG_COPTS} ${CORE_COPTS} ${PROJ_COPTS} ${EXT_COPTS})

CDEFS:=$(foreach def,$(strip ${TC_CDEFS} ${OS_CDEFS} ${EXT_CDEFS}),-D${def})

PROJ_CINCDIRS+=${TARGET_ROOTDIR_RAW}
CINCDIRS:=$(foreach inc,$(strip ${PROJ_CINCDIRS} ${OS_CINCDIRS} ${EXT_CINCDIRS}),-I$(call MK_TC_PATH,${SDK_ROOTDIR}/${inc}))
CINCDIRS_FILSRC:=$(foreach inc,$(strip ${PROJ_CINCDIRS} ${OS_CINCDIRS} ${EXT_CINCDIRS}),$(call MK_TC_PATH,${SDK_ROOTDIR}/${inc}))
ifneq (${USR_ROOTDIR},)
# wtf?? bug occurs because of no space front of next CINCDIRS add
CINCDIRS+=
CINCDIRS+=$(foreach inc,$(strip ${USR_CINCDIRS}),-I$(call MK_TC_PATH,${USR_ROOTDIR}/${inc}))
endif

#####################################
# linker section
#####################################
# build libs list to link for ARM
LIBOPTS:=$(strip ${TC_LIBOPTS} ${CORE_LIBOPTS})

LIBDIRS:=$(foreach inc,$(strip ${COMMON_LIBDIR} ${TC_LIBDIRS} ${PROJ_LIBDIRS} ${EXT_LIBDIRS}),$(call MK_TC_PATH,${SDK_ROOTDIR}/${inc}))

LIBS:=${EXT_LIBS}
LIBS+=$(foreach lib,${LIBNAMES},${MOD_CORE}_${MOD_ARCH}_${lib}_${TC_LIBSUFFIX})
LIBS+=${TC_LIBNAMES}

# scatter file defines
SCF_DEFS:=$(strip $(CDEFS) ${PROJ_SCF_DEFS})

