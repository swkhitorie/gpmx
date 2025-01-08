

BSP_LIBCONFIG_STM32_LLDRV  := y
BSP_LIBCONFIG_STM32_SERIES := BSP_STM32_F4

BSP_LIBCONFIG_STM32_HAL_FLASH := y
BSP_LIBCONFIG_STM32_HAL_IWDG  := y
BSP_LIBCONFIG_STM32_HAL_RTC   := y
BSP_LIBCONFIG_STM32_HAL_I2C   := y
BSP_LIBCONFIG_STM32_HAL_USART := y
BSP_LIBCONFIG_STM32_HAL_SPI   := y
BSP_LIBCONFIG_STM32_HAL_BASTIM:= y

BSP_CDEFS := STM32F427xx
BSP_CDEFS += USE_HAL_DRIVER

include libs/bsp_libs_stm32.mk

LIB_DRVCINCDIRS = ${addprefix libs/,${LIB_DRVCINCDIRS}}
LIB_DRVCSRCS = ${addprefix libs/,${LIB_DRVCSRCS}}

LIB_CINCDIRS = ${addprefix libs/,${LIB_DRVCINCDIRS}}
LIB_CSRCS = ${addprefix libs/,${LIB_DRVCSRCS}}

BSP_CINCDIRS += ${LIB_DRVCINCDIRS}
BSP_CINCDIRS += ${LIB_CINCDIRS}

BSP_CSRCS += ${LIB_CSRCS}
BSP_CSRCS += ${LIB_DRVCSRCS}
BSP_CSRCS += board_irq.c
BSP_CSRCS += board_rcc_init.c
BSP_CSRCS += board_usb.c
BSP_CSRCS += board_init.c

BSP_ENTRY_POINT :=  Reset_Handler

BSP_ASMSOURCES += fmu_startup.s
BSP_LNK_FILE   += fmu_lnk_script.ld

# user use values: 
# BSP_CDEFS
# BSP_CINCDIRS     (need add user path prefix)
# BSP_CSRCS        (need add user path prefix)
# BSP_ASMSOURCES   (need add user path prefix)
# BSP_LNK_FILE     (need add user path prefix)
# BSP_ENTRY_POINT

