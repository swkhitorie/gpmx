#
# compiler defs for gcc arm toolchain
#
include ${MAKEFILES_ROOTDIR}/common/macros.mk

#
# This is the default installation path for gcc arm. Change it if different
# 
# TC_PATH_INST_GCC:=D:\__dev_envir\__env\arm_gcc\gcc-arm-none-eabi
TC_GCC_PREFIX = arm-none-eabi-

#
# GCC ARM Compiler related section
#
TC_PATH_BIN:=$(call MK_SHORTNAME,"${TC_PATH_INST_GCC}/bin")
TC_PATH_INC:=$(call MK_SHORTNAME,"${TC_PATH_INST_GCC}/arm-none-eabi/include")
TC_PATH_LIB:=$(call MK_SHORTNAME,"${TC_PATH_INST_GCC}/arm-none-eabi/lib")
#
# toolchain executables
#
TC_MAKEDEP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)gcc.exe -MM)
TC_CC:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)gcc.exe)
TC_CPP:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)g++.exe)
TC_ASM:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)as.exe)
TC_LINK:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)gcc.exe)
TC_AR:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)ar.exe)
TC_GENBIN:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)objcopy.exe)
TC_SIZE:=$(call MK_PATHTOUNX,${TC_PATH_BIN}/$(TC_GCC_PREFIX)size.exe)

#
# toolchain switches macros
#
TC_ASM_VIA=@${1}
TC_CC_VIA=@${1}
TC_LINK_VIA=@${1}
TC_LINK_LIBUSE=
TC_LINK_ENTRY=-e ${1}
TC_LINK_SCF=-T ${1}
TC_LINK_LIST=-Wl,-Map=${1}

#
# constants
#

# toolchain identifiers
TC_NAME:=gae
TC_SUFFIX:=elf

# Assembly opcodes type
TC_ASMTYPE:=gnu
TC_ASMEXT:=asm

TC_TARGETARM:=-marm
TC_TARGETTHUMB:=-mthumb

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
  -c                    \
  -ffunction-sections   \
  -fdata-sections       \
  -O1                   \
  -gdwarf-2             \
  -MD                   \
  -w -Wno-empty-body       \
  -w -Wno-macro-redefined  \
  -w -Wno-invalid-source-encoding \
  -w -Wno-writable-strings

TC_CPPOPTS:=\
  -c                    \
  -ffunction-sections   \
  -fdata-sections       \
  -O1                   \
  -gdwarf-2             \
  -MD                   \
  -w -Wno-empty-body       \
  -w -Wno-macro-redefined  \
  -w -Wno-invalid-source-encoding \
  -w -Wno-writable-strings

TC_DBG_COPTS:=
  # -g          \
  # -gdwarf-2

# C compiler defines
TC_CDEFS:=

# Linker options
TC_LIBOPTS:=\
  -W                        \
  -W --no-cond-mismatch     \
  -Wl,--gc-sections         \
  --data-sections           \
  --specs=nano.specs        \
  -u _printf_float          \
  -u _scanf_float           \
  -lc                       \
  -lm                       \
  -lnosys                   
  #-nostartfiles             

# Scatter file extension
TC_SCFEXT:=ld

# Linker libraries
TC_LIBNAMES:=\
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
