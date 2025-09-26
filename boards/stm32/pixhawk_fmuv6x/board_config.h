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
 * bug 2: porting freertos v10.2.1
 *       enter hardfault in prvInitialiseNewTask() (called by xTaskCreate and static )
 *       change configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5 -> 9????
 */
#include <stm32h7xx_hal.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
    extern "C" {
#endif


#ifdef __cplusplus
}
#endif



#endif
