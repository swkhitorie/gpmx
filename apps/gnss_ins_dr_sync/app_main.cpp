#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

#include <drivers/drv_hrt.h>
#include <device/serial.h>
#include <rtkcmn.h>

#include "mpu6050.h"

#include "gnss_pps_sync.h"
#include "rtcm_rcver.h"

#include "canspeed_obd.h"

#include "udp_echo.h"
#include "udp_transfer.h"
#include "lwip/timeouts.h"

/** RTCM Parse Process */
static uart_dev_t *rtkin;
static uint8_t rtkc;

/** Sensor IMU Data */
static float imu_ins[6];

/** rtcm buffer for GNSS Module */
static uint8_t buff_rtcmrcv[1024];
static uint16_t buff_rtcmrcvlen = 0;

/** rtcm buffer for IMU */
static uint8_t buff_imu[64];
static uint16_t buff_imulen = 0;

/** rtcm buffer for VehicleSpeed */
static uint8_t buff_speed[64];
static uint16_t buff_speedlen = 0;

/** Debug */
static uint8_t buff_eth[512];

/** OBD Data */
static uint32_t speed_obd = 0;

void gnss_pps_irq(void *p) 
{
    pps_pulse_irq();

    printf("[%lu] pps pulse come\r\n", HAL_GetTick());
}

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();

    rtkin = serial_bus_get(3);

    if (0 == mpu6050_init(1)) {
        printf("[imu] mpu init success \r\n");
    }

    // udp_echo_demo_init();
    udp_transfer_init();

    obd_bus_init();

    uint32_t m1 = HAL_GetTick();
    uint32_t m2 = HAL_GetTick();
    uint32_t m3 = HAL_GetTick();

    for (;;) {

        obd_rx_speed_detect();

        int sz = SERIAL_RDBUF(rtkin, &rtkc, 1);
        if (sz > 0) {
            if (rtcm_rcv_process(rtkc, buff_rtcmrcv, &buff_rtcmrcvlen) == 1) {
                udp_transfer_raw(buff_rtcmrcv, buff_rtcmrcvlen);
            }
        }

        if (HAL_GetTick() - m2 >= 100) {
            m2 = HAL_GetTick();
            mpu6050_read(imu_ins);

            rtcm_imu_t rimu;
            struct timeval tv;
            rimu.now = board_rtc_get_timeval(&tv);
            rimu.subsec = hrt_absolute_time()%1000000;
            rimu.accx = (int32_t)(imu_ins[0]*1e3f);
            rimu.accy = (int32_t)(imu_ins[1]*1e3f);
            rimu.accz = (int32_t)(imu_ins[2]*1e3f);
            rimu.gyrox = (int32_t)(imu_ins[3]*1e3f);
            rimu.gyroy = (int32_t)(imu_ins[4]*1e3f);
            rimu.gyroz = (int32_t)(imu_ins[5]*1e3f);
            buff_imulen = rtcm_imu_encode(buff_imu, &rimu);
            udp_transfer_raw(buff_imu, buff_imulen);

            // printf("[imu] %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\r\n",
            //     imu_ins[0], imu_ins[1], imu_ins[2], 
            //     imu_ins[3], imu_ins[4], imu_ins[5]);

        }

        if (HAL_GetTick() - m3 >= 25) {
            m3 = HAL_GetTick();
            obd_request_speed();
            speed_obd = obd_read_speed();

            rtcm_speed_t rspeed;
            struct timeval tv;
            rspeed.now = board_rtc_get_timeval(&tv);
            rspeed.subsec = hrt_absolute_time()%1000000;
            rspeed.vehicle_speed = speed_obd;
            buff_speedlen = rtcm_speed_encode(buff_speed, &rspeed);
            udp_transfer_raw(buff_speed, buff_speedlen);

            // printf("[obd] speed:%d\r\n", speed_obd);
        }

        if (HAL_GetTick() - m1 >= 500) {
            m1 = HAL_GetTick();
            board_led_toggle(1);
        }

        // lwip timeout task core
        sys_check_timeouts();
    }
}

