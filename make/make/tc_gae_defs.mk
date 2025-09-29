#
# compiler defs for gcc arm toolchain
#
include ${MAKEFILES_ROOTDIR}/make/macros.mk

TC_GCC_PREFIX = arm-none-eabi-

#
# GCC ARM Compiler related section
#
ifneq ($(OS),Linux)
TC_PATH_BIN:=$(call MK_SHORTNAME,"${TC_PATH_INST_GCC}/bin")
TC_PATH_INC:=$(call MK_SHORTNAME,"${TC_PATH_INST_GCC}/arm-none-eabi/include")
TC_PATH_LIB:=$(call MK_SHORTNAME,"${TC_PATH_INST_GCC}/arm-none-eabi/lib")
else
TC_PATH_BIN:=${TC_PATH_INST_GCC}/bin
TC_PATH_INC:=${TC_PATH_INST_GCC}/arm-none-eabi/include
TC_PATH_LIB:=${TC_PATH_INST_GCC}/arm-none-eabi/lib
endif

########################################
# toolchain executables                #
########################################
ifneq ($(OS),Linux)
TC_MAKEDEP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)gcc.exe -MM)
TC_CC:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)gcc.exe)
TC_CPP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)g++.exe)
TC_ASM:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)as.exe)
TC_LINK:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)gcc.exe)
TC_AR:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)ar.exe)
TC_GENSCF:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)cpp.exe)
TC_GENBIN:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)objcopy.exe)
TC_GENHEX:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)objcopy.exe)
TC_SIZE:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)size.exe)
TC_DUMP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)objdump.exe)
else
TC_MAKEDEP:=${TC_PATH_BIN}/$(TC_GCC_PREFIX)gcc -MM
TC_CC:=${TC_PATH_BIN}/$(TC_GCC_PREFIX)gcc
TC_CPP:=${TC_PATH_BIN}/$(TC_GCC_PREFIX)g++
TC_ASM:=${TC_PATH_BIN}/$(TC_GCC_PREFIX)as
TC_LINK:=${TC_PATH_BIN}/$(TC_GCC_PREFIX)gcc
TC_AR:=${TC_PATH_BIN}/$(TC_GCC_PREFIX)ar
TC_GENSCF:=${TC_PATH_BIN}/$(TC_GCC_PREFIX)cpp
TC_GENBIN:=${TC_PATH_BIN}/$(TC_GCC_PREFIX)objcopy
TC_GENHEX:=${TC_PATH_BIN}/$(TC_GCC_PREFIX)objcopy
TC_SIZE:=${TC_PATH_BIN}/$(TC_GCC_PREFIX)size
TC_DUMP:=${TC_PATH_BIN}/$(TC_GCC_PREFIX)objdump
endif

########################################
# toolchain switches macros            #
########################################
TC_ASM_VIA=@${1}
TC_CC_VIA=@${1}
TC_LINK_VIA=@${1}
TC_LINK_LIBUSE=
TC_LINK_ENTRY=-e ${1}
TC_LINK_SCF=-T ${1}
TC_LINK_LIST=-Wl,-Map=${1}

########################################
# constants                            #
########################################

# toolchain identifiers
TC_NAME:=gae
TC_SUFFIX:=elf

# Assembly opcodes type
TC_ASMTYPE:=gnu
TC_ASMEXT:=asm

TC_TARGETARM:=-marm
TC_TARGETTHUMB:=-mthumb

########################################
# compile/assembly/link options        #
########################################
TC_SOURCEOPTS+= -c
TC_SOURCEOPTS+= -g3
TC_SOURCEOPTS+= -gdwarf-4
TC_SOURCEOPTS+= -fstack-usage
TC_SOURCEOPTS+= -fdata-sections
TC_SOURCEOPTS+= -ffunction-sections
TC_SOURCEOPTS+= -fno-common
ifeq (${CONFIG_COMPILE_OPTIMIZE},)
TC_SOURCEOPTS+= -O1
else
TC_SOURCEOPTS+= ${CONFIG_COMPILE_OPTIMIZE}
endif
ifeq (${CONFIG_LIB_USE_NANO},y)
TC_LIB_SELECT+= --specs=nano.specs
else
TC_LIB_SELECT+= --specs=nosys.specs
endif

# Assembly compiler options
TC_ASMOPTS:=\
  --gdwarf-2                  
  
TC_DBG_ASMOPTS:=
  # -g                    \
  # -gdwarf-2

# Assembly compiler defines
TC_ASMDEFS:=

# C compiler options
TC_COPTS:=\
  ${TC_SOURCEOPTS}     \
  ${TC_LIB_SELECT}  
ifneq (${CONFIG_C_STANDARD},)
TC_COPTS += -std=${CONFIG_C_STANDARD}
endif

TC_CPPOPTS:=\
  ${TC_SOURCEOPTS}     \
  ${TC_LIB_SELECT}     \
  -fno-exceptions      \
  -fno-rtti            \
  -fno-use-cxa-atexit
ifneq (${CONFIG_CXX_STANDARD},)
TC_CPPOPTS += -std=${CONFIG_CXX_STANDARD}
endif

TC_DBG_COPTS:=
  # -g          \
  # -gdwarf-2

# C compiler defines
TC_CDEFS:=

#  debug:  -Wl,--print-gc-sections   
# Linker options
TC_LIBOPTS:=\
  -W                         \
  -Wl,--gc-sections --static \
  -Wl,--sort-section=alignment \
  -Wl,--sort-common \
  -Wl,--print-memory-usage   \
  -Wl,--cref                 \
  --data-sections            \
  -lc                        \
  -lm                        \
  ${TC_LIB_SELECT}

ifeq (${CONFIG_LINK_PRINTF_FLOAT},y)
TC_LIBOPTS += -u _printf_float 
endif

ifeq (${CONFIG_LINK_SCANF_FLOAT},y)
TC_LIBOPTS += -u _scanf_float 
endif

# Scatter file extension
TC_SCFEXT:=ld

# Linker libraries
TC_LIBNAMES:=\
  c                     \
  m                     \
  stdc++

TC_LIBPREFIX:=lib
TC_LIBSUFFIX:=gcc4x
TC_LIBEXT:=a

# Archiver options
TC_AROPTS:=\
  -r

#
# toolchain macros
#

# convert PATH to toolchain friendly path
MK_TC_PATH=$(call MK_PATHTOUNX,${1})

# command to generate list of linker directory search paths
MK_TC_LIBDIRS=$(if ${1},$(addprefix -L,${1}))

# command to generate list of linker directory search paths
MK_TC_LIBS=$(if ${1},$(addprefix -l,${1}))

# command to generate scatter file
# 1 - template file
# 2 - output file
# 3 (opt) - armcc options
MK_TC_GENSCF=$(if $(and ${1},${2},${3}),@${TC_GENSCF} -P -E ${1} -o ${2} ${3})

# command to generate binary file
# 1 - input file
# 2 - output file
MK_TC_GENBIN=$(if $(and ${1},${2}),@${TC_GENBIN} -O binary ${1} ${2})

# command to generate hex file
# 1 - input file
# 2 - output file
MK_TC_GENHEX=$(if $(and ${1},${2}),@${TC_GENHEX} -O ihex ${1} ${2})

# command to disassembly output file
# 1 - input file
# 2 - output file
MK_TC_DISASSEMBLY=$(if $(and ${1},${2}), @${TC_DUMP} -h -S ${1} > "${2}")

# more accurate compilation result analysis cmd
# 1 - input file
MK_TC_COMPILE_ANALYZE=$(if $(and ${1},$(wildcard MK_COMPILE_ANALYZE_GCC)), @${MK_PYTHON} ${MK_COMPILE_ANALYZE_GCC} ${1})
