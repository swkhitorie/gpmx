###############################################
# Manufacturer Library files make Configuration
###############################################

# enable/disable low level driver user library
# absolutely enable
BSP_LIBCONFIG_STM32_LLDRV=y

# compile macros for low level driver user library
BSP_LIBCONFIG_STM32_LLDRV_SERIES=DRV_BSP_F1

# sdk sources selection, enable stm32 cubeLibrary F1
BSP_LIBCONFIG_STM32_SERIES=BSP_STM32_F1

# cubeLibrary modules and low level drivers selection
BSP_LIBCONFIG_STM32_HAL_USART=y
BSP_LIBCONFIG_STM32_HAL_IWDG=y
BSP_LIBCONFIG_STM32_HAL_RTC=y
BSP_LIBCONFIG_STM32_HAL_FLASH=n
BSP_LIBCONFIG_STM32_HAL_BASTIM=n
BSP_LIBCONFIG_STM32_HAL_I2C=n
BSP_LIBCONFIG_STM32_HAL_SPI=y

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/bsp/stm32/libs/bsp_libs_stm32.mk

#########################################################################
# BSP macros, sources + asm + link files, includes, and entry address
#########################################################################
BOARD_CDEFS += ${BSP_LIBCONFIG_STM32_LLDRV_SERIES}
BOARD_CDEFS += STM32F103xB
BOARD_CDEFS += USE_HAL_DRIVER

BOARD_CSRCS += board_irq.c
BOARD_CSRCS += board_rcc_init.c
BOARD_CSRCS += board_init.c
BOARD_CSRCS += board_bsp.c

BOARD_ASMSOURCES += stm32f103_eval_startup_gcc.s
BOARD_LNK_FILE   += stm32f103_eval_lnk_gcc.ld

TMPBOARD_CSRCS = ${addprefix bsp/stm32/stm32f103_eval/,${BOARD_CSRCS}}
TMPBOARD_ASMSOURCES = ${addprefix bsp/stm32/stm32f103_eval/,${BOARD_ASMSOURCES}}
TMPBOARD_LNK_FILE = ${addprefix bsp/stm32/stm32f103_eval/,${BOARD_LNK_FILE}}

BSP_BOARD_ENTRY_POINT   := Reset_Handler
BSP_CDEFS               += ${BOARD_CDEFS}
BSP_CSRCS               += ${TMPBOARD_CSRCS} ${LIB_CSRCS}
BSP_CINCDIRS            += ${LIB_CINCDIRS} bsp/stm32/stm32f103_eval
BSP_ASMSOURCES          := ${TMPBOARD_ASMSOURCES}
BSP_LNK_FILE            := ${TMPBOARD_LNK_FILE}

#######################################
# Add all setting to root make variable
#######################################
PROJ_CDEFS              += ${BSP_CDEFS}
CSOURCES                += ${BSP_CSRCS}
PROJ_CINCDIRS           += ${BSP_CINCDIRS}
ASMSOURCES              := ${BSP_ASMSOURCES}
SCF_FILE                := ${BSP_LNK_FILE}
PROJ_ENTRY_POINT        := ${BSP_BOARD_ENTRY_POINT}
