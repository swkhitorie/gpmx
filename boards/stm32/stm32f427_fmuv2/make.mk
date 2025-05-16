###############################################
# Manufacturer Library files make Configuration
###############################################

# enable/disable low level driver user library
# absolutely enable
BSP_LIBCONFIG_STM32_LLDRV=y

# compile macros for low level driver user library
BSP_LIBCONFIG_STM32_LLDRV_SERIES=DRV_BSP_F4

# sdk sources selection, enable stm32 cubeLibrary F4
BSP_LIBCONFIG_STM32_SERIES=BSP_STM32_F4

# cubeLibrary modules and low level drivers selection
BSP_LIBCONFIG_STM32_HAL_USART=y
BSP_LIBCONFIG_STM32_HAL_IWDG=y
BSP_LIBCONFIG_STM32_HAL_RTC=y
BSP_LIBCONFIG_STM32_HAL_FLASH=n
BSP_LIBCONFIG_STM32_HAL_BASTIM=n
BSP_LIBCONFIG_STM32_HAL_I2C=n
BSP_LIBCONFIG_STM32_HAL_SPI=y
BSP_LIBCONFIG_STM32_HAL_QSPI=n
BSP_LIBCONFIG_STM32_HAL_MMCSD=n

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/boards/stm32/libs/bsp_libs_stm32.mk
CSOURCES += ${LIB_CSRCS}
PROJ_CINCDIRS += ${LIB_CINCDIRS}

#########################################################################
# BSP macros, sources + asm + link files, includes, and entry address
#########################################################################
BOARD_CDEFS += ${BSP_LIBCONFIG_STM32_LLDRV_SERIES}
BOARD_CDEFS += STM32F427xx
BOARD_CDEFS += USE_HAL_DRIVER

BOARD_CSRCS += board_irq.c
BOARD_CSRCS += board_rcc_init.c
BOARD_CSRCS += board_usb.c
BOARD_CSRCS += board_init.c
BOARD_CSRCS += board_bsp.c

BOARD_ASMSOURCES += fmuv2_startup.s
BOARD_LNK_FILE   += fmuv2_lnk_script.ld

BOARD_CDEFS += CONFIG_STM32_DMA_SPI1_RX
BOARD_CDEFS += CONFIG_STM32_DMA_SPI1_TX

TMPBOARD_CSRCS = ${addprefix boards/stm32/stm32f427_fmuv2/,${BOARD_CSRCS}}
TMPBOARD_ASMSOURCES = ${addprefix boards/stm32/stm32f427_fmuv2/,${BOARD_ASMSOURCES}}
TMPBOARD_LNK_FILE = ${addprefix boards/stm32/stm32f427_fmuv2/,${BOARD_LNK_FILE}}

#######################################
# Add all setting to root make variable
#######################################
PROJ_ENTRY_POINT        := Reset_Handler
SCF_FILE                := ${TMPBOARD_LNK_FILE}
PROJ_CDEFS              += ${BOARD_CDEFS}
PROJ_CINCDIRS           += boards/stm32/stm32f427_fmuv2
CSOURCES                += ${TMPBOARD_CSRCS}
ASMSOURCES              := ${TMPBOARD_ASMSOURCES}
