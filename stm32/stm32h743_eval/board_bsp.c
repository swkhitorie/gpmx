#include "board_config.h"
#include <drv_uart.h>
#include <drv_qspi.h>
#include <drv_sdmmc.h>

#ifdef BSP_COM_PRINTF
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
FILE __stdin, __stdout, __stderr;
#endif

struct drv_uart_t com1;
uint8_t com1_dma_rxbuff[256];
uint8_t com1_dma_txbuff[256];
uint8_t com1_txbuff[512];
uint8_t com1_rxbuff[512];


struct drv_sdmmc_attr_t sd_attr;
struct drv_sdmmc_t sd;
bool sd_card_check(char* info_str);


struct drv_pin_t led;

struct drv_pin_irq_t io1_irq;
struct drv_pin_t io1;
void io1_trigger() { printf("io1_trigger\r\n"); }

void board_bsp_init()
{
    struct drv_uart_attr_t com1_attr;
    struct drv_uart_dma_attr_t com1_rxdma_attr;
    struct drv_uart_dma_attr_t com1_txdma_attr;
    drv_uart_dma_attr_init(&com1_rxdma_attr, &com1_dma_rxbuff[0], 256, 2);
    drv_uart_dma_attr_init(&com1_txdma_attr, &com1_dma_txbuff[0], 256, 2);
    drv_uart_attr_init(&com1_attr, 115200, WL_8BIT, STB_1BIT, PARITY_NONE, 1);
    drv_uart_buff_init(&com1, &com1_txbuff[0], 512, &com1_rxbuff[0], 512);
    drv_uart_init(1, &com1, 1, 1, &com1_attr, &com1_txdma_attr, &com1_rxdma_attr);

    led = drv_gpio_init(GPIO_nLED_BLUE_PORT, GPIO_nLED_BLUE_PIN, IOMODE_OUTPP, 
                IO_SPEEDHIGH, IO_NOPULL, 0, NULL);
    drv_gpio_irq_init(&io1_irq, 6, io1_trigger);
    io1 = drv_gpio_init(GPIOB, 8, IOMODE_IT_RISING, IO_SPEEDHIGH, IO_PULLDOWN, 0, &io1_irq);

    drv_qspi_init(1, 2, 22, /* io select */ 1, 2, 2, 1, 2, 2);
    int ret1 = w25qxx_init();
    int id = w25qxx_readid();
    int addr = 0x100;
    printf("[Flash] qspi flash init: %d, id: %X\r\n", ret1, id);
    printf("[Flash] qspi flash simple rw test, addr: %X\r\n", addr);
    w25qxx_sector_erase(addr);
    uint8_t write_buf[128];
    uint8_t read_buf[128];
    sprintf((char *)&write_buf[0], "W25QXX TEST String \r");
    int res = w25qxx_writebuffer(&write_buf[0], addr, 60);
    res = w25qxx_readbuffer(&read_buf[0], addr, 60);
    printf("[Flash] qspi flash read : %s \r\n", read_buf);

    drv_sdmmc_attr_init(&sd_attr, 1, 8, /* io select */1, 1, 1, 1, 1, 1, /* priority */ 4);
    int sdret = drv_sdmmc_init(&sd, &sd_attr);
    printf("sdmmc init: %d \r\n", sdret);

    char sd_debug[300];
    sd_card_check(sd_debug);
    printf("%s \r\n", sd_debug);
#ifdef BSP_MODULE_USB_CHERRY
    cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
#endif
}

void board_blue_led_toggle()
{
    int val = drv_gpio_read(&led);
    drv_gpio_write(&led, !val);
}

uint8_t buff_debug[256];
void board_debug()
{
    devbuf_t buf = drv_uart_devbuf(&com1);
    int size = devbuf_size(&buf);
    if (size > 0) {
        printf("[%d,%d] ", HAL_GetTick(),size);
        devbuf_read(&com1.rx_buf, &buff_debug[0], size);
        for (int i = 0; i < size; i++) {
            printf("%d ", buff_debug[i]);
        }
        printf("\r\n");
    }
    board_blue_led_toggle();
}

#ifdef BSP_COM_PRINTF
int _write(int file, char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    if (file == stdout_fileno) {
        drv_uart_send(&com1, ptr, len, RWDMA);
    }
    return len;
}
#endif

bool sd_card_check(char* info_str)
{
    char str[300] = {0}, strtmp[100] = {0};
    uint8_t tmp;

    tmp = drv_sdmmc_wait_ready(&sd);
    if (tmp != HAL_OK) {
        sprintf(info_str,"not ready\r\n");
        return false;
    }
    drv_sdmmc_getinfo(&sd);

    switch (sd.initret) {
    case HAL_ERROR:
        sprintf(info_str,"sd init error\r\n");
        return false;
    case HAL_BUSY:
        sprintf(info_str,"sd init error\r\n");
        return false;
    case HAL_TIMEOUT:
        sprintf(info_str,"sd init timeout\r\n");
        return false;
    }

    switch (tmp) {
    case HAL_ERROR:
        sprintf(info_str,"wait error\r\n");
        return false;
    case HAL_BUSY:
        sprintf(info_str,"wait busy\r\n");
        return false;
    case HAL_TIMEOUT:
        sprintf(info_str,"wait timeout\r\n");
        return false;
    }
    
    sprintf(strtmp,"sd card ready\r\n");
    strcat(str, strtmp);
    float total_capacity = 0;
    total_capacity = (float)(sd.info.LogBlockNbr) / 1024.0f / 1024.0f / 1024.0f;

    sprintf(strtmp,"total capacity : %.2f GB\r\n",(total_capacity * sd.info.BlockSize));
    strcat(str, strtmp);
    sprintf(strtmp,"card block size : %d bytes\r\n",(sd.info.BlockSize));
    strcat(str, strtmp);
    
    switch (sd.info.CardSpeed) {
    case CARD_NORMAL_SPEED:
        sprintf(strtmp,"Normal Speed Card <12.5Mo/s , Spec Version 1.01 \r\n");
        break;
    case CARD_HIGH_SPEED:
        sprintf(strtmp,"High Speed Card <25Mo/s , Spec version 2.00 \r\n");
        break;
    case CARD_ULTRA_HIGH_SPEED:
        sprintf(strtmp,"UHS-I SD Card <50Mo/s for SDR50, DDR5 Cards and <104Mo/s for SDR104, Spec version 3.01 \r\n");
        break;	
    }
    strcat(str, strtmp);
    
    switch (sd.info.CardType) {
    case CARD_SDSC:
        sprintf(strtmp,"SD Standard Capacity <2Go \r\n");
        break;
    case CARD_SDHC_SDXC:
        sprintf(strtmp,"SD High Capacity <32Go, SD Extended Capacity <2To \r\n");
        break;
    case CARD_SECURED:
        sprintf(strtmp,"SD Extended Capacity >2To \r\n");
        break;
    }
    strcat(str, strtmp);
    
    switch (sd.info.CardVersion) {
    case CARD_V1_X:
        sprintf(strtmp,"card v1.x \r\n");
        break;
    case CARD_V2_X:
        sprintf(strtmp,"card v2.x \r\n");
        break;
    }
    strcat(str, strtmp);
    
    strcpy(info_str,str);
    return true;
}
