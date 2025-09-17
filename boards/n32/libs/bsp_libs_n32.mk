
BSP_LIBS_PATH=boards/n32/libs
BSP_LIBS_PATHSDK=${SDK_ROOTDIR}/boards/n32/libs

#########################################################################
# LLD Drivers
#########################################################################
ifeq (${BSP_LIBCONFIG_N32_LLDRV},y)

ifeq (${BSP_LIBCONFIG_N32_SERIES},BSP_N32G430)
include ${BSP_LIBS_PATHSDK}/g430_driver/Make.defs
TMPDRV_CINCDIRS = ${addprefix ${BSP_LIBS_PATH}/g430_driver/,${LIBDRV_CINCDIRS}}
TMPDRV_CSRCS = ${addprefix ${BSP_LIBS_PATH}/g430_driver/,${LIBDRV_CSRCS}}
else
endif

endif

#########################################################################
# Chips SDK
#########################################################################
ifeq (${BSP_LIBCONFIG_N32_SERIES},BSP_N32G430)

include ${BSP_LIBS_PATHSDK}/n32g430_stdlib/Make.defs
TMPSTD_CINCDIRS = ${addprefix ${BSP_LIBS_PATH}/n32g430_stdlib/,${LIBSTD_CINCDIRS}}
TMPSTD_CSRCS = ${addprefix ${BSP_LIBS_PATH}/n32g430_stdlib/,${LIBSTD_CSRCS}}

endif

LIB_CINCDIRS += ${BSP_LIBS_PATH}/cmsis
LIB_CINCDIRS += ${TMPDRV_CINCDIRS} ${TMPSTD_CINCDIRS}
LIB_CSRCS += ${TMPDRV_CSRCS} ${TMPSTD_CSRCS}

