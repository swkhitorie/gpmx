#
# compiler defs for native Linux GCC toolchain
#
include ${MAKEFILES_ROOTDIR}/make/macros.mk

# No cross-prefix
TC_GCC_PREFIX =

#
# Native GCC Compiler related section
#
# Use system gcc, no installation path required.
TC_PATH_BIN :=
TC_PATH_INC :=
TC_PATH_LIB :=

########################################
# toolchain executables                #
########################################
# Use standard system tools (found via PATH)
TC_MAKEDEP := gcc -MM
TC_CC      := gcc
TC_CPP     := g++
TC_ASM     := gcc           # Use gcc to preprocess .S files
TC_LINK    := gcc
TC_AR      := ar
TC_GENSCF  := cpp           # Preprocessor for generating scatter files (not typical)
TC_GENBIN  := objcopy
TC_GENHEX  := objcopy
TC_SIZE    := size
TC_DUMP    := objdump

# On non-Linux hosts (e.g. Windows), one might add .exe suffixes,
# but for native Linux we leave as is.

########################################
# toolchain switches macros            #
########################################
TC_ASM_VIA = @${1}
TC_CC_VIA  = @${1}
TC_LINK_VIA= @${1}
TC_LINK_LIBUSE =
TC_LINK_ENTRY = -e ${1}
TC_LINK_SCF   = -T ${1}
TC_LINK_LIST  = -Wl,-Map=${1}

########################################
# constants                            #
########################################

# toolchain identifiers
TC_NAME   := gcc
TC_SUFFIX := elf          # Keep elf as generic object format (Linux produces ELF)
TC_LIB_SUFFIX:=a

# Assembly opcodes type
TC_ASMTYPE := gnu
TC_ASMEXT  := asm

# No ARM/Thumb switching needed on x86
TC_TARGETARM   :=
TC_TARGETTHUMB :=

########################################
# compile/assembly/link options        #
########################################
TC_SOURCEOPTS += -c
TC_SOURCEOPTS += -g3
TC_SOURCEOPTS += -gdwarf-4
TC_SOURCEOPTS += -fstack-usage
TC_SOURCEOPTS += -fdata-sections
TC_SOURCEOPTS += -ffunction-sections

ifeq (${BUILD_COMPILE_OPTIMIZE},)
TC_SOURCEOPTS += -O1
else
TC_SOURCEOPTS += ${BUILD_COMPILE_OPTIMIZE}
endif

# No nano/sys specs for native Linux
TC_LIB_SELECT :=

# Assembly compiler options
TC_ASMOPTS := \
  -c -gdwarf-2

# Assembly compiler defines
TC_ASMDEFS :=

# C compiler options
TC_COPTS := \
  ${TC_SOURCEOPTS}
ifneq (${BUILD_C_STANDARD},)
TC_COPTS += -std=${BUILD_C_STANDARD}
endif

TC_CPPOPTS := \
  ${TC_SOURCEOPTS}
ifneq (${BUILD_CXX_STANDARD},)
TC_CPPOPTS += -std=${BUILD_CXX_STANDARD}
endif

TC_DBG_COPTS :=

# C compiler defines
TC_CDEFS :=

# Linker options
TC_LIBOPTS := \
  -Wl,--gc-sections          \
  -Wl,--sort-section=alignment \
  -Wl,--sort-common           \
  -Wl,--print-memory-usage    \
  -Wl,--cref                  \
  -lc -lm                     \
  ${EXT_LIBOPTS}

# No _printf_float / _scanf_float hacks needed on native
# (float support is standard in glibc)

# Scatter file extension
TC_SCFEXT := ld

# Linker libraries
TC_LIBNAMES := \
  c    \
  m    \
  stdc++ \
  pthread

TC_LIBPREFIX := lib
TC_LIBSUFFIX :=
TC_LIBEXT    := a

# Archiver options
TC_AROPTS := -r

#
# toolchain macros
#

# convert PATH to toolchain friendly path (unchanged)
MK_TC_PATH = $(call MK_PATHTOUNX,${1})

# command to generate list of linker directory search paths
MK_TC_LIBDIRS = $(if ${1},$(addprefix -L,${1}))

# command to generate list of linker library names
MK_TC_LIBS = $(if ${1},$(addprefix -l,${1}))

# command to generate scatter file
MK_TC_GENSCF = $(if $(and ${1},${2},${3}),@${TC_GENSCF} -P -E ${1} -o ${2} ${3})

# command to generate binary file
MK_TC_GENBIN = $(if $(and ${1},${2}),@${TC_GENBIN} -O binary ${1} ${2})

# command to generate hex file
MK_TC_GENHEX = $(if $(and ${1},${2}),@${TC_GENHEX} -O ihex ${1} ${2})

# command to disassembly output file
MK_TC_DISASSEMBLY = $(if $(and ${1},${2}),@${TC_DUMP} -h -S ${1} > "${2}")
