
BSP_LIBCONFIG_STM32_LLDRV  := y
BSP_LIBCONFIG_STM32_SERIES := BSP_STM32_H7

BSP_LIBCONFIG_STM32_HAL_FLASH := y
BSP_LIBCONFIG_STM32_HAL_IWDG  := y
BSP_LIBCONFIG_STM32_HAL_RTC   := y
BSP_LIBCONFIG_STM32_HAL_I2C   := y
BSP_LIBCONFIG_STM32_HAL_USART := y
BSP_LIBCONFIG_STM32_HAL_SPI   := y
BSP_LIBCONFIG_STM32_HAL_BASTIM:= y

BSP_BOARD_CDEFS := STM32H743
BSP_BOARD_CDEFS += USE_HAL_DRIVER

BSP_BOARD_CSRCS += board_irq.c
BSP_BOARD_CSRCS += board_rcc_init.c
BSP_BOARD_CSRCS += board_usb.c
BSP_BOARD_CSRCS += board_init.c
BSP_BOARD_CSRCS += board_bsp.c

BSP_BOARD_ASMSOURCES += fankeh7_startup_gcc.s
BSP_BOARD_LNK_FILE   += fankeh7_lnk_gcc.ld

BSP_BOARD_CSRCS = ${addprefix ${BSP_ROOT}/stm32/stm32h743_fanke_eval/,${BSP_BOARD_CSRCS}}
BSP_BOARD_ASMSOURCES = ${addprefix ${BSP_ROOT}/stm32/stm32h743_fanke_eval/,${BSP_BOARD_ASMSOURCES}}
BSP_BOARD_LNK_FILE = ${addprefix ${BSP_ROOT}/stm32/stm32h743_fanke_eval/,${BSP_BOARD_LNK_FILE}}

BSP_BOARD_ENTRY_POINT :=  Reset_Handler

# user use values: 
# BSP_BOARD_CDEFS
# BSP_CINCDIRS     (need add user path prefix)
# BSP_BOARD_CSRCS        (need add user path prefix)
# BSP_BOARD_ASMSOURCES   (need add user path prefix)
# BSP_BOARD_LNK_FILE     (need add user path prefix)
# BSP_BOARD_ENTRY_POINT
