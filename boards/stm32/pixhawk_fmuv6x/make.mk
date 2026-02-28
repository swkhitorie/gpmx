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

MK_GPDRIVE_RINGBUFFER=y
MK_GPDRIVE_DNODE=y
MK_GPDRIVE_SERIAL=y
MK_GPDRIVE_I2C=y
MK_GPDRIVE_SPI=y
MK_GPDRIVE_CAN=n
MK_GPDRIVE_QUADSPI=n
MK_GPDRIVE_RTC=y

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/boards/stm32/libs/bsp_libs_stm32.mk
CSOURCES += ${LIB_CSRCS}
PROJ_CINCDIRS += ${LIB_CINCDIRS}

BOARD_BSP_PATH := boards/stm32/pixhawk_fmuv6x

#########################################################################
# BSP macros, sources + asm + link files, includes, and entry address
#########################################################################
PROJ_CDEFS += ${BSP_LIBCONFIG_STM32_LLDRV_SERIES}
PROJ_CDEFS += STM32H753xx
PROJ_CDEFS += USE_HAL_DRIVER

PROJ_CINCDIRS += ${BOARD_BSP_PATH}
PROJ_CINCDIRS += ${BOARD_BSP_PATH}/component
CSOURCES += ${BOARD_BSP_PATH}/board_irq.c
CSOURCES += ${BOARD_BSP_PATH}/board_rcc_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_msp.c
CSOURCES += ${BOARD_BSP_PATH}/board_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_bsp.c

ifeq (${MK_USE_CRUSB},y)
ifeq (${MK_USE_CRUSB_CLASS},cdc_acm)
ifeq (${MK_USE_CRUSB_IP},dwc2_st)
CSOURCES += ${BOARD_BSP_PATH}/component/board_usb_cdc.c
CSOURCES += ${BOARD_BSP_PATH}/component/board_usb_msp.c
else
$(error Invalid USB IP setting in board pixhawk_fmuv2)
endif # end with MK_USE_CRUSB_IP
endif # end with MK_USE_CRUSB_CLASS
endif # end with MK_USE_CRUSB

MOD_ARCH = m7
TC_FPU_OPTION = dp
PROJ_ENTRY_POINT := Reset_Handler

ASMSOURCES += ${BOARD_BSP_PATH}/fmuv6x_startup.s
SCF_FILE   += ${BOARD_BSP_PATH}/fmuv6x_lnk_script.ld
