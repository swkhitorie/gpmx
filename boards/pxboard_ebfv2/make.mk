###############################################
# Manufacturer Library files make Configuration
###############################################

# enable/disable low level driver user library
# absolutely enable
BSP_LIBCONFIG_STM32_LLDRV=y

# compile macros for low level driver user library
BSP_LIBCONFIG_STM32_LLDRV_SERIES=DRV_STM32_F4

# sdk sources selection, enable stm32 cubeLibrary F4
BSP_LIBCONFIG_STM32_SERIES=BSP_STM32_F4

# cubeLibrary modules and low level drivers selection
BSP_LIBCONFIG_STM32_DRV_LEGACY=n

BSP_LIBCONFIG_STM32_HAL_USART=y
BSP_LIBCONFIG_STM32_HAL_IWDG=n
BSP_LIBCONFIG_STM32_HAL_RTC=y
BSP_LIBCONFIG_STM32_HAL_FLASH=n
BSP_LIBCONFIG_STM32_HAL_BASTIM=n
BSP_LIBCONFIG_STM32_HAL_I2C=y
BSP_LIBCONFIG_STM32_HAL_SPI=y
BSP_LIBCONFIG_STM32_HAL_CAN=y
BSP_LIBCONFIG_STM32_HAL_ETH_LEGACY=n
BSP_LIBCONFIG_STM32_HAL_SDRAM=y
BSP_LIBCONFIG_STM32_HAL_MMCSD=y

CONFIG_DRIVER_SERIAL=y
CONFIG_DRIVER_I2C=y
CONFIG_DRIVER_SPI=y
CONFIG_DRIVER_CAN=y
CONFIG_DRIVER_QUADSPI=n
CONFIG_DRIVER_RTC=y
CONFIG_DRIVER_MMCSDSPI=n

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/boards/arch/stm32/bsp_libs_stm32.mk
CSOURCES += ${LIB_CSRCS}
PROJ_CINCDIRS += ${LIB_CINCDIRS}

BOARD_BSP_PATH := boards/pxboard_ebfv2

#########################################################################
# BSP macros, sources + asm + link files, includes, and entry address
#########################################################################
EXT_CDEFS += STM32F407xx
EXT_CDEFS += USE_HAL_DRIVER
PROJ_CDEFS += ${BSP_LIBCONFIG_STM32_LLDRV_SERIES}
PROJ_CDEFS += BOARD_PXBOARD_EBFV2

PROJ_CDEFS += BSP_RTC_USING_LSE
PROJ_CDEFS += PHY_USING_LAN8720A
PROJ_CDEFS += CONFIG_LWIP_USING_HW_CHECKSUM

PROJ_CINCDIRS += ${BOARD_BSP_PATH}
PROJ_CINCDIRS += ${BOARD_BSP_PATH}/component
CSOURCES += ${BOARD_BSP_PATH}/board_irq.c
CSOURCES += ${BOARD_BSP_PATH}/board_rcc_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_bsp.c
CSOURCES += ${BOARD_BSP_PATH}/board_msp.c

ifeq (${CONFIG_CMBACKTRACE},y)
CSOURCES += ${BOARD_BSP_PATH}/component/hardfault_log.c
endif

ifeq (${CONFIG_FS_FATFS},y)
ifeq (${CONFIG_DRIVER_MMCSDSPI},n)
PROJ_CDEFS += CONFIG_STM32_MMCSD_FATFS_ENABLE
endif
endif

ifeq (${CONFIG_CRUSB_DEVICE_ENABLE},y)
ifeq (${CONFIG_CRUSB_IP},"dwc2_st")
ifeq ($(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE),y)
CONFIG_CRUSB_DEVICE_ENABLE=n
# CSOURCES += ${BOARD_BSP_PATH}/component/board_usb_cdc.c
# CSOURCES += ${BOARD_BSP_PATH}/component/board_usb_msp.c
endif # end with CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE
else
$(error Invalid USB IP setting in board pxboard_ebfv2)
endif # end with CONFIG_CRUSB_IP
endif # end with CONFIG_CRUSB_DEVICE_ENABLE

ifeq (${CONFIG_MTD},y)
CONFIG_MTD_AT24XX=y
CONFIG_AT24XX_SIZE=2
CONFIG_AT24XX_ADDR=0x50
CONFIG_AT24XX_FREQUENCY=100000
CONFIG_AT24XX_MTD_BLOCKSIZE=8

CONFIG_MTD_W25=y
CONFIG_W25_SPIMODE=SPIDEV_MODE3
CONFIG_W25_SECTOR512=y
CONFIG_MTD_BYTE_WRITE=y
endif

# lwip-eth test
# MK_USE_NET_LWIP_CORE:=y
# BSP_LIBCONFIG_STM32_HAL_ETH_LEGACY=y

ASMSOURCES := ${BOARD_BSP_PATH}/stm32f407_ebf_startup_gcc.S
SCF_FILE := ${BOARD_BSP_PATH}/stm32f407_ebf_lnk_gcc.ld

MOD_ARCH = m4
TC_FPU_OPTION = sp
PROJ_ENTRY_POINT := Reset_Handler
