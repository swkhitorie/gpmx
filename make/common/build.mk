################################################################################
#
# build.mk
#
# Build rules to generate exe file from sources
#
################################################################################

include ${MAKEFILES_ROOTDIR}/common/const.mk

include ${MAKEFILES_ROOTDIR}/common/dbg_openocd.mk 

include ${MAKEFILES_ROOTDIR}/common/check.mk

include ${MAKEFILES_ROOTDIR}/common/common.mk

################################################################################
#
# Constants section
#
################################################################################

################################################################################
#
# Macros section
#
################################################################################

# Filter a source file from a list.
# As vpath is used to add c/s/asm sources paths to make search folders, it could
# happen that a file with same name is present in more folders. To avoid
# building the wrong one, this macro can filter a sources list and return the
# proper file in the list.
# 1 - source to look for (with or without path)
# 2 - target sources lists
MK_LOOKUPSRC=$(filter %/$(notdir ${1}),${2})

################################################################################
# Variable section
################################################################################

# C thumb sources
BUILD_CSOURCES:=$(foreach src,${PROJ_CSOURCES},${SDK_ROOTDIR}/$(src))

BUILD_CPPSOURCES:=$(foreach src,${PROJ_CPPSOURCES},${SDK_ROOTDIR}/$(src))

# C ARM sources
BUILD_CARMSOURCES:=$(foreach src,${PROJ_CARMSOURCES},${SDK_ROOTDIR}/$(src))

# Assembly sources
BUILD_ASMSOURCES:=$(foreach src,${PROJ_ASMSOURCES},${SDK_ROOTDIR}/$(src))

# All sources
BUILD_SOURCES:=${BUILD_CSOURCES} ${BUILD_CPPSOURCES} ${BUILD_CARMSOURCES} ${BUILD_ASMSOURCES}

# All dependency files
BUILD_DEPENDS:=$(patsubst %.c,%.d,${BUILD_CSOURCES} ${BUILD_CARMSOURCES})

# All sources
PROJ_OBJS:=$(foreach obj,$(basename $(notdir ${BUILD_SOURCES})),${OBJS_FOLDER}/${obj}.o)

#
# dependency file or dependency variable
# 
#C_ADEP := $(call TC_CC_VIA,${TARGET_ROOTDIR}/copts.via)
#ASM_ADEP := $(call TC_ASM_VIA,${TARGET_ROOTDIR}/asmopts.via)
#LINK_ADEP := $(call TC_LINK_VIA, ${TARGET_ROOTDIR}/lopts.via)
C_ADEP := ${COPTS} ${CDEFS} ${CINCDIRS}
CPP_ADEP := ${CPPOPTS} ${CDEFS} ${CINCDIRS}
ASM_ADEP := ${ASMOPTS} ${ASMDEFS} ${ASMINCDIRS}
LINK_ADEP := ${LIBOPTS} $(call MK_TC_LIBDIRS,${LIBDIRS}) $(call MK_TC_LIBS,${LIBS})

# Add sources paths to makefile search paths specific for each pattern
vpath %.c $(dir ${BUILD_CSOURCES} ${BUILD_CARMSOURCES})
vpath %.cpp $(dir ${BUILD_CPPSOURCES})
vpath %.s $(dir ${BUILD_ASMSOURCES})
vpath %.asm $(dir ${BUILD_ASMSOURCES})

##################################
# Targets section - Folder
##################################
${TARGET_ROOTDIR}:
	$(call MK_MKDIR,"$(call MK_PATHTOWIN,$@)")

${OBJS_FOLDER}:
	$(call MK_MKDIR,"$(call MK_PATHTOWIN,$@)")

${TARGET_DEST_ROOTDIR}:
	$(call MK_MKDIR,"$(call MK_PATHTOWIN,$@)")

##################################
# Targets section - Via files
##################################
${ASMOPTS_FILE}: ${CONFIG_FILE} ${LIST_DEFSINC} ${PROJ_MAKEFILE}
	$(call MK_ECHO,Generating assembler via file for ${PROJ_TC} builder)
	$(call MK_RMFILE,${ASMOPTS_FILE})
	$(call MK_APPEND,${ASMOPTS},"${ASMOPTS_FILE}")
	$(call MK_APPEND,${ASMDEFS},"${ASMOPTS_FILE}")
	$(call MK_APPEND,${ASMINCDIRS},"${ASMOPTS_FILE}")

${COPTS_FILE}: ${CONFIG_FILE} ${LIST_DEFSINC} ${PROJ_MAKEFILE}
	$(call MK_APPGET,Generating C compiler via file for ${PROJ_TC} builder)
	$(call MK_RMFILE,${COPTS_FILE})
	$(call MK_APPEND,${COPTS},"${COPTS_FILE}")
	$(call MK_APPEND,${CDEFS},"${COPTS_FILE}")
	$(call MK_APPEND,${CINCDIRS},"${COPTS_FILE}")

${CPPOPTS_FILE}: ${CONFIG_FILE} ${LIST_DEFSINC} ${PROJ_MAKEFILE}
	$(call MK_APPGET,Generating Cpp compiler via file for ${PROJ_TC} builder)
	$(call MK_RMFILE,${CPPOPTS_FILE})
	$(call MK_APPEND,${CPPOPTS},"${CPPOPTS_FILE}")
	$(call MK_APPEND,${CDEFS},"${CPPOPTS_FILE}")
	$(call MK_APPEND,${CINCDIRS},"${CPPOPTS_FILE}")

${LOPTS_FILE}: ${CONFIG_FILE} ${LIST_DEFSINC} ${PROJ_MAKEFILE}
	$(call MK_ECHO,Generating linker via file for ${PROJ_TC} builder)
	$(call MK_RMFILE,${LOPTS_FILE})
	$(call MK_APPEND,${LIBOPTS},"${LOPTS_FILE}")
	$(call MK_APPEND,$(call MK_TC_LIBDIRS,${LIBDIRS}),"${LOPTS_FILE}")
	$(call MK_APPEND,$(call MK_TC_LIBS,${LIBS}),"${LOPTS_FILE}")

##################################
# Targets section - Compiler implicit targets
##################################
#${OBJS_FOLDER}/%.o: %.c ${OBJS_FOLDER}/%.d
${OBJS_FOLDER}/%.o: %.c ${COPTS_FILE}
	$(eval SRCFILE:=$(call MK_LOOKUPSRC,$^,${BUILD_CSOURCES} ${BUILD_CARMSOURCES}))
	$(eval SRCMODE:=$(if $(findstring $<,${BUILD_CARMSOURCES}),${TC_TARGETARM},${TC_TARGETTHUMB}))
	$(call MK_ECHO,Compiling ${SRCFILE} )
	@${TC_CC} ${C_ADEP} ${SRCMODE} -o $@ ${SRCFILE}

${OBJS_FOLDER}/%.o: %.cpp ${CPPOPTS_FILE}
	$(eval SRCFILE:=$(call MK_LOOKUPSRC,$^,${BUILD_CPPSOURCES}))
	$(eval SRCMODE:=$(if $(findstring $<,${BUILD_CARMSOURCES}),${TC_TARGETARM},${TC_TARGETTHUMB}))
	$(call MK_ECHO,Compiling ${SRCFILE})
	@${TC_CPP} ${CPP_ADEP} ${SRCMODE} -o $@ ${SRCFILE}

# asm file suffix for arm is (.s), not (.asm)
${OBJS_FOLDER}/%.o: %.s ${ASMOPTS_FILE}
	$(eval SRCFILE:=$(call MK_LOOKUPSRC,$^,${BUILD_ASMSOURCES}))
	$(call MK_ECHO,Assembling ${SRCFILE})
	@${TC_ASM} ${ASM_ADEP} -o $@ ${SRCFILE}

# ${OBJS_FOLDER}/%.o: %.asm ${ASMOPTS_FILE}
# 	$(eval SRCFILE:=$(call MK_LOOKUPSRC,$^,${BUILD_ASMSOURCES}))
# 	$(call MK_ECHO,Assembling gae ${SRCFILE})
# 	@${TC_ASM} ${ASM_ADEP} -o $@ ${SRCFILE}

# .PRECIOUS: ${OBJS_FOLDER}/%.d
# ${OBJS_FOLDER}/%.d: %.c
# 	$(eval SRCFILE:=$(call MK_LOOKUPSRC,$^,${BUILD_CSOURCES} ${BUILD_CARMSOURCES}))
# 	$(call MK_ECHO,Building dependency for ${SRCFILE})
# 	@${TC_MAKEDEP} $(call TC_CC_VIA,${TARGET_ROOTDIR}/copts.via) \
# 		-o $(patsubst %.d,%.o,$@) ${SRCFILE} > $@
# 		-o $(patsubst %.d,%.o,$@) ${SRCFILE} > $@
# 		${SRCFILE} > $@_tmp ; \
# 	  $(MK_BUSYBOX) awk "{ print \"pippo/\"$1\" \"$2}" $@_tmp)
# 	$(call MK_RMFILE,$@_tmp)

-include ${OBJS_FOLDER}/%.d

#
# Executable target
#
${TARGET_SRC_FILENAME_EXE}: ${LOPTS_FILE} ${OBJS_FOLDER} ${PROJ_OBJS} ${LOPTS_FILE}
	$(call MK_ECHO,Linking to $(subst ${SDK_ROOTDIR}/,,$@))
	@${TC_LINK} -o $@ ${PROJ_OBJS} \
	${LINK_ADEP} \
	$(call TC_LINK_ENTRY,${PROJ_ENTRY_POINT}) \
    $(call TC_LINK_SCF,${SCF_FILE_NAME}) \
	$(call TC_LINK_LIST,${TARGET_SRC_FILENAME_LIST})
	@${TC_SIZE} $@

#
# Binary target
#
${TARGET_DEST_FILENAME_BIN}: ${TARGET_DEST_ROOTDIR} ${TARGET_SRC_FILENAME_EXE} ${TARGET_EXTRA_RULES}
	$(call MK_ECHO,Copying to bin folder)
	$(call MK_RMFILE,"${TARGET_DEST_FILENAME_EXE}" "${TARGET_DEST_FILENAME_LIST}" "${TARGET_DEST_FILENAME_BIN}")
	$(call MK_CP,"${TARGET_SRC_FILENAME_EXE}","${TARGET_DEST_FILENAME_EXE}")
	$(call MK_CP,"${TARGET_SRC_FILENAME_LIST}","${TARGET_DEST_FILENAME_LIST}")
	$(call MK_ECHO,Generating standard binary image)
	$(call MK_TC_GENBIN,"${TARGET_SRC_FILENAME_EXE}","${TARGET_DEST_FILENAME_BIN}")

# TARGET_BIN_MDK := ${SDK_ROOTDIR}/build/mdk/gpilot-v2/output/gpilot-v2.bin
#
# internal secondary targets
#
.SECONDARY: generatemk build postbuild clean distclean program debug

FORCE:

# pre build target
prebuild: ${TARGET_ROOTDIR} ${ASMOPTS_FILE} ${COPTS_FILE} ${LOPTS_FILE}

# build makefile target
build: ${TARGET_SRC_FILENAME_EXE}

# apply default configuration target
postbuild: ${TARGET_POSTBUILD} #${OZONE_PROJ}

#
# exported targets
#
clean:
	$(call MK_ECHO,Executing clean on target ${PROJ_NAME})
	$(call MK_RMFILE,${PROJ_OBJS})
	$(call MK_RMFILE,${TARGET_SRC_FILENAME_EXE})
	$(call MK_RMFILE,${TARGET_SRC_FILENAME_LIST})

distclean:
	$(call MK_ECHO,Executing distclean on target ${PROJ_NAME})
	$(call MK_RMDIR,${TARGET_ROOTDIR})

program: ${TARGET_POSTBUILD}
	$(call MK_ECHO,Programming image)
	$(call MK_FWPROGRAM,${TC_OPENOCD_PATH},${TC_OPENOCD_DEBUG_CFG},${TC_OPENOCD_CHIP_CFG},${TARGET_DEST_FILENAME_BIN},${PROJ_OPENOCD_LOAD_ADDR})

upgrade: ${TARGET_POSTBUILD}
ifeq (${MOD_FWUPG},)
	$(call MK_ECHO,Upgrading image)
	$(call MK_FWUPGRADE,${TARGET_DEST_FILENAME_UPG},${PROJ_BOARD_COM},${PROJ_BOARD_COMSPEED})
else
	$(call MK_ECHO,Upgrading off for this target)
endif

debug: ${OZONE_PROJ}
	$(call MK_ECHO,Starting Ozone debugger)
	${DBG_OZONE_EXE} ${OZONE_PROJ}

rebuild:
	${MAKE} clean
	${MAKE} all

regenerate:
	${MAKE} distclean
	${MAKE} all
