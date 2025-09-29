#
# compiler defs for ARM Compiler v6 clang toolchain
#
include ${MAKEFILES_ROOTDIR}/make/macros.mk

#
# ARM Compiler related section
#
ifneq ($(OS),Linux)
TC_PATH_BIN:=$(call MK_SHORTNAME,"${TC_PATH_INST_ARMCLANG}/bin")
TC_PATH_INC:=$(call MK_SHORTNAME,"${TC_PATH_INST_ARMCLANG}/include")
TC_PATH_LIB:=$(call MK_SHORTNAME,"${TC_PATH_INST_ARMCLANG}/lib")
else
TC_PATH_BIN:=${TC_PATH_INST_ARMCLANG}/bin
TC_PATH_INC:=${TC_PATH_INST_ARMCLANG}/include
TC_PATH_LIB:=${TC_PATH_INST_ARMCLANG}/lib
endif

########################################
# toolchain executables                #
########################################
ifneq ($(OS),Linux)
TC_MAKEDEP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armclang.exe -M)
TC_CC:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armclang.exe)
TC_CPP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armclang.exe)
TC_ASM:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armclang.exe)
TC_LINK:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armlink.exe)
TC_AR:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armar.exe)
TC_GENSCF:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armclang.exe)
TC_GENBIN:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/fromelf.exe)
TC_GENHEX:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/fromelf.exe)
TC_SIZE:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/fromelf.exe -z)
TC_DUMP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/fromelf.exe)
else
TC_MAKEDEP:=${TC_PATH_BIN}/armclang
TC_CC:=${TC_PATH_BIN}/armclang
TC_CPP:=${TC_PATH_BIN}/armclang
TC_ASM:=${TC_PATH_BIN}/armclang
TC_LINK:=${TC_PATH_BIN}/armlink
TC_AR:=${TC_PATH_BIN}/armar
TC_GENSCF:=${TC_PATH_BIN}/armclang
TC_GENBIN:=${TC_PATH_BIN}/fromelf
TC_GENHEX:=${TC_PATH_BIN}/fromelf
TC_SIZE:=${TC_PATH_BIN}/fromelf -z
TC_DUMP:=${TC_PATH_BIN}/fromelf
endif

########################################
# toolchain switches macros            #
########################################
TC_ASM_VIA=@${1}
TC_CC_VIA=@${1}
TC_LINK_VIA=@${1}
TC_LINK_ENTRY=--entry=${1}
TC_LINK_SCF=--scatter=${1}
TC_LINK_LIST=--list=${1}

########################################
# constants                            #
########################################

# toolchain identifiers
TC_NAME:=armclang
TC_SUFFIX:=axf

# Assembly opcodes type
TC_ASMTYPE:=arm
TC_ASMEXT:=s

TC_TARGETTHUMB:=-mthumb
TC_TARGETARM:=-marm

########################################
# compile/assembly/link options        #
########################################
TC_SOURCEOPTS+= -c
TC_SOURCEOPTS+= -gdwarf-4
TC_SOURCEOPTS+= -fno-rtti
TC_SOURCEOPTS+= -funsigned-char
TC_SOURCEOPTS+= -fshort-enums
TC_SOURCEOPTS+= -fshort-wchar
TC_SOURCEOPTS+= -ffunction-sections
ifeq (${CONFIG_COMPILE_OPTIMIZE},)
TC_SOURCEOPTS+= -O1
else
TC_SOURCEOPTS+= ${CONFIG_COMPILE_OPTIMIZE}
endif
ifeq (${CONFIG_LIB_USE_NANO},y)
TC_LIB_SELECT+= --library_type=microlib
else
TC_LIB_SELECT+= --library_type=standardlib
endif

# Assembly compiler options
TC_ASMOPTS:=\
  --target=arm-arm-none-eabi        \
  -masm=auto                        \
  -c                                \
  -gdwarf-4
ifeq (${CONFIG_LIB_USE_NANO},y)
TC_ASMOPTS+= -Wa,armasm,--pd,"__MICROLIB SETA 1"
endif

# Assembly compiler defines
TC_ASMDEFS:=

# debug: why c opts -xc -std=c11 could not work with -xc++ -std=c++11???
# C compiler options
TC_COPTS:=\
  --target=arm-arm-none-eabi        \
  ${TC_SOURCEOPTS}                  \
  -xc
ifneq (${CONFIG_C_STANDARD},)
TC_COPTS += -std=${CONFIG_C_STANDARD}
endif

TC_CPPOPTS:=\
  --target=arm-arm-none-eabi        \
  ${TC_SOURCEOPTS}                  \
  -xc++
ifneq (${CONFIG_CXX_STANDARD},)
TC_CPPOPTS += -std=${CONFIG_CXX_STANDARD}
endif


# C compiler defines
TC_CDEFS:=

# Linker options
TC_LIBOPTS:=\
  ${TC_LIB_SELECT}                        \
  --strict                                \
  --summary_stderr                        \
  --map                                   \
  --xref                                  \
  --callgraph                             \
  --symbols                               \
  --info=summarysizes                     \
  --info=sizes                            \
  --info=totals                           \
  --info=unused                           \
  --info=veneers                          \
  --info=stack                            \
  --diag_style=gnu                        \
  --diag_suppress=6314,9931               \
  --verbose                               \
  --load_addr_map_info

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

# command to generate scatter file, only support 32bits system
# 1 - template file
# 2 - output file
# 3 (opt) - armcc options
MK_TC_GENSCF=$(if $(and ${1},${2},${3}),@${TC_GENSCF} -P -E --target=arm-arm-none-eabi -march=armv7-m -x c ${1} -o ${2} ${3})

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
MK_TC_COMPILE_ANALYZE=$(if $(and ${1},$(wildcard ${MK_COMPILE_ANALYZE_ARM})), @${MK_PYTHON} ${MK_COMPILE_ANALYZE_ARM} ${1})
