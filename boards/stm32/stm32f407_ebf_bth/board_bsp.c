#include <board_config.h>
#include <drv_uart.h>
#include <drv_i2c.h>
#include <drv_rtc.h>
#include <drv_can.h>
#include <device/dnode.h>
#include <device/serial.h>
#include <device/i2c_master.h>
#include <device/can.h>

/* COM1 */
uint8_t com1_dma_rxbuff[128];
uint8_t com1_dma_txbuff[512];
uint8_t com1_txbuff[512];
uint8_t com1_rxbuff[128];
struct up_uart_dev_s com1_dev = {
    .dev = {
        .baudrate = 115200,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 128,
            .buffer = com1_rxbuff,
        },
        .xmit = {
            .capacity = 512,
            .buffer = com1_txbuff,
        },
        .dmarx = {
            .capacity = 128,
            .buffer = com1_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 512,
            .buffer = com1_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com1_dev,
    },
    .id = 1, //usart1
    .pin_tx = 0, //PA9
    .pin_rx = 0, //PA10
    .priority = 3,
    .priority_dmarx = 3,
    .priority_dmatx = 2,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

/* COM3 */
uint8_t com3_dma_rxbuff[1024*4];
uint8_t com3_dma_txbuff[256];
uint8_t com3_txbuff[256];
uint8_t com3_rxbuff[1024*4];
struct up_uart_dev_s com3_dev = {
    .dev = {
        .baudrate = 460800,
        .wordlen = 8,
        .stopbitlen = 1,
        .parity = 'n',
        .recv = {
            .capacity = 1024*4,
            .buffer = com3_rxbuff,
        },
        .xmit = {
            .capacity = 256,
            .buffer = com3_txbuff,
        },
        .dmarx = {
            .capacity = 1024*4,
            .buffer = com3_dma_rxbuff,
        },
        .dmatx = {
            .capacity = 256,
            .buffer = com3_dma_txbuff,
        },
        .ops       = &g_uart_ops,
        .priv      = &com3_dev,
    },
    .id = 3, //usart3
    .pin_tx = 0, //PB10
    .pin_rx = 0, //PB11
    .priority = 1,
    .priority_dmarx = 1,
    .priority_dmatx = 5,
    .enable_dmarx = true,
    .enable_dmatx = true,
};

/**************
 * Sensor I2C --- MPU6050
 **************/
struct up_i2c_master_s i2c1_dev = 
{
    .dev = {
        .cfg = {
            .address = 0x00,
            .addrlen = 7,
            .frequency = 100000,
        },
        .ops       = &g_i2c_master_ops,
        .priv      = &i2c1_dev,
    },
	.id = 1, //i2c1
	.pin_scl = 1, //PB8
    .pin_sda = 0, //PB9
    .priority_event = 4,
    .priority_error = 5,
};

struct up_can_dev_s can2_dev = {
    .dev = {
        .cd_status = 0,
        .cd_baud = 1000000,
        .cd_mode = {
            .bm_loopback = 0,
            .bm_silent = 0,
        },
        .cd_ops = &g_can_master_ops,
        .cd_priv = &can2_dev,
    },
    .id = 2,
    .pin_tx = 1,
    .pin_rx = 0,
    .priority = 1,
};

uart_dev_t *dstdout;
uart_dev_t *dstdin;

void board_bsp_init()
{
    LOW_INITPIN(GPIOF, 6, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_INITPIN(GPIOF, 7, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);
    LOW_INITPIN(GPIOF, 8, IOMODE_OUTPP, IO_NOPULL, IO_SPEEDHIGH);

    LOW_IOSET(GPIOF, 6, 1);
    LOW_IOSET(GPIOF, 7, 1);
    LOW_IOSET(GPIOF, 8, 1);

    serial_register(&com1_dev.dev, 1);
    serial_register(&com3_dev.dev, 3);

    i2c_register(&i2c1_dev.dev, 1);

    can_register(&can2_dev.dev, 2);

    serial_bus_initialize(1);
    serial_bus_initialize(3);

    dstdout = serial_bus_get(1);
    dstdin = serial_bus_get(1);

    i2c_bus_initialize(1);

    stm32_rtc_setup();

    stm32_gpiosetevent(GET_PINHAL(GPIOC, 10), true, false, true, gnss_pps_irq, NULL, 4);

    hw_stm32_eth_init();

    can_bus_initialize(2);
    // can2_dev.dev.cd_ops->co_setup(&can2_dev.dev);
}

struct can_msg_s msg_test = {
    .cm_hdr = {
        .ch_dlc = 8,
        .ch_extid = 0,
        .ch_id = 0x12,
        .ch_rtr = 0,
    },
};

void board_debug()
{
    board_led_toggle(1);

    memset(&msg_test, 0, sizeof(struct can_msg_s));
    msg_test.cm_hdr.ch_dlc = 8;
    msg_test.cm_hdr.ch_id = 0x11;
    for (int i = 0; i < 8; i++) {
        msg_test.cm_data[i] = 40+i;
    }

    can2_dev.dev.cd_ops->co_send(&can2_dev.dev, &msg_test);

    printf("send msg\r\n");

    struct can_msg_s aamsg;
    if (can_rxfifo_get(&can2_dev.dev.cd_rxfifo, &aamsg) == DTRUE) {
        printf("rcv dlc:%d, rtr/dtr:%d, std/ext:%d, id:%x ",
            aamsg.cm_hdr.ch_dlc, aamsg.cm_hdr.ch_rtr, aamsg.cm_hdr.ch_extid,
            aamsg.cm_hdr.ch_id);
        for (int i = 0; i < 8; i++) {
            printf("%x ", aamsg.cm_data[i]);
        }
        printf("\r\n");
    }

    // struct tm now;
    // stm32_rtc_get_tm(&now);

    // printf("%d-%02d-%02d %02d:%02d:%02d \r\n",
    //     now.tm_year+1900, now.tm_mon+1, now.tm_mday, 
    //     now.tm_hour+8, now.tm_min, now.tm_sec);

}

void board_led_toggle(uint8_t idx)
{
    int val;
    switch (idx) {
    case 0:
        val = LOW_IOGET(GPIOF, 6);
        LOW_IOSET(GPIOF, 6, !val);
        LOW_IOSET(GPIOF, 7, 1);
        LOW_IOSET(GPIOF, 8, 1);
        break;
    case 1:
        val = LOW_IOGET(GPIOF, 7);
        LOW_IOSET(GPIOF, 7, !val);
        LOW_IOSET(GPIOF, 6, 1);
        LOW_IOSET(GPIOF, 8, 1);
        break;
    case 2:
        val = LOW_IOGET(GPIOF, 8);
        LOW_IOSET(GPIOF, 8, !val);
        LOW_IOSET(GPIOF, 6, 1);
        LOW_IOSET(GPIOF, 7, 1);
        break;
    }
}

time_t board_rtc_get_timeval(struct timeval *tv)
{
    return stm32_rtc_get_timeval(tv);
}

void board_rtc_get_tm(struct tm *now_i)
{
    stm32_rtc_get_tm(now_i);
}

bool board_rtc_set_time_stamp(time_t time_stamp)
{
    return stm32_rtc_set_time_stamp(time_stamp);
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

