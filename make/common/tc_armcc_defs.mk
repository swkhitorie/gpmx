#
# compiler defs for ARM Compiler v5 armcc toolchain
#
include ${MAKEFILES_ROOTDIR}/common/macros.mk

#
# This is the default installation path for Keil/MDK. Change it if different
# 
# TC_PATH_INST_ARMCC := D:\__dev_envir\__env\mdk5\CORE\ARM\ARMCC

#
# ARM Compiler related section
#
TC_PATH_BIN:=$(call MK_SHORTNAME,"${TC_PATH_INST_ARMCC}\bin")
TC_PATH_INC:=$(call MK_SHORTNAME,"${TC_PATH_INST_ARMCC}\include")
TC_PATH_LIB:=$(call MK_SHORTNAME,"${TC_PATH_INST_ARMCC}\lib")
#
# toolchain executables
#
TC_MAKEDEP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armcc.exe -M --no_depend_system_headers)
TC_CC:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armcc.exe)
TC_CPP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armcc.exe)
TC_ASM:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armasm.exe)
TC_LINK:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armlink.exe)
TC_AR:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/armar.exe)
TC_GENBIN:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/fromelf.exe)

#
# toolchain switches macros
#
TC_ASM_VIA=--via ${1}
TC_CC_VIA=--via ${1}
TC_LINK_VIA=--via ${1}
TC_LINK_ENTRY=--entry=${1}
TC_LINK_SCF=--scatter=${1}
TC_LINK_LIST=--list=${1}

#
# constants
#

# toolchain identifiers
TC_NAME:=armcc
TC_SUFFIX:=axf

# Assembly opcodes type
TC_ASMTYPE:=arm
TC_ASMEXT:=s

TC_TARGETTHUMB:=--thumb
TC_TARGETARM:=--arm

# Assembly compiler options
TC_ASMOPTS:=\
  -g                          \
  --diag_style=gnu            \
  --diag_suppress=9931
# --list ".\Obj\*.lst" 
# --xref -o "*.o" 
#  --depend "*.d" 

# Assembly compiler defines
TC_ASMDEFS:=
# C compiler options
TC_COPTS:=\
  --c99                       \
  -c                          \
  -O0                         \
  -g                          \
  --diag_style=gnu            \
  --diag_suppress=1267,9931   \
  --split_sections                   

TC_CPPOPTS:=\
  --cpp                       \
  -c                          \
  -O0                         \
  -g                          \
  --diag_style=gnu            \
  --diag_suppress=1267,9931   \
  --split_sections 

# C compiler defines
TC_CDEFS:=

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
  --info=summarysizes                     \
  --info=sizes                            \
  --info=totals                           \
  --info=unused                           \
  --info=veneers                          \
  --info=stack                            \
  --diag_style=gnu                        \
  --diag_suppress=6314,9931

# Scatter file extension
TC_SCFEXT:=sct

#
# toolchain macros
#

# convert PATH to toolchain friendly path
MK_TC_PATH=$(call MK_PATHTOWIN,${1})

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
