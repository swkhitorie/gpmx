BSP_LIBS_PATH=boards/stm32/libs
BSP_LIBS_PATHSDK=${SDK_ROOTDIR}/boards/stm32/libs

#########################################################################
# LLD Drivers
#########################################################################
ifeq (${BSP_LIBCONFIG_STM32_LLDRV},y)
include ${BSP_LIBS_PATHSDK}/drivers/Make.defs
TMPDRV_CINCDIRS = ${addprefix ${BSP_LIBS_PATH}/drivers/,${LIBDRV_CINCDIRS}}
TMPDRV_CSRCS = ${addprefix ${BSP_LIBS_PATH}/drivers/,${LIBDRV_CSRCS}}
endif

#########################################################################
# Chips SDK
#########################################################################
ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_G0)

include ${BSP_LIBS_PATHSDK}/stm32g0xx_hal/Make.defs
TMPHAL_CINCDIRS = ${addprefix ${BSP_LIBS_PATH}/stm32g0xx_hal/,${LIBHAL_CINCDIRS}}
TMPHAL_CSRCS = ${addprefix ${BSP_LIBS_PATH}/stm32g0xx_hal/,${LIBHAL_CSRCS}}

else ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_F1)

include ${BSP_LIBS_PATHSDK}/stm32f1xx_hal/Make.defs
TMPHAL_CINCDIRS = ${addprefix ${BSP_LIBS_PATH}/stm32f1xx_hal/,${LIBHAL_CINCDIRS}}
TMPHAL_CSRCS = ${addprefix ${BSP_LIBS_PATH}/stm32f1xx_hal/,${LIBHAL_CSRCS}}

else ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_F4)

include ${BSP_LIBS_PATHSDK}/stm32f4xx_hal/Make.defs
TMPHAL_CINCDIRS = ${addprefix ${BSP_LIBS_PATH}/stm32f4xx_hal/,${LIBHAL_CINCDIRS}}
TMPHAL_CSRCS = ${addprefix ${BSP_LIBS_PATH}/stm32f4xx_hal/,${LIBHAL_CSRCS}}

else ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_H7)

include ${BSP_LIBS_PATHSDK}/stm32h7xx_hal/Make.defs
TMPHAL_CINCDIRS = ${addprefix ${BSP_LIBS_PATH}/stm32h7xx_hal/,${LIBHAL_CINCDIRS}}
TMPHAL_CSRCS = ${addprefix ${BSP_LIBS_PATH}/stm32h7xx_hal/,${LIBHAL_CSRCS}}

else ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_WL)

include ${BSP_LIBS_PATHSDK}/stm32wlxx_hal/Make.defs
TMPHAL_CINCDIRS = ${addprefix ${BSP_LIBS_PATH}/stm32wlxx_hal/,${LIBHAL_CINCDIRS}}
TMPHAL_CSRCS = ${addprefix ${BSP_LIBS_PATH}/stm32wlxx_hal/,${LIBHAL_CSRCS}}

endif

LIB_CINCDIRS += ${BSP_LIBS_PATH}/cmsis
LIB_CINCDIRS += ${TMPDRV_CINCDIRS} ${TMPHAL_CINCDIRS}
LIB_CSRCS += ${TMPDRV_CSRCS} ${TMPHAL_CSRCS}
