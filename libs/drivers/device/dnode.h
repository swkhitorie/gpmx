#ifndef DEV_OPS_NODE_H_
#define DEV_OPS_NODE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if defined (__CC_ARM)
#include "cmsis_armcc.h"
#elif defined(__clang__) && (__GNUC__)
#include "cmsis_armclang.h"
#elif defined(__GNUC__)
#include "cmsis_gcc.h"
#endif

#define gpdrv_irq_disable()                 __disable_irq()
#define gpdrv_irq_enable()                  __enable_irq()

#ifndef dn_time
#include <board_config.h>
/** return cpu running time units in ms */
#define dn_time()      HAL_GetTick()
#endif

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <portmacro.h>

#define gpdrv_isisr()                       xPortIsInsideInterrupt()
#define gpdrv_enter_critical_section()      ulPortRaiseBASEPRI()
#define gpdrv_leave_critical_section(x)     vPortSetBASEPRI(x)
#define gpdrv_time()       (xTaskGetTickCount()*(1000/configTICK_RATE_HZ))
#else
#define gpdrv_isisr()
#define gpdrv_enter_critical_section()      gpdrv_irq_disable()
#define gpdrv_leave_critical_section(x)     gpdrv_irq_enable()
#define gpdrv_time()                        dn_time()
#endif

#include <stdio.h>
#ifndef drvlog_d
#define drvlog_d(...)
#endif
#ifndef drvlog_w
#define drvlog_w(...)
#endif
#ifndef drvlog_e
#define drvlog_e(...)
#endif

#include <string.h>
#define gmemcpy(dst,src,len)   memcpy(dst,src,len)

#define GOK             (0)

#define _IOC(type,nr)   ((type)|(nr))

#define _TIOCBASE       (0x0100) /* Terminal I/O ioctl commands */
#define _WDIOCBASE      (0x0200) /* Watchdog driver ioctl commands */
#define _FIOCBASE       (0x0300) /* File system ioctl commands */
#define _DIOCBASE       (0x0400) /* Character driver ioctl commands */
#define _BIOCBASE       (0x0500) /* Block driver ioctl commands */
#define _MTDIOCBASE     (0x0600) /* MTD ioctl commands */
#define _SIOCBASE       (0x0700) /* Socket ioctl commands */
#define _ARPIOCBASE     (0x0800) /* ARP ioctl commands */
#define _TSIOCBASE      (0x0900) /* Touchscreen ioctl commands */
#define _SNIOCBASE      (0x0a00) /* Sensor ioctl commands */
#define _ANIOCBASE      (0x0b00) /* Analog (DAC/ADC) ioctl commands */
#define _PWMIOCBASE     (0x0c00) /* PWM ioctl commands */
#define _CAIOCBASE      (0x0d00) /* CDC/ACM ioctl commands */
#define _BATIOCBASE     (0x0e00) /* Battery driver ioctl commands */
#define _QEIOCBASE      (0x0f00) /* Quadrature encoder ioctl commands */
#define _AUDIOIOCBASE   (0x1000) /* Audio ioctl commands */
#define _LCDIOCBASE     (0x1100) /* LCD character driver ioctl commands */
#define _SLCDIOCBASE    (0x1200) /* Segment LCD ioctl commands */
#define _WLIOCBASE      (0x1300) /* Wireless modules ioctl network commands */
#define _WLCIOCBASE     (0x1400) /* Wireless modules ioctl character driver commands */
#define _CFGDIOCBASE    (0x1500) /* Config Data device (app config) ioctl commands */
#define _TCIOCBASE      (0x1600) /* Timer ioctl commands */
#define _JOYBASE        (0x1700) /* Joystick ioctl commands */
#define _PIPEBASE       (0x1800) /* FIFO/pipe ioctl commands */
#define _RTCBASE        (0x1900) /* RTC ioctl commands */
#define _RELAYBASE      (0x1a00) /* Relay devices ioctl commands */
#define _CANBASE        (0x1b00) /* CAN ioctl commands */
#define _BTNBASE        (0x1c00) /* Button ioctl commands */
#define _ULEDBASE       (0x1d00) /* User LED ioctl commands */
#define _ZCBASE         (0x1e00) /* Zero Cross ioctl commands */
#define _LOOPBASE       (0x1f00) /* Loop device commands */
#define _MODEMBASE      (0x2000) /* Modem ioctl commands */
#define _I2CBASE        (0x2100) /* I2C driver commands */
#define _SPIBASE        (0x2200) /* SPI driver commands */
#define _GPIOBASE       (0x2300) /* GPIO driver commands */
#define _CLIOCBASE      (0x2400) /* Contactless modules ioctl commands */
#define _USBCBASE       (0x2500) /* USB-C controller ioctl commands */
#define _MAC802154BASE  (0x2600) /* 802.15.4 MAC ioctl commands */
#define _PWRBASE        (0x2700) /* Power-related ioctl commands */
#define _FBIOCBASE      (0x2800) /* Frame buffer character driver ioctl commands */
#define _NXTERMBASE     (0x2900) /* NxTerm character driver ioctl commands */
#define _RFIOCBASE      (0x2a00) /* RF devices ioctl commands */
#define _RPTUNBASE      (0x2b00) /* Remote processor tunnel ioctl commands */

#define _CANIOC(nr)       _IOC(_CANBASE,nr)

struct dnode {
    void    *_devops;
    char  _devname[16];
};

#if defined(__cplusplus)
extern "C"{
#endif

bool     dn_register(const char *name, void *dev);
void    *dn_bind(const char *name);

#if defined(__cplusplus)
}
#endif

#endif
