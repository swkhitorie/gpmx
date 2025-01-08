

ifeq (${BSP_LIBCONFIG_STM32_LLDRV},y)

include drivers/Make.defs
LIB_DRVCINCDIRS = ${addprefix drivers/,${LIB_DRVCINCDIRS}}
LIB_DRVCSRCS = ${addprefix drivers/,${LIB_DRVCSRCS}}

endif

ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_F1)

include stm32f1xx_hal/Make.defs
LIB_CINCDIRS = ${addprefix stm32f1xx_hal/,${LIB_DRVCINCDIRS}}
LIB_CSRCS = ${addprefix stm32f1xx_hal/,${LIB_DRVCSRCS}}

else ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_F4)

include stm32f4xx_hal/Make.defs
LIB_CINCDIRS = ${addprefix stm32f4xx_hal/,${LIB_DRVCINCDIRS}}
LIB_CSRCS = ${addprefix stm32f4xx_hal/,${LIB_DRVCSRCS}}

else ifeq (${BSP_LIBCONFIG_STM32_SERIES},BSP_STM32_H7)

include stm32h7xx_hal/Make.defs
LIB_CINCDIRS = ${addprefix stm32h7xx_hal/,${LIB_DRVCINCDIRS}}
LIB_CSRCS = ${addprefix stm32h7xx_hal/,${LIB_DRVCSRCS}}
endif

LIB_CINCDIRS += cmsis
