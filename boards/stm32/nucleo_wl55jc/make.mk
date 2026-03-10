###############################################
# Manufacturer Library files make Configuration
###############################################

# enable/disable low level driver user library
# absolutely enable
BSP_LIBCONFIG_STM32_LLDRV=y

# compile macros for low level driver user library
BSP_LIBCONFIG_STM32_LLDRV_SERIES=DRV_STM32_WL

# sdk sources selection, enable stm32 cubeLibrary WL
BSP_LIBCONFIG_STM32_SERIES=BSP_STM32_WL

# cubeLibrary modules and low level drivers selection
BSP_LIBCONFIG_STM32_DRV_LEGACY=n

BSP_LIBCONFIG_STM32_HAL_USART=y
BSP_LIBCONFIG_STM32_HAL_IWDG=y
BSP_LIBCONFIG_STM32_HAL_RTC=y
BSP_LIBCONFIG_STM32_HAL_FLASH=y
BSP_LIBCONFIG_STM32_HAL_BASTIM=n
BSP_LIBCONFIG_STM32_HAL_I2C=n
BSP_LIBCONFIG_STM32_HAL_SPI=n
BSP_LIBCONFIG_STM32_HAL_RNG=y
BSP_LIBCONFIG_STM32_HAL_CRC=y

BSP_LIBCONFIG_STM32WL_UTILITIES_MISC=y
BSP_LIBCONFIG_STM32WL_UTILITIES_LPM=y
BSP_LIBCONFIG_STM32WL_UTILITIES_SEQUENCER=y
BSP_LIBCONFIG_STM32WL_UTILITIES_TRACE=y
BSP_LIBCONFIG_STM32WL_UTILITIES_TIMER=y
BSP_LIBCONFIG_STM32WL_MIDDLEWARE_SUBGHZ_PHY=y
BSP_LIBCONFIG_STM32WL_MIDDLEWARE_LORAWAN=n

MK_GPDRIVE_RINGBUFFER=y
MK_GPDRIVE_DNODE=y
MK_GPDRIVE_SERIAL=y
MK_GPDRIVE_I2C=n
MK_GPDRIVE_SPI=n
MK_GPDRIVE_CAN=n
MK_GPDRIVE_QUADSPI=n
MK_GPDRIVE_RTC=y

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/boards/stm32/libs/bsp_libs_stm32.mk
CSOURCES += ${LIB_CSRCS}
PROJ_CINCDIRS += ${LIB_CINCDIRS}

BOARD_BSP_PATH := boards/stm32/nucleo_wl55jc

#########################################################################
# BSP macros, sources + asm + link files, includes, and entry address
#########################################################################
PROJ_CDEFS += ${BSP_LIBCONFIG_STM32_LLDRV_SERIES}
PROJ_CDEFS += STM32WL55xx
PROJ_CDEFS += USE_HAL_DRIVER
PROJ_CDEFS += CORE_CM4
# PROJ_CDEFS += CONFIG_RTC_USING_LSE

PROJ_CINCDIRS += ${BOARD_BSP_PATH}
PROJ_CINCDIRS += ${BOARD_BSP_PATH}/component
CSOURCES += ${BOARD_BSP_PATH}/board_irq.c
CSOURCES += ${BOARD_BSP_PATH}/board_rcc_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_bsp.c
CSOURCES += ${BOARD_BSP_PATH}/board_msp.c
CSOURCES += ${BOARD_BSP_PATH}/board_rand.c
CSOURCES += ${BOARD_BSP_PATH}/board_crc.c
CSOURCES += ${BOARD_BSP_PATH}/board_subghz.c

PROJ_CINCDIRS += ${BOARD_BSP_PATH}/middleware_subghz_if/
CSOURCES += ${BOARD_BSP_PATH}/middleware_subghz_if/radio_board.c
CSOURCES += ${BOARD_BSP_PATH}/middleware_subghz_if/radio_board_if.c

PROJ_CINCDIRS += ${BOARD_BSP_PATH}/utilities_if/
CSOURCES += ${BOARD_BSP_PATH}/utilities_if/stm32_adv_trace_if.c
CSOURCES += ${BOARD_BSP_PATH}/utilities_if/stm32_lpm_if.c
CSOURCES += ${BOARD_BSP_PATH}/utilities_if/timer_if.c

ifeq (${MK_USE_KERNEL_HRT},y)
PROJ_CINCDIRS += ${BOARD_BSP_PATH}/hrtimer
CSOURCES += ${BOARD_BSP_PATH}/hrtimer/hrt.c
endif

MOD_ARCH = m4
TC_FPU_OPTION = none
PROJ_ENTRY_POINT := Reset_Handler

ASMSOURCES += ${BOARD_BSP_PATH}/nucleo_wl55jc_startup.s
SCF_FILE   += ${BOARD_BSP_PATH}/nucleo_wl55jc_linker.ld

# use bare metal
MK_RTOS:=none
MK_USE_HARDFAULTINFO:=n
MK_USE_KERNEL_CPP:=n
MK_USE_KERNEL_POSIX_MQUEUE:=n
MK_USE_KERNEL_POSIX_PTHREAD:=n
MK_USE_KERNEL_POSIX_SEMAPHORE:=n
MK_USE_KERNEL_POSIX_TIMER:=n
MK_USE_KERNEL_POSIX_TIME:=n
MK_USE_KERNEL_WORKQUEUE:=n
MK_USE_KERNEL_PERF:=n

MK_USE_FS_FATFS:=n
MK_USE_FS_LITTLEFS:=n
MK_USE_FS_ROMFS:=n
MK_USE_NET_LWIP_CORE:=n
MK_USE_CRUSB:=n

PROJ_SCF_DEFS += -DLR_CODE_BASE=0x08000000
PROJ_SCF_DEFS += -DLR_CODE_SIZE=254K
PROJ_SCF_DEFS += -DNVM_DATA_BASE=0x0803F800
PROJ_SCF_DEFS += -DNVM_DATA_SIZE=2K
PROJ_CDEFS += NVM_DATA_BASE=0x0803F800
