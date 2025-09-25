###############################################
# Manufacturer Library files make Configuration
###############################################

# enable/disable low level driver user library
# absolutely enable
BSP_LIBCONFIG_STM32_LLDRV=y

# compile macros for low level driver user library
BSP_LIBCONFIG_STM32_LLDRV_SERIES=DRV_STM32_H7

# sdk sources selection, enable stm32 cubeLibrary H7
BSP_LIBCONFIG_STM32_SERIES=BSP_STM32_H7

# cubeLibrary modules and low level drivers selection 
BSP_LIBCONFIG_STM32_DRV_LEGACY=n

BSP_LIBCONFIG_STM32_HAL_USART=y
BSP_LIBCONFIG_STM32_HAL_IWDG=y
BSP_LIBCONFIG_STM32_HAL_RTC=y
BSP_LIBCONFIG_STM32_HAL_FLASH=y
BSP_LIBCONFIG_STM32_HAL_BASTIM=n
BSP_LIBCONFIG_STM32_HAL_I2C=y
BSP_LIBCONFIG_STM32_HAL_SPI=y
BSP_LIBCONFIG_STM32_HAL_QSPI=y
BSP_LIBCONFIG_STM32_HAL_MMCSD=y

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/boards/stm32/libs/bsp_libs_stm32.mk
CSOURCES += ${LIB_CSRCS}
PROJ_CINCDIRS += ${LIB_CINCDIRS}

BOARD_BSP_PATH := boards/stm32/fanke_tech_h7

#########################################################################
# BSP macros, sources + asm + link files, includes, and entry address
#########################################################################
BOARD_CDEFS += ${BSP_LIBCONFIG_STM32_LLDRV_SERIES}
BOARD_CDEFS += STM32H743xx
BOARD_CDEFS += USE_HAL_DRIVER
BOARD_CDEFS += CONFIG_RTC_USING_LSE

BOARD_CSRCS += board_irq.c
BOARD_CSRCS += board_rcc_init.c
BOARD_CSRCS += board_usb.c
BOARD_CSRCS += board_init.c
BOARD_CSRCS += board_bsp.c
BOARD_CSRCS += board_msp.c

ifeq (${PROJ_TC},gae)
BOARD_ASMSOURCES += fanketech_h7_startup_gcc.s
BOARD_LNK_FILE   += fanketech_h7_lnk_gcc.ld
else ifeq ($(PROJ_TC),armclang)
BOARD_ASMSOURCES += fanketech_h7_startup_arm.s
BOARD_LNK_FILE   += fanketech_h7_lnk_arm.sct
else
$(error invalid compiler selection)
endif

TMPBOARD_CSRCS = ${addprefix ${BOARD_BSP_PATH}/,${BOARD_CSRCS}}
TMPBOARD_ASMSOURCES = ${addprefix ${BOARD_BSP_PATH}/,${BOARD_ASMSOURCES}}
TMPBOARD_LNK_FILE = ${addprefix ${BOARD_BSP_PATH}/,${BOARD_LNK_FILE}}

#######################################
# Add all setting to root make variable
#######################################
MOD_ARCH                 = m7
PROJ_ENTRY_POINT        := Reset_Handler
SCF_FILE                := ${TMPBOARD_LNK_FILE}

PROJ_CDEFS              += ${BOARD_CDEFS}

PROJ_CINCDIRS           += ${BOARD_BSP_PATH}
CSOURCES                += ${TMPBOARD_CSRCS}
ASMSOURCES              := ${TMPBOARD_ASMSOURCES}
