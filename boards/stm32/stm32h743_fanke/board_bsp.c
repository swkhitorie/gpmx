#include "board_config.h"
#include <drv_uart.h>
#include <device/dnode.h>
#include <device/serial.h>

/* COM1 */
uint8_t com1_dma_rxbuff[4096];
uint8_t com1_dma_txbuff[256];
uint8_t com1_txbuff[512];
uint8_t com1_rxbuff[2048];
struct up_uart_dev_s com1_dev = {
    .dev = {
        .baudrate = 115200,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 2048,
            .buffer = com1_rxbuff,
        },
        .xmit = {
            .capacity = 512,
            .buffer = com1_txbuff,
        },
        .dmarx = {
            .capacity = 4096,
            .buffer = com1_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 256,
            .buffer = com1_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com1_dev,
    },
    .id = 1,
    .pin_tx = 1,
    .pin_rx = 1,
    .priority = 1,
    .priority_dmarx = 2,
    .priority_dmatx = 3,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

uart_dev_t *dstdout;
uart_dev_t *dstdin;

void board_bsp_init()
{
    __HAL_RCC_GPIOH_CLK_ENABLE();

    LOW_INITPIN(GPIOH, 7, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);

    serial_register(&com1_dev.dev, 1);

    serial_bus_initialize(1);

    dstdout = serial_bus_get(1);
    dstdin = serial_bus_get(1);

#ifdef CONFIG_BOARD_MTD_QSPIFLASH_ENABLE
    board_mtd_init();
#ifdef CONFIG_BOARD_MTD_QSPIFLASH_RAW_RW_TEST
    board_mtd_rw_test();
#endif
#endif

#ifdef CONFIG_BOARD_MMCSD_ENABLE
    board_mmcsd_init();
#ifdef CONFIG_BOARD_MMCSD_RAW_RW_TEST
    board_mmcsd_rw_test();
#endif
#endif

#ifdef CONFIG_BOARD_CRUSB_CDC_ACM_ENABLE
    HAL_Delay(300);
    cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
    HAL_Delay(100);  // wait cdc init completed
#endif

    stm32_rtc_setup();
}

void board_led_toggle(uint8_t idx)
{
    switch (idx) {
    case 0: {
            int val = LOW_IOGET(GPIOH, 7);
            LOW_IOSET(GPIOH, 7, !val);
        }
        break;
    }
}

void board_debug()
{
    board_led_toggle(0);
}

void board_rtc_setup()
{
    low_rtc_setup();
}

time_t board_rtc_get_timestamp(struct timeval *now)
{
    return low_rtc_get_timeval(now);
}

bool board_rtc_set_timestamp(time_t now)
{
    return low_rtc_set_time_stamp(now);
}

#ifdef CONFIG_BOARD_COM_STDINOUT
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
FILE __stdin, __stdout, __stderr;
size_t fwrite(const void *ptr, size_t size, size_t n_items, FILE *stream)
{
    return _write(stream->_file, ptr, size*n_items);
}
int _write(int file, char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    if (file == stdout_fileno) {
#ifdef CONFIG_BOARD_COM_STDOUT_DMA
        SERIAL_DMASEND(dstdout, ptr, len);
#else
        SERIAL_SEND(dstdout, ptr, len);
#endif
    }
    return len;
}
size_t fread(void *ptr, size_t size, size_t n_items, FILE *stream)
{
    return _read(stream->_file, ptr, size*n_items);
}
// nonblock
int _read(int file, char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    int rsize = 0;
    if (file == stdin_fileno) {
        rsize = SERIAL_RDBUF(dstdin, ptr, len);
    }
    return rsize;
}
#endif

#ifdef CONFIG_BOARD_HRT_TIMEBASE
#include <drivers/drv_hrt.h>
hrt_abstime hrt_absolute_time(void)
{
    // uint64_t m0 = HAL_GetTick();
    // volatile uint64_t u0 = SysTick->VAL;
    // const uint64_t tms = SysTick->LOAD + 1;
    // volatile uint64_t abs_time = (m0 * 1000 + ((tms - u0) * 1000) / tms);
    // return abs_time;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint32_t m = HAL_GetTick();
    volatile uint32_t v = SysTick->VAL;
    // If an overflow happened since we disabled irqs, it cannot have been
    // processed yet, so increment m and reload VAL to ensure we get the
    // post-overflow value.
    if (SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) {
        ++m;
        v = SysTick->VAL;
    }

    // Restore irq status
    __set_PRIMASK(primask);

    const uint32_t tms = SysTick->LOAD + 1;
    return (m * 1000 + ((tms - v) * 1000) / tms);
}
#endif
