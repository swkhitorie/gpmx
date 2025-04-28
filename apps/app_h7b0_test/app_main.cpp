#include <board_config.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <dev/serial.h>

#include "sh5001.hpp"
#include "rtcm3.h"
#include "rtkcmn.h"

SH5001 imu("/sensor_i2c");

uart_dev_t *gnss_module;
uint8_t gnss_rtcm_buff[512];

rtcm_ut msgf;

char output[256];
char check[5];
double tow_t = 0.0;

int main(int argc, char *argv[])
{
    board_init();
    hrt_init();

    gnss_module = (uart_dev_t *)dbind("/com7");
    int ret = imu.init();
    if (ret) {
        printf("[sh5001] init success\r\n");
    } else {
        printf("[sh5001] init failed\r\n");
    }

    int sz;
    uint32_t m = HAL_GetTick();
    for (;;) {

        sz = SERIAL_RDBUF(gnss_module, gnss_rtcm_buff, 512);
        if (sz > 0) {
            for (int i = 0; i < sz; i++) {
                ret = input_rtcm3(&msgf, gnss_rtcm_buff[i]);
                switch (ret) {
                case -2: break;
                case -1: break;
                case 0: break;
                default: 
                    tow_t = msgf.tow;
                    break;
                }
            }
        }

        if (HAL_GetTick() - m >= 10) {
            m = HAL_GetTick();

            imu.run();

            sprintf(output, "$IMURAW,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f*",
                imu.gyro_rad[0], imu.gyro_rad[1], imu.gyro_rad[2],
                imu.accel_m2_s[0], imu.accel_m2_s[1], imu.accel_m2_s[2],
                hrt_absolute_time()/1e6f,tow_t);
            uint8_t checksum = xor_sum_check((const uint8_t *)&output[0]);
            sprintf(check, "%02X\r\n",checksum);
            strcat(output, check);

            printf("%s", output);
        }
    }
}

