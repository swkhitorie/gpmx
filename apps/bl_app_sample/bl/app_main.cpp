#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

#include "ymdm_rcv.h"
#include <device/serial.h>
#include <drv_flash.h>

static uint32_t boot_app_addr = 0x08020000;
static uint32_t boot_app_addr_wd = boot_app_addr;

struct __ymodem_receiver yrv;
int total_ysize_cal = 0;
int total_ysize = 0;

struct uart_dev_s *bus;
uint8_t bf_bl1[2048];

static void write_appbin(uint32_t appAddr, uint8_t *appbuf, uint32_t appsize)
{
    static uint32_t iapbuf[512];
	uint8_t *byteStream = appbuf;
	uint32_t wordCount = 0;
	uint32_t i = 0;
	uint32_t writeword;
	uint32_t fileWriteAddr = appAddr;

	for (i = 0; i < appsize; i += 4) {
		writeword = (uint32_t)byteStream[3] << 24;   
		writeword |= (uint32_t)byteStream[2] << 16;    
		writeword |= (uint32_t)byteStream[1] << 8;
		writeword |= (uint32_t)byteStream[0];
		byteStream += 4;
		
		iapbuf[wordCount++] = writeword;
		if (wordCount == 8) {
			wordCount = 0;
			stm32_flash_write(fileWriteAddr,iapbuf,8);
			fileWriteAddr += (8*4);
		}
	}

	if (wordCount) {
		stm32_flash_write(fileWriteAddr,iapbuf,wordCount);
	}
}

static bool check_appbin(uint32_t appAddr, uint8_t *appbuf, uint32_t appsize)
{
    static int check_error_cnt = 0;
	uint32_t word_value = 0;
	uint32_t read_flash_size = appsize / 4;
	uint32_t i = 0;

	bool tag = false;

	for (i = 0; i < read_flash_size; i++) {
		word_value = (uint32_t)appbuf[3] << 24;
		word_value |= (uint32_t)appbuf[2] << 16;
		word_value |= (uint32_t)appbuf[1] << 8;
		word_value |= (uint32_t)appbuf[0];
		appbuf += 4;
		if (word_value != stm32_flash_readword((appAddr + i * 4))) {
			check_error_cnt++;

            printf("appbin flash error addr: wordval: %x, flashval: %x, %x\r\n",
                word_value,
                (appAddr + i * 4), stm32_flash_readword((appAddr + i * 4)));
		}
	}

	if (check_error_cnt != 0)
		return false;
	else
		return true;
}

#define arch_setvtor(address) SCB->VTOR = (uint32_t)address;

/* Make the actual jump to app */
void
arch_do_jump(const uint32_t *app_base)
{
	/* extract the stack and entrypoint from the app vector table and go */
	uint32_t stacktop = app_base[0];
	uint32_t entrypoint = app_base[1];

	asm volatile(
		"msr msp, %0  \n"
		"bx %1  \n"
		: : "r"(stacktop), "r"(entrypoint) :);

	// just to keep noreturn happy
	for (;;) ;
}

static void exec_app()
{
	uint32_t i = 0;
	void (*AppJump)(void);

	__disable_irq(); 

	HAL_RCC_DeInit();

	SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

	for (i = 0; i < 8; i++) {
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}	
	__enable_irq();

    SCB->VTOR = 0x08020000;

	AppJump = (void (*)(void)) (*((volatile uint32_t *) (0x08020000 + 4)));
	__set_MSP(*(volatile uint32_t *)0x08020000);
	AppJump(); 

    arch_do_jump(boot_app_addr);

    printf("error fuck \r\n");
	while (1) {
	}

}

void ymodem_send(uint8_t c)
{
    SERIAL_SEND(bus, &c, 1);
}

void ymodem_receiver_callback(uint32_t seq, uint8_t *p, uint16_t size)
{
    bool ck_val = false;

    if (seq == 1) {
        total_ysize = ymodem_filesz(&yrv);
        printf("total rcv filesz: %d, ready to flash\r\n", total_ysize);
        stm32_flash_erase(boot_app_addr, 1024*256);
    }

    write_appbin(boot_app_addr_wd, p, size);

    ck_val = check_appbin(boot_app_addr_wd, p, size);

    if (!ck_val) {
        printf("error write flash \r\n");
        while(1) {}
    }

    boot_app_addr_wd += size;

    total_ysize_cal += size;
    printf("write rseq: %d, size: %d total: %d\r\n", seq, size, total_ysize_cal);
}

int main(int argc, char *argv[])
{
    int sz = 0;

    board_init();
    board_bsp_init();

    bus = serial_bus_get(1);

    ymodem_reset(&yrv);
    ymodem_cfg_snd_interface(&yrv, ymodem_send);
    ymodem_cfg_callback(&yrv, ymodem_receiver_callback);
    ymodem_start(&yrv);

    // printf("read to jump: %x \r\n", boot_app_addr);
    // HAL_Delay(500);
    // exec_app();
    // printf("un\r\n");

    uint32_t m = HAL_GetTick();
    for (;;) {

        if (HAL_GetTick() >= 2*1000 && (ymodem_state(&yrv) == YMODEM_SYNC_WAIT)) {
            exec_app();
        }

        if (HAL_GetTick() - m >= 100) {
            m = HAL_GetTick();

            sz = SERIAL_RDBUF(bus, bf_bl1, 1200);
            if (sz > 0 && sz != 11) {
                ymodem_rx_process(&yrv, bf_bl1, sz);
            } else {
                if (ymodem_state(&yrv) == YMODEM_SYNC_WAIT) {
                    ymodem_send(YMODEM_C);
                }
            }

            board_debug();
        }

        if (total_ysize_cal == total_ysize && ymodem_state(&yrv) == YMODEM_RCV_COMPLETED) {
            printf("jump \r\n");
            exec_app();
        }
    }
}

