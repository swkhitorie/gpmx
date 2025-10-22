#
# compiler defs for ARM Compiler v5 armcc toolchain
#
include ${MAKEFILES_ROOTDIR}/make/macros.mk

#
# ARM Compiler related section
#
ifneq ($(OS),Linux)
TC_PATH_BIN:=$(call MK_SHORTNAME,"${TC_PATH_INST_ARMCC}\bin")
TC_PATH_INC:=$(call MK_SHORTNAME,"${TC_PATH_INST_ARMCC}\include")
TC_PATH_LIB:=$(call MK_SHORTNAME,"${TC_PATH_INST_ARMCC}\lib")
else
TC_PATH_BIN:=${TC_PATH_INST_ARMCC}/bin
TC_PATH_INC:=${TC_PATH_INST_ARMCC}/include
TC_PATH_LIB:=${TC_PATH_INST_ARMCC}/lib
endif

########################################
# toolchain executables                #
########################################
ifneq ($(OS),Linux)
TC_MAKEDEP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armcc.exe -M --no_depend_system_headers)
TC_CC:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armcc.exe)
TC_CPP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armcc.exe)
TC_ASM:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armasm.exe)
TC_LINK:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armlink.exe)
TC_AR:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armar.exe)
TC_GENSCF:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armcc.exe)
TC_GENBIN:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/fromelf.exe)
TC_GENHEX:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/fromelf.exe)
TC_SIZE:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/fromelf.exe -z)
TC_DUMP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/fromelf.exe)
else
TC_MAKEDEP:=${TC_PATH_BIN}/armcc -M --no_depend_system_headers
TC_CC:=${TC_PATH_BIN}/armcc
TC_CPP:=${TC_PATH_BIN}/armcc
TC_ASM:=${TC_PATH_BIN}/armasm
TC_LINK:=${TC_PATH_BIN}/armlink
TC_AR:=${TC_PATH_BIN}/armar
TC_GENSCF:=${TC_PATH_BIN}/armcc
TC_GENBIN:=${TC_PATH_BIN}/fromelf
TC_GENHEX:=${TC_PATH_BIN}/fromelf
TC_SIZE:=${TC_PATH_BIN}/fromelf -z
TC_DUMP:=${TC_PATH_BIN}/fromelf
endif

########################################
# toolchain switches macros            #
########################################
TC_ASM_VIA=--via ${1}
TC_CC_VIA=--via ${1}
TC_LINK_VIA=--via ${1}
TC_LINK_ENTRY=--entry=${1}
TC_LINK_SCF=--scatter=${1}
TC_LINK_LIST=--list=${1}

########################################
# constants                            #
########################################

# toolchain identifiers
TC_NAME:=armcc
TC_SUFFIX:=axf

# Assembly opcodes type
TC_ASMTYPE:=arm
TC_ASMEXT:=s

TC_TARGETTHUMB:=--thumb
TC_TARGETARM:=--arm

########################################
# compile/assembly/link options        #
########################################
TC_SOURCEOPTS+= -c
TC_SOURCEOPTS+= -g
TC_SOURCEOPTS+= -W
TC_SOURCEOPTS+= --split_sections
ifeq (${CONFIG_COMPILE_OPTIMIZE},)
TC_SOURCEOPTS+= -O1
else
TC_SOURCEOPTS+= ${CONFIG_COMPILE_OPTIMIZE}
endif
ifeq (${CONFIG_LIB_USE_NANO},y)
TC_CDEFS+= __MICROLIB
TC_LIB_SELECT+= --library_type=microlib
endif

TC_ASMOPTS:=\
  -g --16                    \
  --diag_style=gnu
ifeq (${CONFIG_LIB_USE_NANO},y)
TC_ASMOPTS+= --pd "__MICROLIB SETA 1"
endif

# Assembly compiler defines
TC_ASMDEFS:=

# C compiler options
TC_COPTS:= --c99 \
  ${TC_SOURCEOPTS}           

TC_CPPOPTS:= --cpp   \
  ${TC_SOURCEOPTS}

# Linker options
TC_LIBOPTS:=\
  --libpath=$(call MK_PATHTOUNX,${TC_PATH_LIB}) \
  --strict                                \
  --summary_stderr                        \
  --map                                   \
  --xref                                  \
  --callgraph                             \
  --symbols                               \
  --verbose                               \
  --info summarysizes                     \
  --info sizes                            \
  --info totals                           \
  --info unused                           \
  --info veneers                          \
  --info stack                            \
  ${TC_LIB_SELECT}

# Scatter file extension
TC_SCFEXT:=sct

#
# toolchain macros
#

# convert PATH to toolchain friendly path
ifneq ($(OS),Linux)
MK_TC_PATH=$(call MK_PATHTOWIN,${1})
else
MK_TC_PATH=$(call MK_PATHTOUNX,${1})
endif

# command to generate list of linker directory search paths
MK_TC_LIBDIRS=$(if ${1},--userlibpath $(call MK_SPCTOCOM,${1}))

# command to generate list of linker directory search paths
MK_TC_LIBS=$(if ${1},$(addsuffix .${TC_LIBEXT},${1}))

# command to generate scatter file
# 1 - template file
# 2 - output file
# 3 (opt) - armcc options
MK_TC_GENSCF=$(if $(and ${1},${2},${3}),@${TC_GENSCF} -E ${1} -o ${2} ${3})

# command to generate binary file
# 1 - input file
# 2 - output file
MK_TC_GENBIN=$(if $(and ${1},${2}),@${TC_GENBIN} -c --bin -o ${2} ${1})

# command to generate hex file
# 1 - input file
# 2 - output file
MK_TC_GENHEX=$(if $(and ${1},${2}),@${TC_GENHEX} -c --i32 -o ${2} ${1})

# command to disassembly output file
# 1 - input file
# 2 - output file
MK_TC_DISASSEMBLY=$(if $(and ${1},${2}), @${TC_DUMP} -c -a ${1} -o ${2})

# more accurate compilation result analysis cmd
# 1 - input file
# MK_TC_COMPILE_ANALYZE=$(if $(and ${1},$(wildcard ${MK_COMPILE_ANALYZE_ARM})), $(if (@python3 ${MK_COMPILE_ANALYZE_ARM} ${1}), @python ${MK_COMPILE_ANALYZE_ARM} ${1}))
MK_TC_COMPILE_ANALYZE=$(if $(and ${1},$(wildcard ${MK_COMPILE_ANALYZE_ARM})), @python3 ${MK_COMPILE_ANALYZE_ARM} ${1} || python ${MK_COMPILE_ANALYZE_ARM} ${1})
