#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

#include <drivers/drv_hrt.h>
#include <device/serial.h>
#include <rtkcmn.h>

#include "gnss_pps_sync.h"
#include "rtcm_rcver.h"

#include "mpu6050.h"
#include "udp_echo.h"
#include "lwip/timeouts.h"

static uart_dev_t *rtkin;
static uint8_t rtkc;

static float ins[6];

void gnss_pps_irq(void *p) 
{
    pps_pulse_irq();
    // printf("[%lu] pps come\r\n", HAL_GetTick());
}

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();

    rtkin = serial_bus_get(3);

    // if (0 == mpu6050_init()) {
    //     printf("imu init success \r\n");
    // }

    // udp_echo_demo_init();

    uint32_t m = HAL_GetTick();
    for (;;) {

        // int sz = SERIAL_RDBUF(rtkin, &rtkc, 1);
        // if (sz > 0) {
        //     rtcm_rcv_process(rtkc);
        // }

        if (HAL_GetTick() - m >= 100) {
            m = HAL_GetTick();
            // mpu6050_read(ins);
            board_debug();
        }

        // sys_check_timeouts();
    }
}

