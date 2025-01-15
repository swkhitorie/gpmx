################################################################################
#
# common.mk
#
# This makefiles defines all variables and macros used by build makefiles
#
################################################################################

include ${MAKEFILES_ROOTDIR}/common/macros.mk

#####################################
# folder and file defs
#####################################

# folder for tools
TOOLS_DIR := ${SDK_ROOTDIR}/tools

# folder for common makefiles
MAKEFILES_COMMONDIR := ${MAKEFILES_ROOTDIR}/common

# folder for project files
MAKEFILES_PROJDIR := ${MAKEFILES_ROOTDIR}/${PROJ_NAME}

# folder for target files
TARGET_ROOTDIR := ${SDK_ROOTDIR}/build/output/${MOD_CORE}/${PROJ_NAME}

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
include ${COREDEFSINC}

# definitions for binary folder target outputs
TARGET_DEST_ROOTDIR       := ${SDK_ROOTDIR}/${COMMON_BINDIR}
TARGET_DEST_FILENAME      := ${PROJ_NAME}_${PROJ_TC}_${COMPILE_TIME}
TARGET_DEST_FILENAME_EXE  := ${TARGET_DEST_ROOTDIR}/${TARGET_DEST_FILENAME}.${TC_SUFFIX}
TARGET_DEST_FILENAME_BIN  := ${TARGET_DEST_ROOTDIR}/${TARGET_DEST_FILENAME}.${MOD_BINEXT}
TARGET_DEST_FILENAME_LIST := ${TARGET_DEST_ROOTDIR}/${TARGET_DEST_FILENAME}.${MOD_LNKLSTEXT}

#####################################
# define target files
#####################################

# Configuration file
# CONFIG_FILE := ${MAKEFILES_PROJDIR}/${PROJ_CFG}.mk

# Assembler options file
ASMOPTS_FILE := ${TARGET_ROOTDIR}/asmopts.via

# C options file
COPTS_FILE := ${TARGET_ROOTDIR}/copts.via

# C options file
LOPTS_FILE := ${TARGET_ROOTDIR}/lopts.via

# Project makefile
PROJ_MAKEFILE := ${SDK_ROOTDIR}/Makefile

# scatter file name
SCF_FILE_NAME := ${SDK_ROOTDIR}/${SCF_FILE}


# definitions for local folder target outputs
TARGET_SRC_FILENAME_EXE    :=${TARGET_ROOTDIR}/${PROJ_NAME}.${TC_SUFFIX}
TARGET_SRC_FILENAME_LIST   :=${TARGET_ROOTDIR}/${PROJ_NAME}.${MOD_LNKLSTEXT}

# definitions for binary folder target outputs
ifeq (${TARGET_POSTBUILD},)
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

ASMDEFS:=$(strip ${TC_ASMDEFS} ${PROJ_ASMDEFS} ${OS_ASMDEFS} ${EXT_ASMDEFS})

ASMINCDIRS:=$(foreach inc,$(strip ${TC_ASMINCDIRS} ${PROJ_ASMINCDIRS} ${OS_ASMINCDIRS} ${EXT_ASMINCDIRS}),-I$(call MK_TC_PATH,${SDK_ROOTDIR}/${inc}))

#####################################
# compiler section
#####################################
COPTS:=$(strip ${TC_COPTS} ${TC_DBG_COPTS} ${CORE_COPTS} ${PROJ_COPTS} ${EXT_COPTS})

CPPOPTS:=$(strip ${TC_CPPOPTS} ${TC_DBG_COPTS} ${CORE_COPTS} ${PROJ_COPTS} ${EXT_COPTS})

CDEFS:=$(foreach def,$(strip ${TC_CDEFS} ${PROJ_CDEFS} ${OS_CDEFS} ${EXT_CDEFS}),-D${def})

CINCDIRS:=$(foreach inc,$(strip ${PROJ_CINCDIRS} ${OS_CINCDIRS} ${EXT_CINCDIRS}),-I$(call MK_TC_PATH,${SDK_ROOTDIR}/${inc}))

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

