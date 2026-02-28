#ifndef BOARD_CONFIG_H_
#define BOARD_CONFIG_H_

/**
 * Pixhawk FMU v6x Board Config Header File
 * cpu: stm32h753iik6
 * oscillator: 16MHz
 * frequency: 480MHz
 * memory: 1024K = (128K + 512K + 128K + 128K + 32K + 64K + ...)
 * flash:  2048K
 * 
 * bug 1: v6c can not enter SysTick_Handler() irq
 *       -> fixed: call __set_BASEPRI(0); of set it priority higher, default 15
 * warning 1: v6c can not enter OTG_FS_IRQHandler() irq
 *       change h7 usb's clock source HSI48 -> PLL3Q (48M)
 *       edit VID and PID to default 0xFFFF, why????
 * bug 2: v6c porting freertos v10.2.1
 *       enter hardfault in prvInitialiseNewTask() (called by xTaskCreate and static )
 *       change configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5 -> 9????
 */
#include <stm32h7xx_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <drv_gpio.h>
#include <drv_rtc.h>

#define APP_LOAD_ADDRESS      (0x08020000)
#define HSE_VALUE             (16000000UL)
#define LSE_VALUE             (32768UL)

#if !defined(CONFIG_BOARD_PRINTF_SOURCE) || \
    (!defined(CONFIG_CRUSB_DEVICE_ENABLE) || !defined(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE))
#define CONFIG_BOARD_PRINTF_SOURCE  (2)
#endif

#define BOARD_PRINTF(...)    board_printf(__VA_ARGS__)

#define BOARD_CRUSH_PRINTF(...)    board_stream_printf(0, __VA_ARGS__)

#ifndef __FPU_PRESENT
#define __FPU_PRESENT         1
#endif

#ifndef __FPU_USED
#define __FPU_USED            1
#endif

#ifndef __FPU_USED
#define __MPU_PRESENT         1
#endif

#define STM32_PLLCFG_PLL1M       (1)
#define STM32_PLLCFG_PLL1N       (60)
#define STM32_PLLCFG_PLL1P       (2)
#define STM32_PLLCFG_PLL1Q       (4)

#define STM32_PLL1P_FREQUENCY    (((HSE_VALUE/STM32_PLLCFG_PLL1M)*STM32_PLLCFG_PLL1N)/STM32_PLLCFG_PLL1P)
#define STM32_SYSCLK_FREQUENCY   (STM32_PLL1P_FREQUENCY)
#define STM32_CPUCLK_FREQUENCY   (STM32_SYSCLK_FREQUENCY / 1)

#define STM32_HCLK_FREQUENCY      (STM32_CPUCLK_FREQUENCY / 2)    /* HCLK in D2, HCLK4 in D3 */

/* APB1 clock (PCLK1) is HCLK/2 (120 MHz) */      
#define STM32_PCLK1_FREQUENCY     (STM32_HCLK_FREQUENCY/2)       /* PCLK1 = HCLK / 2 */

/* APB2 clock (PCLK2) is HCLK/2 (120 MHz) */    
#define STM32_PCLK2_FREQUENCY     (STM32_HCLK_FREQUENCY/2)       /* PCLK2 = HCLK / 2 */

/* APB3 clock (PCLK3) is HCLK/2 (120 MHz) */   
#define STM32_PCLK3_FREQUENCY     (STM32_HCLK_FREQUENCY/2)       /* PCLK3 = HCLK / 2 */

/* APB4 clock (PCLK4) is HCLK/4 (120 MHz) */ 
#define STM32_PCLK4_FREQUENCY     (STM32_HCLK_FREQUENCY/2)       /* PCLK4 = HCLK / 2 */

/* Timers driven from APB1 will be twice PCLK1 */
#define STM32_APB1_TIM2_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM3_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM4_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM5_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM6_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM7_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM12_CLKIN  (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM13_CLKIN  (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM14_CLKIN  (2*STM32_PCLK1_FREQUENCY)

/* Timers driven from APB2 will be twice PCLK2 */
#define STM32_APB2_TIM1_CLKIN   (2*STM32_PCLK2_FREQUENCY)
#define STM32_APB2_TIM8_CLKIN   (2*STM32_PCLK2_FREQUENCY)
#define STM32_APB2_TIM15_CLKIN  (2*STM32_PCLK2_FREQUENCY)
#define STM32_APB2_TIM16_CLKIN  (2*STM32_PCLK2_FREQUENCY)
#define STM32_APB2_TIM17_CLKIN  (2*STM32_PCLK2_FREQUENCY)

#define GPIO_nLED_RED_PORT   (GPIOE)
#define GPIO_nLED_RED_PIN    (3)

#define GPIO_nLED_GREEN_PORT   (GPIOE)
#define GPIO_nLED_GREEN_PIN    (4)

#define GPIO_nLED_BLUE_PORT   (GPIOE)
#define GPIO_nLED_BLUE_PIN    (5)

#define GPIO_nPOWER_IN_A_PORT   (GPIOG)
#define GPIO_nPOWER_IN_A_PIN    (1)

#define GPIO_nPOWER_IN_B_PORT   (GPIOG)
#define GPIO_nPOWER_IN_B_PIN    (2)

#define GPIO_nPOWER_IN_C_PORT   (GPIOG)
#define GPIO_nPOWER_IN_C_PIN    (3)

#define GPIO_VDD_5V_PERIPH_nEN_PORT  (GPIOG)
#define GPIO_VDD_5V_PERIPH_nEN_PIN  (4)

#define GPIO_VDD_5V_PERIPH_nOC_PORT  (GPIOE)
#define GPIO_VDD_5V_PERIPH_nOC_PIN  (15)

#define GPIO_VDD_5V_HIPOWER_nEN_PORT  (GPIOG)
#define GPIO_VDD_5V_HIPOWER_nEN_PIN  (10)

#define GPIO_VDD_5V_HIPOWER_nOC_PORT  (GPIOF)
#define GPIO_VDD_5V_HIPOWER_nOC_PIN  (13)

#define GPIO_VDD_3V3_SENSORS_EN_PORT  (GPIOG)
#define GPIO_VDD_3V3_SENSORS_EN_PIN  (8)

#define GPIO_VDD_3V3_SPEKTRUM_POWER_EN_PORT  (GPIOH)
#define GPIO_VDD_3V3_SPEKTRUM_POWER_EN_PIN  (2)

#define GPIO_VDD_3V3_SD_CARD_EN_PORT      (GPIOC)
#define GPIO_VDD_3V3_SD_CARD_EN_PIN       (13)

#define GPIO_ETH_POWER_EN_PORT (GPIOG)
#define GPIO_ETH_POWER_EN_PIN  (15)

#define GPIO_OTGFS_VBUS_PORT           (GPIOA)
#define GPIO_OTGFS_VBUS_PIN           (9)

#define VDD_5V_PERIPH_EN(on_true)          LOW_IOSET(GPIO_VDD_5V_PERIPH_nEN_PORT, GPIO_VDD_5V_PERIPH_nEN_PIN, !(on_true))
#define VDD_5V_HIPOWER_EN(on_true)         LOW_IOSET(GPIO_VDD_5V_HIPOWER_nEN_PORT, GPIO_VDD_5V_HIPOWER_nEN_PIN, !(on_true))
#define VDD_3V3_SENSORS_EN(on_true)        LOW_IOSET(GPIO_VDD_3V3_SENSORS_EN_PORT, GPIO_VDD_3V3_SENSORS_EN_PIN, (on_true))
#define VDD_3V3_SPEKTRUM_POWER_EN(on_true) LOW_IOSET(GPIO_VDD_3V3_SPEKTRUM_POWER_EN_PORT, GPIO_VDD_3V3_SPEKTRUM_POWER_EN_PIN, (on_true))
#define READ_VDD_3V3_SPEKTRUM_POWER_EN()   LOW_IOGET(GPIO_VDD_3V3_SPEKTRUM_POWER_EN, GPIO_VDD_3V3_SPEKTRUM_POWER_EN_PIN)
#define VDD_3V3_SD_CARD_EN(on_true)        LOW_IOSET(GPIO_VDD_3V3_SD_CARD_EN_PORT, GPIO_VDD_3V3_SD_CARD_EN_PIN, (on_true))
#define VDD_3V3_ETH_POWER_EN(on_true)      LOW_IOSET(GPIO_ETH_POWER_EN_PORT, GPIO_ETH_POWER_EN_PIN, (on_true))

#define SPEKTRUM_POWER(_on_true)           VDD_3V3_SPEKTRUM_POWER_EN(_on_true)

#define BOARD_ADC_USB_CONNECTED (LOW_IOGET(GPIO_OTGFS_VBUS_PORT, GPIO_OTGFS_VBUS_PIN))
#define BOARD_ADC_USB_VALID     (!LOW_IOGET(GPIO_nPOWER_IN_C_PORT, GPIO_nPOWER_IN_C_PIN))
#define BOARD_ADC_PERIPH_5V_OC  (!LOW_IOGET(GPIO_VDD_5V_PERIPH_nOC_PORT, GPIO_VDD_5V_PERIPH_nOC_PIN))
#define BOARD_ADC_HIPOWER_5V_OC (!LOW_IOGET(GPIO_VDD_5V_HIPOWER_nOC_PORT, GPIO_VDD_5V_HIPOWER_nOC_PIN))

#define BOARD_BLUE_LED(on_true)       LOW_IOSET(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN, !(on_true))
#define BOARD_RED_LED(on_true)        LOW_IOSET(GPIO_nLED_RED_PORT, GPIO_nLED_RED_PIN, !(on_true))
#define BOARD_GREEN_LED(on_true)      LOW_IOSET(GPIO_nLED_GREEN_PORT, GPIO_nLED_GREEN_PIN, !(on_true))

#ifdef __cplusplus
extern "C" {
#endif

void board_init();
void board_deinit();
void board_bsp_init();
void board_bsp_deinit();

void board_reboot();
void board_get_uid(uint32_t *p);
uint32_t board_get_time();
void board_delay(uint32_t ms);
uint32_t board_elapsed_time(const uint32_t timestamp);
/*-------------- board bsp interface --------------*/

void board_test();
void board_led_toggle(uint8_t idx);

int  board_stream_in(int port, void *p, int size);
int  board_stream_out(int port, const void *p, int size, int way);
void board_stream_printf(int port, const char *format, ...);
void board_printf(const char *format, ...);

bool board_rtc_set_timestamp(rclk_time_t now);
rclk_time_t board_rtc_get_timestamp(struct rclk_timeval *now);

#ifdef __cplusplus
}
#endif

#endif
