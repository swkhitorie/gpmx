###############################################
# Manufacturer Library files make Configuration
###############################################

# enable/disable low level driver user library
# absolutely enable
BSP_LIBCONFIG_STM32_LLDRV=y

# compile macros for low level driver user library
BSP_LIBCONFIG_STM32_LLDRV_SERIES=DRV_BSP_H7

# sdk sources selection, enable stm32 cubeLibrary H7
BSP_LIBCONFIG_STM32_SERIES=BSP_STM32_H7

# cubeLibrary modules and low level drivers selection
BSP_LIBCONFIG_STM32_DRV_LEGACY=y

BSP_LIBCONFIG_STM32_HAL_USART=y
BSP_LIBCONFIG_STM32_HAL_IWDG=y
BSP_LIBCONFIG_STM32_HAL_RTC=n
BSP_LIBCONFIG_STM32_HAL_FLASH=n
BSP_LIBCONFIG_STM32_HAL_BASTIM=n
BSP_LIBCONFIG_STM32_HAL_I2C=y
BSP_LIBCONFIG_STM32_HAL_SPI=y
BSP_LIBCONFIG_STM32_HAL_QSPI=n
BSP_LIBCONFIG_STM32_HAL_MMCSD=y

MK_GPDRIVE_RINGBUFFER=y
MK_GPDRIVE_DNODE=y
MK_GPDRIVE_SERIAL=y
MK_GPDRIVE_I2C=y
MK_GPDRIVE_SPI=y
MK_GPDRIVE_CAN=n
MK_GPDRIVE_QUADSPI=n

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/boards/stm32/libs/bsp_libs_stm32.mk
CSOURCES += ${LIB_CSRCS}
PROJ_CINCDIRS += ${LIB_CINCDIRS}

BOARD_BSP_PATH := boards/stm32/stm32h743_fmuv6

#########################################################################
# BSP macros, sources + asm + link files, includes, and entry address
#########################################################################
PROJ_CDEFS += ${BSP_LIBCONFIG_STM32_LLDRV_SERIES}
PROJ_CDEFS += STM32H743xx
PROJ_CDEFS += USE_HAL_DRIVER

PROJ_CDEFS += CONFIG_STM32_DMA_SPI1_RX
PROJ_CDEFS += CONFIG_STM32_DMA_SPI1_TX
PROJ_CDEFS += CONFIG_STM32_DMA_SPI2_RX
PROJ_CDEFS += CONFIG_STM32_DMA_SPI2_TX

PROJ_CINCDIRS += ${BOARD_BSP_PATH}

CSOURCES += ${BOARD_BSP_PATH}/board_irq.c
CSOURCES += ${BOARD_BSP_PATH}/board_rcc_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_usb.c
CSOURCES += ${BOARD_BSP_PATH}/board_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_bsp.c

# BOARD_CPPSRCS += px4_i2c.cpp
# BOARD_CPPSRCS += px4_spi.cpp

MOD_ARCH = m7
TC_FPU_OPTION = dp
PROJ_ENTRY_POINT := Reset_Handler

ASMSOURCES += ${BOARD_BSP_PATH}/fmuv6_startup.s
SCF_FILE   += ${BOARD_BSP_PATH}/fmuv6_lnk_script.ld
