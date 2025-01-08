

ifeq (${BSP_LIBCONFIG_STM32_LLDRV},y)

include ${BSP_ABSROOTDIR}/stm32/libs/drivers/Make.defs
LIB_DRVCINCDIRS = ${addprefix ${BSP_ROOT}/stm32/libs/drivers/,${LIB_DRVCINCDIRS}}
LIB_DRVCSRCS = ${addprefix ${BSP_ROOT}/stm32/libs/drivers/,${LIB_DRVCSRCS}}

endif

ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_F1)

include ${BSP_ABSROOTDIR}/stm32/libs/stm32f1xx_hal/Make.defs
LIB_CINCDIRS = ${addprefix ${BSP_ROOT}/stm32/libs/stm32f1xx_hal/,${LIB_DRVCINCDIRS}}
LIB_CSRCS = ${addprefix ${BSP_ROOT}/stm32/libs/stm32f1xx_hal/,${LIB_DRVCSRCS}}

else ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_F4)

include ${BSP_ABSROOTDIR}/stm32/libs/stm32f4xx_hal/Make.defs
LIB_CINCDIRS = ${addprefix ${BSP_ROOT}/stm32/libs/stm32f4xx_hal/,${LIB_DRVCINCDIRS}}
LIB_CSRCS = ${addprefix ${BSP_ROOT}/stm32/libs/stm32f4xx_hal/,${LIB_DRVCSRCS}}

else ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_H7)

include ${BSP_ABSROOTDIR}/stm32/libs/stm32h7xx_hal/Make.defs
LIB_CINCDIRS = ${addprefix ${BSP_ROOT}/stm32/libs/stm32h7xx_hal/,${LIB_DRVCINCDIRS}}
LIB_CSRCS = ${addprefix ${BSP_ROOT}/stm32/libs/stm32h7xx_hal/,${LIB_DRVCSRCS}}
endif

LIB_CINCDIRS += cmsis
