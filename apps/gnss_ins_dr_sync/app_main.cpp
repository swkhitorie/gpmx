#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

#include <drivers/drv_hrt.h>
#include <device/serial.h>
#include <rtkcmn.h>

#include "mpu6050.h"
#include "sch1633.h"

#include "gnss_pps_sync.h"
#include "rtcm_rcver.h"

#include "canspeed_obd.h"

#include "udp_echo.h"
#include "udp_transfer.h"
#include "lwip/timeouts.h"

#include "sens_sync_proto.h"

#ifndef UP_UDP_IP
#define UP_UDP_IP "192.168.75.34"
#endif

#ifndef UP_UDP_PORT
#define UP_UDP_PORT 5001
#endif

__attribute__((section(".ccmram"))) struct __sens_sync_proto sens_sync;

static uart_dev_t *rtk_rover_port;
static uart_dev_t *rtk_base_port;
static uint8_t rtk_rover_c;
static uint8_t rtk_base_c;
static uint8_t buff_rover[1024];
static uint16_t buff_roverlen = 0;
static uint8_t buff_base[1024];
static uint16_t buff_baselen = 0;

static float imu_ins[6];
static uint8_t buff_imu[64];
static uint16_t buff_imulen = 0;

static uint8_t buff_speed[64];
static uint16_t buff_speedlen = 0;

/** Debug */
static uint8_t buff_eth[512];

/** OBD Data */
static uint32_t speed_obd = 0;

static ip_addr_t up_addr;
static uint16_t up_port;

void gnss_pps_irq(void *p) 
{
    pps_pulse_irq();

    printf("[%lu] pps pulse come\r\n", HAL_GetTick());
}

SCH1_raw_data       SCH1_data;
SCH1_filter         SCH1_Filter;
SCH1_sensitivity    SCH1_Sensitivity;
SCH1_decimation     SCH1_Decimation;
SCH1_result         SCH1_ret_data;
SCH1_status         SCH1_Status;
void imu_init()
{
    char serial_num[15];
    int  init_status;

    if (0 == mpu6050_init(1)) {
        printf("[imu] mpu init success \r\n");
    }

    hw_CS_High();
    hw_init();

    // SCH1600 filter settings
    SCH1_Filter.Rate12 = FILTER_RATE;
    SCH1_Filter.Acc12  = FILTER_ACC12;
    SCH1_Filter.Acc3   = FILTER_ACC3;

    // SCH1600 sensitivity settings
    SCH1_Sensitivity.Rate1 = SENSITIVITY_RATE1;
    SCH1_Sensitivity.Rate2 = SENSITIVITY_RATE2;
    SCH1_Sensitivity.Acc1  = SENSITIVITY_ACC1;
    SCH1_Sensitivity.Acc2  = SENSITIVITY_ACC2;
    SCH1_Sensitivity.Acc3  = SENSITIVITY_ACC3;

    // SCH1600 decimation settings (for Rate2 and Acc2 channels).
    SCH1_Decimation.Rate2 = DECIMATION_RATE;
    SCH1_Decimation.Acc2  = DECIMATION_ACC;

    init_status = SCH1_init(SCH1_Filter, SCH1_Sensitivity, SCH1_Decimation, false);
    if (init_status != SCH1_OK) {
        printf("[IMU] ERROR: SCH1_init failed with code: %d\r\nApplication halted\r\n", init_status);
        HAL_Delay(10);
        while (true);
    }

    // Read serial number from the sensor.
    strcpy(serial_num, SCH1_getSnbr());
    printf("[IMU] SCH1633 Serial number: %s\r\n\r\n", serial_num);
}

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();

    rtk_base_port = serial_bus_get(2);
    rtk_rover_port = serial_bus_get(3);

    imu_init();

    obd_bus_init();

    udp_transfer_init();

    up_addr.addr = inet_addr(UP_UDP_IP);
    up_port = UP_UDP_PORT;
    udp_set_target_ip_port(up_addr, up_port);

    uint32_t m1 = HAL_GetTick();
    uint32_t m2 = HAL_GetTick();
    uint32_t m3 = HAL_GetTick();

    for (;;) {

        udp_request();

        /** OBD Speed */
        obd_rx_speed_detect();

        /** Rover Handle */
        int sz = SERIAL_RDBUF(rtk_rover_port, &rtk_rover_c, 1);
        if (sz > 0) {
            if (rtcm_rover_process(rtk_rover_c, buff_rover, &buff_roverlen) == 1) {
                int rover_len = sens_sync_encode_rover(&sens_sync, (const uint8_t *)&buff_rover[0], buff_roverlen);
                udp_transfer_raw_control(sens_sync.buff, rover_len);
            }
        }

        /** Base Handle */
        int sz2 = SERIAL_RDBUF(rtk_base_port, &rtk_base_c, 1);
        if (sz2 > 0) {
            if (rtcm_base_process(rtk_base_c, buff_base, &buff_baselen) == 1) {
                int base_len = sens_sync_encode_rover(&sens_sync, (const uint8_t *)&buff_base[0], buff_baselen);
                udp_transfer_raw_control(sens_sync.buff, base_len);
            }
        }

        if (HAL_GetTick() - m2 >= 20) {
            m2 = HAL_GetTick();

            // mpu6050_read(imu_ins);

            SCH1_getData(&SCH1_data);
            SCH1_convert_data(&SCH1_data, &SCH1_ret_data);
            SCH1_getStatus(&SCH1_Status);

            struct timeval tv;
            int imu_len = sens_sync_encode_imu(&sens_sync, 
                board_rtc_get_timeval(&tv), 
                hrt_absolute_time()%1000000,
                SENS_IMU_ID_SCH1633,
                (int32_t)(SCH1_ret_data.Acc1[AXIS_X]*1e3f),
                (int32_t)(SCH1_ret_data.Acc1[AXIS_Y]*1e3f),
                (int32_t)(SCH1_ret_data.Acc1[AXIS_Z]*1e3f),
                (int32_t)(SCH1_ret_data.Rate1[AXIS_X]*1e3f),
                (int32_t)(SCH1_ret_data.Rate1[AXIS_Y]*1e3f),
                (int32_t)(SCH1_ret_data.Rate1[AXIS_Z]*1e3f));

            udp_transfer_raw_control(sens_sync.buff, imu_len);

            // printf("[imu] %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\r\n",
            //     imu_ins[0], imu_ins[1], imu_ins[2], 
            //     imu_ins[3], imu_ins[4], imu_ins[5]);

            // printf("$IMURAW,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\r\n",
            //     SCH1_ret_data.Rate1[AXIS_X], SCH1_ret_data.Rate1[AXIS_Y], SCH1_ret_data.Rate1[AXIS_Z],
            //     SCH1_ret_data.Acc1[AXIS_X], SCH1_ret_data.Acc1[AXIS_Y], SCH1_ret_data.Acc1[AXIS_Z],
			// 	HAL_GetTick()/1e3f);
        }

        if (HAL_GetTick() - m3 >= 25) {
            m3 = HAL_GetTick();
            obd_request_speed();
            speed_obd = obd_read_speed();

            struct timeval tv;

            int spd_len = sens_sync_encode_wheelspd(&sens_sync, 
                board_rtc_get_timeval(&tv), 
                hrt_absolute_time()%1000000,
                speed_obd);

            udp_transfer_raw_control(sens_sync.buff, spd_len);

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

