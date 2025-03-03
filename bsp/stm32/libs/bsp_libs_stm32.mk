

ifeq (${BSP_LIBCONFIG_STM32_LLDRV},y)
include ${SDK_ROOTDIR}/bsp/stm32/libs/drivers/Make.defs
TMPDRV_CINCDIRS = ${addprefix bsp/stm32/libs/drivers/,${LIBDRV_CINCDIRS}}
TMPDRV_CSRCS = ${addprefix bsp/stm32/libs/drivers/,${LIBDRV_CSRCS}}
endif

ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_G0)

include ${SDK_ROOTDIR}/bsp/stm32/libs/stm32g0xx_hal/Make.defs
TMPHAL_CINCDIRS = ${addprefix bsp/stm32/libs/stm32g0xx_hal/,${LIBHAL_CINCDIRS}}
TMPHAL_CSRCS = ${addprefix bsp/stm32/libs/stm32g0xx_hal/,${LIBHAL_CSRCS}}

else ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_F1)

include ${SDK_ROOTDIR}/bsp/stm32/libs/stm32f1xx_hal/Make.defs
TMPHAL_CINCDIRS = ${addprefix bsp/stm32/libs/stm32f1xx_hal/,${LIBHAL_CINCDIRS}}
TMPHAL_CSRCS = ${addprefix bsp/stm32/libs/stm32f1xx_hal/,${LIBHAL_CSRCS}}

else ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_F4)

include ${SDK_ROOTDIR}/bsp/stm32/libs/stm32f4xx_hal/Make.defs
TMPHAL_CINCDIRS = ${addprefix bsp/stm32/libs/stm32f4xx_hal/,${LIBHAL_CINCDIRS}}
TMPHAL_CSRCS = ${addprefix bsp/stm32/libs/stm32f4xx_hal/,${LIBHAL_CSRCS}}

else ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_H7)

include ${SDK_ROOTDIR}/bsp/stm32/libs/stm32h7xx_hal/Make.defs
TMPHAL_CINCDIRS = ${addprefix bsp/stm32/libs/stm32h7xx_hal/,${LIBHAL_CINCDIRS}}
TMPHAL_CSRCS = ${addprefix bsp/stm32/libs/stm32h7xx_hal/,${LIBHAL_CSRCS}}
endif

LIB_CINCDIRS += bsp/stm32/libs/cmsis
LIB_CINCDIRS += ${TMPDRV_CINCDIRS} ${TMPHAL_CINCDIRS}
LIB_CSRCS += ${TMPDRV_CSRCS} ${TMPHAL_CSRCS}
