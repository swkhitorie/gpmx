###############################################
# Manufacturer Library files make Configuration
###############################################

# enable/disable low level driver user library
# absolutely enable
BSP_LIBCONFIG_STM32_LLDRV=y

# compile macros for low level driver user library
BSP_LIBCONFIG_STM32_LLDRV_SERIES=DRV_BSP_WL

# sdk sources selection, enable stm32 cubeLibrary WL
BSP_LIBCONFIG_STM32_SERIES=BSP_STM32_WL

# cubeLibrary modules and low level drivers selection
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

# include all cubelibrary files and low level driver files
include ${SDK_ROOTDIR}/boards/stm32/libs/bsp_libs_stm32.mk
CSOURCES += ${LIB_CSRCS}
PROJ_CINCDIRS += ${LIB_CINCDIRS}

BOARD_BSP_PATH := boards/stm32/stm32wle5_e77_900mbl

#########################################################################
# BSP macros, sources + asm + link files, includes, and entry address
#########################################################################
BOARD_CDEFS += ${BSP_LIBCONFIG_STM32_LLDRV_SERIES}
BOARD_CDEFS += STM32WLE5xx
BOARD_CDEFS += USE_HAL_DRIVER
BOARD_CDEFS += CORE_CM4

BOARD_CSRCS += board_irq.c
BOARD_CSRCS += board_rcc_init.c
BOARD_CSRCS += board_init.c
BOARD_CSRCS += board_bsp.c
BOARD_CSRCS += board_rand.c
BOARD_CSRCS += board_crc.c
BOARD_CSRCS += board_subghz.c

BOARD_MIDDLEWARE_SUBGHZ_IF_CSRCS += middleware_subghz_if/radio_board.c
BOARD_MIDDLEWARE_SUBGHZ_IF_CSRCS += middleware_subghz_if/radio_board_if.c
BOARD_MIDDLEWARE_SUBGHZ_IF_INCDIRS += ${BOARD_BSP_PATH}/middleware_subghz_if

# BOARD_MIDDLEWARE_LORAWAN_IF_CSRCS += middleware_lorawan_if/lora_info.c
# BOARD_MIDDLEWARE_LORAWAN_IF_INCDIRS += ${BOARD_BSP_PATH}/middleware_lorawan_if

BOARD_UTILITIES_IF_CSRCS += utilities_if/stm32_adv_trace_if.c
BOARD_UTILITIES_IF_CSRCS += utilities_if/stm32_lpm_if.c
BOARD_UTILITIES_IF_CSRCS += utilities_if/timer_if.c
BOARD_UTILITIES_IF_INCDIRS += ${BOARD_BSP_PATH}/utilities_if

BOARD_ASMSOURCES += e77_900mbl_startup.s
BOARD_LNK_FILE   += e77_900mbl_linker.ld

# TMPBOARD_LORAWAN_CSRCS = ${addprefix ${BOARD_BSP_PATH}/,${BOARD_MIDDLEWARE_LORAWAN_IF_CSRCS}}
TMPBOARD_SUBGHZ_CSRCS = ${addprefix ${BOARD_BSP_PATH}/,${BOARD_MIDDLEWARE_SUBGHZ_IF_CSRCS}}
TMPBOARD_UTILITIES_CSRCS = ${addprefix ${BOARD_BSP_PATH}/,${BOARD_UTILITIES_IF_CSRCS}}
TMPBOARD_CSRCS = ${addprefix ${BOARD_BSP_PATH}/,${BOARD_CSRCS}}
TMPBOARD_ASMSOURCES = ${addprefix ${BOARD_BSP_PATH}/,${BOARD_ASMSOURCES}}
TMPBOARD_LNK_FILE = ${addprefix ${BOARD_BSP_PATH}/,${BOARD_LNK_FILE}}

#######################################
# Add all setting to root make variable
#######################################
MOD_ARCH                 = m4
PROJ_ENTRY_POINT        := Reset_Handler
SCF_FILE                := ${TMPBOARD_LNK_FILE}
PROJ_CDEFS              += ${BOARD_CDEFS}

PROJ_CINCDIRS          += ${BOARD_MIDDLEWARE_SUBGHZ_IF_INCDIRS} \
                            ${BOARD_UTILITIES_IF_INCDIRS} \
							${BOARD_BSP_PATH}

CSOURCES          += ${TMPBOARD_CSRCS} \
                            ${TMPBOARD_SUBGHZ_CSRCS} \
							${TMPBOARD_UTILITIES_CSRCS}

ASMSOURCES        := ${TMPBOARD_ASMSOURCES}
