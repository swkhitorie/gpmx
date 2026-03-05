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
BSP_LIBCONFIG_STM32_HAL_ETH_LEGACY=y
BSP_LIBCONFIG_STM32_HAL_SDRAM=y
BSP_LIBCONFIG_STM32_HAL_MMCSD=y

MK_GPDRIVE_RINGBUFFER=y
MK_GPDRIVE_DNODE=y
MK_GPDRIVE_SERIAL=y
MK_GPDRIVE_I2C=y
MK_GPDRIVE_SPI=y
MK_GPDRIVE_CAN=y
MK_GPDRIVE_QUADSPI=n
MK_GPDRIVE_RTC=y
MK_GPDRIVE_MMCSDSPI=n

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/boards/stm32/libs/bsp_libs_stm32.mk
CSOURCES += ${LIB_CSRCS}
PROJ_CINCDIRS += ${LIB_CINCDIRS}

BOARD_BSP_PATH := boards/stm32/pxboard_ebfv2

#########################################################################
# BSP macros, sources + asm + link files, includes, and entry address
#########################################################################
PROJ_CDEFS += ${BSP_LIBCONFIG_STM32_LLDRV_SERIES}
PROJ_CDEFS += STM32F407xx
PROJ_CDEFS += USE_HAL_DRIVER

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

ifeq (${MK_USE_KERNEL_HRT},y)
PROJ_CINCDIRS += ${BOARD_BSP_PATH}/hrtimer
CSOURCES += ${BOARD_BSP_PATH}/hrtimer/hrt.c
endif

ifeq (${MK_USE_FS_FATFS},y)
ifeq (${MK_GPDRIVE_MMCSDSPI},n)
PROJ_CDEFS += CONFIG_STM32_MMCSD_FATFS_ENABLE
endif
endif

ifeq (${MK_USE_CRUSB},y)
ifeq (${MK_USE_CRUSB_IP},dwc2_st)
ifneq ($(filter msc, $(MK_USE_CRUSB_CLASS)),)
CSOURCES += ${BOARD_BSP_PATH}/component/board_usb_cdc.c
CSOURCES += ${BOARD_BSP_PATH}/component/board_usb_msp.c
MK_USE_CRUSB_CLASS+=cdc_acm
else
MK_USE_CRUSB:=n
endif # end with MK_USE_CRUSB_CLASS
else
$(error Invalid USB IP setting in board pxboard_ebfv2)
endif # end with MK_USE_CRUSB_IP
endif # end with MK_USE_CRUSB

ASMSOURCES := ${BOARD_BSP_PATH}/stm32f407_ebf_startup_gcc.s
SCF_FILE := ${BOARD_BSP_PATH}/stm32f407_ebf_lnk_gcc.ld

MOD_ARCH = m4
TC_FPU_OPTION = sp
PROJ_ENTRY_POINT := Reset_Handler
