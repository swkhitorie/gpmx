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
BSP_LIBCONFIG_STM32_HAL_IWDG=y
BSP_LIBCONFIG_STM32_HAL_RTC=y
BSP_LIBCONFIG_STM32_HAL_FLASH=n
BSP_LIBCONFIG_STM32_HAL_BASTIM=y

BSP_LIBCONFIG_STM32_HAL_USART=y
BSP_LIBCONFIG_STM32_HAL_I2C=y
BSP_LIBCONFIG_STM32_HAL_SPI=y
BSP_LIBCONFIG_STM32_HAL_CAN=n
BSP_LIBCONFIG_STM32_HAL_QSPI=n
BSP_LIBCONFIG_STM32_HAL_MMCSD=y

CONFIG_DRIVER_SERIAL=y
CONFIG_DRIVER_I2C=y
CONFIG_DRIVER_SPI=y
CONFIG_DRIVER_CAN=n
CONFIG_DRIVER_QUADSPI=n
CONFIG_DRIVER_RTC=y
CONFIG_DRIVER_MMCSDSPI=n

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/boards/arch/stm32/bsp_libs_stm32.mk
CSOURCES += ${LIB_CSRCS}
PROJ_CINCDIRS += ${LIB_CINCDIRS}

BOARD_BSP_PATH := boards/pixhawk_fmuv2

#########################################################################
# BSP macros, sources + asm + link files, includes, and entry address
#########################################################################
PROJ_CDEFS += ${BSP_LIBCONFIG_STM32_LLDRV_SERIES}
EXT_CDEFS += STM32F427xx
EXT_CDEFS += USE_HAL_DRIVER
PROJ_CDEFS += BOARD_PIXHAWK_FMUV2

PROJ_CDEFS += CONFIG_FMU_V2_USB_FS

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

ifeq (${CONFIG_FATFS_ENABLE},y)
PROJ_CDEFS += CONFIG_STM32_MMCSD_FATFS_ENABLE
endif

ifeq (${CONFIG_CRUSB_DEVICE_ENABLE},y)
ifeq (${CONFIG_CRUSB_IP},"dwc2_st")
ifeq ($(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE),y)
CSOURCES += ${BOARD_BSP_PATH}/component/board_usb_cdc.c
CSOURCES += ${BOARD_BSP_PATH}/component/board_usb_msp.c
endif # end with CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE
else
$(error Invalid USB IP setting in board pixhawk_fmuv2)
endif # end with CONFIG_CRUSB_IP
endif # end with CONFIG_CRUSB_DEVICE_ENABLE

ifeq (${CONFIG_MTD},y)
CONFIG_MTD_RAMTRON=y
endif

MOD_ARCH = m4
TC_FPU_OPTION = sp
PROJ_ENTRY_POINT := Reset_Handler

ASMSOURCES += ${BOARD_BSP_PATH}/fmuv2_startup.S
SCF_FILE   += ${BOARD_BSP_PATH}/fmuv2_lnk_script.ld

PROJ_CINCDIRS += ${BOARD_BSP_PATH}/driver
CSOURCES += ${BOARD_BSP_PATH}/driver/fm25vxx_flash.c
CSOURCES += ${BOARD_BSP_PATH}/driver/l3gd20_drv.c
CSOURCES += ${BOARD_BSP_PATH}/driver/lsm303d_drv.c
CSOURCES += ${BOARD_BSP_PATH}/driver/mpu6000_drv.c
CSOURCES += ${BOARD_BSP_PATH}/driver/tca62724.c

