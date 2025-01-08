#include "board_config.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
FILE __stdin, __stdout, __stderr;

static lld_uart_t com1;

void board_bsp_init()
{
    lld_uart_init(&com1,1,115200,1,1,true,true,
        UART_PARITY_NONE,
        UART_WORDLENGTH_8B,
        UART_STOPBITS_1);

#ifdef BSP_USB_CDC_ENABLE
    HAL_Delay(400);
    board_usb_init();
#endif

}

#ifdef BSP_COM_PRINTF
__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    if (file == stdout_fileno) {
        lld_uart_send_bytes(&com1, (const uint8_t *)ptr, len, 1);
    }
    return len;
}
#endif
