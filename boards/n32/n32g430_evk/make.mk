###############################################
# Manufacturer Library files make Configuration
###############################################

# enable/disable low level driver user library
# absolutely enable
BSP_LIBCONFIG_N32_LLDRV=y

# compile macros for low level driver user library
BSP_LIBCONFIG_N32_LLDRV_SERIES=DRV_BSP_G430

# sdk sources selection, enable stm32 cubeLibrary F4
BSP_LIBCONFIG_N32_SERIES=BSP_N32G430

# cubeLibrary modules and low level drivers selection
BSP_LIBCONFIG_N32_STD_USART=y
BSP_LIBCONFIG_N32_STD_SPI=y
BSP_LIBCONFIG_N32_STD_FLASH=y

MK_GPDRIVE_RINGBUFFER=y

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/boards/n32/libs/bsp_libs_n32.mk
CSOURCES += ${LIB_CSRCS}
PROJ_CINCDIRS += ${LIB_CINCDIRS}

BOARD_BSP_PATH := boards/n32/n32g430_evk

#########################################################################
# BSP macros, sources + asm + link files, includes, and entry address
#########################################################################
PROJ_CDEFS += N32G430
PROJ_CDEFS += USE_STDPERIPH_DRIVER

PROJ_CINCDIRS += ${BOARD_BSP_PATH}
CSOURCES += ${BOARD_BSP_PATH}/board_irq.c
CSOURCES += ${BOARD_BSP_PATH}/board_rcc_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_init.c
CSOURCES += ${BOARD_BSP_PATH}/board_bsp.c

MOD_ARCH = m4
TC_FPU_OPTION = sp
PROJ_ENTRY_POINT := Reset_Handler

ASMSOURCES += ${BOARD_BSP_PATH}/n32g430_startup.s
SCF_FILE   += ${BOARD_BSP_PATH}/n32g430_linker.sct
