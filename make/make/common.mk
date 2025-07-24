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
TOOLS_DIR := ${MAKEFILES_ROOTDIR}/buildtools_win

# folder for common makefiles
MAKEFILES_COMMONDIR := ${MAKEFILES_ROOTDIR}/make

# folder for project files
MAKEFILES_PROJDIR := ${MAKEFILES_ROOTDIR}/${PROJ_NAME}

# folder for target files
ifneq (${PUSER_ROOTDIR},)
TARGET_ROOTDIR := ${PUSER_ROOTDIR}/build/output/${MOD_CORE}/${PROJ_NAME}
else
TARGET_ROOTDIR := ${SDK_ROOTDIR}/build/output/${MOD_CORE}/${PROJ_NAME}
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
ifneq (${PUSER_ROOTDIR},)
TARGET_DEST_ROOTDIR       := ${PUSER_ROOTDIR}/${COMMON_BINDIR}
else
TARGET_DEST_ROOTDIR       := ${SDK_ROOTDIR}/${COMMON_BINDIR}
endif
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
ifneq (${PUSER_ROOTDIR},)
SCF_FILE_NAME := ${PUSER_ROOTDIR}/${SCF_FILE}
else
SCF_FILE_NAME := ${SDK_ROOTDIR}/${SCF_FILE}
endif


# definitions for local folder target outputs
TARGET_SRC_FILENAME_EXE    :=${TARGET_ROOTDIR}/${PROJ_NAME}.${TC_SUFFIX}
TARGET_SRC_FILENAME_LIST   :=${TARGET_ROOTDIR}/${PROJ_NAME}.${MOD_LNKLSTEXT}
TARGET_SRC_FILENAME_DISASM :=${TARGET_ROOTDIR}/${PROJ_NAME}.list

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
ifneq (${PUSER_ROOTDIR},)
CINCDIRS+=$(foreach inc,$(strip ${PUSER_CINCDIRS}),-I$(call MK_TC_PATH,${PUSER_ROOTDIR}/${inc}))
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

