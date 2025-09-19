#ifndef DRIVE_COMMON_H_
#define DRIVE_COMMON_H_

#if   defined(DRV_STM32_F1)
#include "stm32f1xx.h"
#include "core_cm3.h"
#elif defined(DRV_STM32_F4)
#include "stm32f4xx.h"
#include "core_cm4.h"
#elif defined(DRV_STM32_H7)
#include "stm32h7xx.h"
#include "core_cm7.h"
#elif defined(DRV_STM32_WL)
#include "stm32wlxx.h"
#if defined(CORE_M4)
#include "core_cm4.h"
#elif defined(CORE_M0)
#include "core_cm0.h"
#endif
#endif

#define arm_dsb()  __asm__ __volatile__ ("dsb " : : : "memory")
#define arm_isb()  __asm__ __volatile__ ("isb " : : : "memory")
#define arm_dmb()  __asm__ __volatile__ ("dmb " : : : "memory")

#define UP_DSB()  arm_dsb()
#define UP_ISB()  arm_isb()
#define UP_DMB()  arm_dmb()

#define UP_MB()   \
    do {          \
        UP_DSB(); \
        UP_ISB(); \
    } while (0)

#include <stdint.h>

#define getreg8(a)     (*(volatile uint8_t *)(a))
#define putreg8(v,a)   (*(volatile uint8_t *)(a) = (v))
#define getreg16(a)    (*(volatile uint16_t *)(a))
#define putreg16(v,a)  (*(volatile uint16_t *)(a) = (v))
#define getreg32(a)    (*(volatile uint32_t *)(a))
#define putreg32(v,a)  (*(volatile uint32_t *)(a) = (v))
#define getreg64(a)    (*(volatile uint64_t *)(a))
#define putreg64(v,a)  (*(volatile uint64_t *)(a) = (v))

#define modreg8(v,m,a)  putreg8((getreg8(a) & ~(m)) | ((v) & (m)), (a))
#define modreg16(v,m,a) putreg16((getreg16(a) & ~(m)) | ((v) & (m)), (a))
#define modreg32(v,m,a) putreg32((getreg32(a) & ~(m)) | ((v) & (m)), (a))
#define modreg64(v,m,a) putreg64((getreg64(a) & ~(m)) | ((v) & (m)), (a))


/****************************************************************************
 * EXIT IO Periphal
 ****************************************************************************/
#define CONFIG_STM32_IO_IRQ_LINE_NUM (16)

/****************************************************************************
 * USART Periphal
 ****************************************************************************/

#if defined(UART8)
#define CONFIG_STM32_UART_NUM (8)
#elif defined(UART7)
#define CONFIG_STM32_UART_NUM (7)
#elif defined(USART6)
#define CONFIG_STM32_UART_NUM (6)
#elif defined(UART5) || defined(USART5)
#define CONFIG_STM32_UART_NUM (5)
#elif defined(USART4) || defined(UART4)
#define CONFIG_STM32_UART_NUM (4)
#elif defined(USART3)
#define CONFIG_STM32_UART_NUM (3)
#elif defined(USART2)
#define CONFIG_STM32_UART_NUM (2)
#elif defined(USART1)
#define CONFIG_STM32_UART_NUM (1)
#else
#define CONFIG_STM32_UART_NUM (0)
#endif

/****************************************************************************
 * SPI Periphal
 ****************************************************************************/

#if defined(SPI6)
#define CONFIG_STM32_SPI_NUM (6)
#elif defined(SPI5)
#define CONFIG_STM32_SPI_NUM (5)
#elif defined(SPI4)
#define CONFIG_STM32_SPI_NUM (4)
#elif defined(SPI3)
#define CONFIG_STM32_SPI_NUM (3)
#elif defined(SPI2)
#define CONFIG_STM32_SPI_NUM (2)
#elif defined(SPI1)
#define CONFIG_STM32_SPI_NUM (1)
#else
#define CONFIG_STM32_SPI_NUM (0)
#endif

/****************************************************************************
 * I2C Periphal
 ****************************************************************************/

#if defined(I2C5)
#define CONFIG_STM32_I2C_NUM (5)
#elif defined(I2C4)
#define CONFIG_STM32_I2C_NUM (4)
#elif defined(I2C3)
#define CONFIG_STM32_I2C_NUM (3)
#elif defined(I2C2)
#define CONFIG_STM32_I2C_NUM (2)
#elif defined(I2C1)
#define CONFIG_STM32_I2C_NUM (1)
#else
#define CONFIG_STM32_I2C_NUM (0)
#endif

/****************************************************************************
 * CAN Periphal
 ****************************************************************************/

#if defined(CAN3)
#define CONFIG_STM32_CAN_NUM (3)
#elif defined(CAN2)
#define CONFIG_STM32_CAN_NUM (2)
#elif defined(CAN1)
#define CONFIG_STM32_CAN_NUM (1)
#else
#define CONFIG_STM32_CAN_NUM (0)
#endif



#endif

