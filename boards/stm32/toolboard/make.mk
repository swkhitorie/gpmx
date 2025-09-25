###############################################
# Manufacturer Library files make Configuration
###############################################

# enable/disable low level driver user library
# absolutely enable
BSP_LIBCONFIG_STM32_LLDRV=y

# compile macros for low level driver user library
BSP_LIBCONFIG_STM32_LLDRV_SERIES=DRV_STM32_F1

# sdk sources selection, enable stm32 cubeLibrary F1
BSP_LIBCONFIG_STM32_SERIES=BSP_STM32_F1

# cubeLibrary modules and low level drivers selection
BSP_LIBCONFIG_STM32_DRV_LEGACY=n

BSP_LIBCONFIG_STM32_HAL_USART=y
BSP_LIBCONFIG_STM32_HAL_IWDG=y
BSP_LIBCONFIG_STM32_HAL_RTC=y
BSP_LIBCONFIG_STM32_HAL_FLASH=n
BSP_LIBCONFIG_STM32_HAL_BASTIM=n
BSP_LIBCONFIG_STM32_HAL_I2C=n
BSP_LIBCONFIG_STM32_HAL_SPI=n

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/boards/stm32/libs/bsp_libs_stm32.mk
CSOURCES += ${LIB_CSRCS}
PROJ_CINCDIRS += ${LIB_CINCDIRS}

BOARD_BSP_PATH := boards/stm32/toolboard

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
BOARD_CSRCS += board_msp.c

BOARD_ASMSOURCES += stm32f103_eval_startup_gcc.s
BOARD_LNK_FILE   += stm32f103_eval_lnk_gcc.ld

TMPBOARD_CSRCS = ${addprefix ${BOARD_BSP_PATH}/,${BOARD_CSRCS}}
TMPBOARD_ASMSOURCES = ${addprefix ${BOARD_BSP_PATH}/,${BOARD_ASMSOURCES}}
TMPBOARD_LNK_FILE = ${addprefix ${BOARD_BSP_PATH}/,${BOARD_LNK_FILE}}

#######################################
# Add all setting to root make variable
#######################################
MOD_ARCH                 = m3
PROJ_ENTRY_POINT        := Reset_Handler
SCF_FILE                := ${TMPBOARD_LNK_FILE}

PROJ_CDEFS              += ${BOARD_CDEFS}

PROJ_CINCDIRS           += ${BOARD_BSP_PATH}
CSOURCES                += ${TMPBOARD_CSRCS}
ASMSOURCES              := ${TMPBOARD_ASMSOURCES}
