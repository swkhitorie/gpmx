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

BSP_LIBCONFIG_STM32_HAL_IWDG=y
BSP_LIBCONFIG_STM32_HAL_RTC=y
BSP_LIBCONFIG_STM32_HAL_FLASH=n
BSP_LIBCONFIG_STM32_HAL_BASTIM=n

BSP_LIBCONFIG_STM32_HAL_USART=y
BSP_LIBCONFIG_STM32_HAL_I2C=y
BSP_LIBCONFIG_STM32_HAL_SPI=y
BSP_LIBCONFIG_STM32_HAL_CAN=n
BSP_LIBCONFIG_STM32_HAL_QSPI=n
BSP_LIBCONFIG_STM32_HAL_MMCSD=y
# BSP_LIBCONFIG_STM32_HAL_ETH=y
BSP_LIBCONFIG_STM32_HAL_ETH_LEGACY=n

MK_GPDRIVE_RINGBUFFER=y
MK_GPDRIVE_DNODE=y
MK_GPDRIVE_SERIAL=y
MK_GPDRIVE_I2C=y
MK_GPDRIVE_SPI=y
MK_GPDRIVE_CAN=n
MK_GPDRIVE_QUADSPI=n
MK_GPDRIVE_RTC=y

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/boards/arch/stm32/bsp_libs_stm32.mk
CSOURCES += ${LIB_CSRCS}
PROJ_CINCDIRS += ${LIB_CINCDIRS}

BOARD_BSP_PATH := boards/pixhawk_fmuv6x

#########################################################################
# BSP macros, sources + asm + link files, includes, and entry address
#########################################################################
PROJ_CDEFS += ${BSP_LIBCONFIG_STM32_LLDRV_SERIES}
PROJ_CDEFS += STM32H753xx
PROJ_CDEFS += USE_HAL_DRIVER
PROJ_CDEFS += BOARD_CUAV_FMUV6X

PROJ_CDEFS += PHY_USING_DP83848C
PROJ_CDEFS += CONFIG_LWIP_USING_HW_CHECKSUM
# PROJ_CDEFS += CONFIG_ETH_PHY_ADDR_USER=1

PROJ_CINCDIRS += ${BOARD_BSP_PATH}
PROJ_CINCDIRS += ${BOARD_BSP_PATH}/component
CSOURCES += ${BOARD_BSP_PATH}/board_irq.c
CSOURCES += ${BOARD_BSP_PATH}/board_rcc_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_msp.c
CSOURCES += ${BOARD_BSP_PATH}/board_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_bsp.c

ifeq (${MK_USE_KERNEL_HRT},y)
PROJ_CDEFS += CONFIG_MODULE_HRT
PROJ_CINCDIRS += ${BOARD_BSP_PATH}/hrtimer
CSOURCES += ${BOARD_BSP_PATH}/hrtimer/hrt.c
endif

ifeq (${MK_USE_HARDFAULTINFO},y)
CSOURCES += ${BOARD_BSP_PATH}/component/hardfault_log.c
endif

ifeq (${MK_USE_FS_FATFS},y)
PROJ_CDEFS += CONFIG_STM32_MMCSD_FATFS_ENABLE
endif

ifeq (${MK_USE_CRUSB},y)
ifeq (${MK_USE_CRUSB_IP},dwc2_st)
ifneq ($(filter cdc_acm, $(MK_USE_CRUSB_CLASS)),)
CSOURCES += ${BOARD_BSP_PATH}/component/board_usb_cdc.c
CSOURCES += ${BOARD_BSP_PATH}/component/board_usb_msp.c
endif # end with MK_USE_CRUSB_CLASS
else
$(error Invalid USB IP setting in board pixhawk_fmuv6x)
endif # end with MK_USE_CRUSB_IP
endif # end with MK_USE_CRUSB

# lwip-eth test
MK_USE_NET_LWIP_CORE:=n

MOD_ARCH = m7
TC_FPU_OPTION = dp
PROJ_ENTRY_POINT := Reset_Handler

ASMSOURCES += ${BOARD_BSP_PATH}/fmuv6x_startup.S
SCF_FILE   += ${BOARD_BSP_PATH}/fmuv6x_lnk_script.ld
