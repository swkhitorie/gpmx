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
BSP_LIBCONFIG_STM32_HAL_I2C=n
BSP_LIBCONFIG_STM32_HAL_SPI=n
BSP_LIBCONFIG_STM32_HAL_QSPI=y
BSP_LIBCONFIG_STM32_HAL_MMCSD=y

MK_GPDRIVE_RINGBUFFER=y
MK_GPDRIVE_DNODE=y
MK_GPDRIVE_SERIAL=y
MK_GPDRIVE_I2C=n
MK_GPDRIVE_SPI=n
MK_GPDRIVE_CAN=n
MK_GPDRIVE_QUADSPI=y
MK_GPDRIVE_RTC=y

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/boards/stm32/libs/bsp_libs_stm32.mk
CSOURCES += ${LIB_CSRCS}
PROJ_CINCDIRS += ${LIB_CINCDIRS}

BOARD_BSP_PATH := boards/stm32/fanke_tech_h7

#########################################################################
# BSP macros, sources + asm + link files, includes, and entry address
#########################################################################
PROJ_CDEFS += ${BSP_LIBCONFIG_STM32_LLDRV_SERIES}
PROJ_CDEFS += STM32H743xx
PROJ_CDEFS += USE_HAL_DRIVER
PROJ_CDEFS += CONFIG_RTC_USING_LSE

ifeq (${MK_USE_FS_LITTLEFS},y)
PROJ_CDEFS += LFS_DEFINES=lfs_conf.h
CSOURCES += ${BOARD_BSP_PATH}/w25qxx_driver.c
CSOURCES += ${BOARD_BSP_PATH}/component/lfs_sflash_drv.c
endif

MOD_ARCH = m7
PROJ_ENTRY_POINT := Reset_Handler

PROJ_CINCDIRS += ${BOARD_BSP_PATH}
PROJ_CINCDIRS += ${BOARD_BSP_PATH}/component
CSOURCES += ${BOARD_BSP_PATH}/board_irq.c
CSOURCES += ${BOARD_BSP_PATH}/board_rcc_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_bsp.c
CSOURCES += ${BOARD_BSP_PATH}/board_msp.c
CSOURCES += ${BOARD_BSP_PATH}/component/board_usb_cdc.c
CSOURCES += ${BOARD_BSP_PATH}/component/board_usb_msp.c

ifeq (${PROJ_TC},gae)
ASMSOURCES += ${BOARD_BSP_PATH}/fanketech_h7_startup_gcc.s
SCF_FILE   += ${BOARD_BSP_PATH}/fanketech_h7_lnk_gcc.ld
else ifeq ($(PROJ_TC),armclang)
ASMSOURCES += ${BOARD_BSP_PATH}/fanketech_h7_startup_arm.s
SCF_FILE   += ${BOARD_BSP_PATH}/fanketech_h7_lnk_arm.sct
else
$(error invalid compiler selection)
endif
