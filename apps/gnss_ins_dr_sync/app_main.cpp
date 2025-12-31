#include <board_config.h>
#include <stdio.h>
#include <stdint.h>

#include <drivers/drv_hrt.h>
#include <device/serial.h>
#include <rtkcmn.h>

#include "mpu6050.h"
#include "sch1633.h"
#include "asm330l.h"

#include "gnss_pps_sync.h"
#include "rtcm_rcver.h"

#include "canspeed_obd.h"

#include "udp_transfer.h"
#include "lwip/timeouts.h"

#include "sens_sync_proto.h"

uint32_t run_tag = 0;

#ifndef UP_UDP_IP
#define UP_UDP_IP "192.168.75.34"
#endif

#ifndef UP_UDP_PORT
#define UP_UDP_PORT 5001
#endif

__attribute__((section(".ccmram"))) struct __sens_sync_proto sens_sync;

static uart_dev_t *rtk_rover_port;
static uart_dev_t *rtk_base_port;
static uint8_t rtk_rover_c = 0;
static uint8_t rtk_base_c = 0;
__attribute__((section(".ccmram"))) static uint8_t buff_rover[1024];
static uint16_t buff_roverlen = 0;
__attribute__((section(".ccmram"))) static uint8_t buff_base[1024];
static uint16_t buff_baselen = 0;

static float imu_ins[6];
static uint8_t buff_imu[64];
static uint16_t buff_imulen = 0;
static uint32_t imu_seq = 0;

static uint8_t buff_speed[64];
static uint16_t buff_speedlen = 0;
static uint32_t spd_seq = 0;

/** Debug */
__attribute__((section(".ccmram"))) static uint8_t buff_eth[512];

/** OBD Data */
static int32_t speed_obd = 0;

static ip_addr_t up_addr;
static uint16_t up_port;

void gnss_pps_irq(void *p) 
{
    pps_pulse_irq();

    board_led_toggle(3);
}

#if defined(CONFIG_SPI_IMU_SCH1633)
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

    // if (0 == mpu6050_init(1)) {
    //     printf("[imu] mpu init success \r\n");
    // }

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
#endif
#if defined(CONFIG_SPI_IMU_ASM330LHH)
float ins[6];
#endif

struct timeval tv;
int main(int argc, char *argv[])
{
    int rbuf_sz = 0;

    board_init();
    board_bsp_init();

    rtk_base_port = serial_bus_get(2);
    rtk_rover_port = serial_bus_get(3);

#if defined(CONFIG_SPI_IMU_SCH1633)
    imu_init();
#elif defined(CONFIG_SPI_IMU_ASM330LHH)
    int rq1 = asm330l_init(1);
    if (rq1 != 0) {
        printf("asm330l init failed \r\n");
        while(1){}
    }
#endif

    obd_bus_init();

    udp_transfer_init();

    up_addr.addr = inet_addr(UP_UDP_IP);
    up_port = UP_UDP_PORT;
    udp_set_target_ip_port(up_addr, up_port);

    uint32_t m1 = HAL_GetTick();
    uint32_t m2 = HAL_GetTick();
    uint32_t m3 = HAL_GetTick();
    uint32_t m4 = HAL_GetTick();

    for (;;) {

        udp_request();

        run_tag = 0x0001;
        /** Rover Handle */
        int sz = SERIAL_RDBUF(rtk_rover_port, &rtk_rover_c, 1);
        if (sz > 0) {
            run_tag = 0x0002;
            if (rtcm_rover_process(rtk_rover_c, buff_rover, &buff_roverlen) == 1) {
                run_tag = 0x0003;
                int rover_len = sens_sync_encode_rover(&sens_sync, (const uint8_t *)&buff_rover[0], buff_roverlen);
                run_tag = 0x0004;
                udp_transfer_raw_control(sens_sync.buff, rover_len);
                run_tag = 0x0005;
                memset(sens_sync.buff, 0, SENS_SYNC_PROTO_PARSEBUFFERLEN);
            }
        }

        run_tag = 0x0011;
        /** Base Handle */
        int sz2 = SERIAL_RDBUF(rtk_base_port, &rtk_base_c, 1);
        if (sz2 > 0) {
            run_tag = 0x0012;
            if (rtcm_base_process(rtk_base_c, buff_base, &buff_baselen) == 1) {
                run_tag = 0x0013;
                int base_len = sens_sync_encode_base(&sens_sync, (const uint8_t *)&buff_base[0], buff_baselen);
                run_tag = 0x0014;
                udp_transfer_raw_control(sens_sync.buff, base_len);
                run_tag = 0x0015;
                memset(sens_sync.buff, 0, SENS_SYNC_PROTO_PARSEBUFFERLEN);
            }
        }

        run_tag = 0x0021;
        if (HAL_GetTick() - m2 >= 2) {
            m2 = HAL_GetTick();
#if defined(CONFIG_SPI_IMU_SCH1633)
            run_tag = 0x0022;
            SCH1_getData(&SCH1_data);
            SCH1_convert_data(&SCH1_data, &SCH1_ret_data);
            SCH1_getStatus(&SCH1_Status);

            run_tag = 0x0023;
            gnsspps_timestamp(&tv);
            run_tag = 0x0024;
            time_t timestamp = tv.tv_sec;
            uint32_t subsec = tv.tv_usec;
            imu_seq++;
            int imu_len = sens_sync_encode_imu(&sens_sync, 
                timestamp, 
                subsec,
                SENS_IMU_ID_SCH1633,
                SCH1_ret_data.Acc1[AXIS_X],
                SCH1_ret_data.Acc1[AXIS_Y],
                SCH1_ret_data.Acc1[AXIS_Z],
                SCH1_ret_data.Rate1[AXIS_X],
                SCH1_ret_data.Rate1[AXIS_Y],
                SCH1_ret_data.Rate1[AXIS_Z],
                imu_seq);

            printf("ins: %.3f, %.3f, %.3f, %.3f, %.3f, %.3f \r\n",
                SCH1_ret_data.Acc1[AXIS_X], SCH1_ret_data.Acc1[AXIS_Y], SCH1_ret_data.Acc1[AXIS_Z],
                SCH1_ret_data.Rate1[AXIS_X], SCH1_ret_data.Rate1[AXIS_Y], SCH1_ret_data.Rate1[AXIS_Z]);
#endif

#if defined(CONFIG_SPI_IMU_ASM330LHH)
            run_tag = 0x0022;
            asm330l_read(&ins[0]);

            run_tag = 0x0023;
            gnsspps_timestamp(&tv);
            run_tag = 0x0024;
            time_t timestamp = tv.tv_sec;
            uint32_t subsec = tv.tv_usec;
            imu_seq++;
            int imu_len = sens_sync_encode_imu(&sens_sync, 
                timestamp, 
                subsec,
                SENS_IMU_ID_SCH1633,
                ins[0]*9.7913f,
                ins[1]*9.7913f,
                ins[2]*9.7913f,
                ins[3]*3.1415926f/180.0f,
                ins[4]*3.1415926f/180.0f,
                ins[5]*3.1415926f/180.0f,
                imu_seq);

            // printf("ins: %.3f, %.3f, %.3f, %.3f, %.3f, %.3f \r\n",
            //     ins[0], ins[1], ins[2], ins[3], ins[4], ins[5]);
#endif
            run_tag = 0x0025;
            udp_transfer_raw_control(sens_sync.buff, imu_len);

            run_tag = 0x0026;
            memset(sens_sync.buff, 0, SENS_SYNC_PROTO_PARSEBUFFERLEN);
        }

        if (HAL_GetTick() - m3 >= 20) {
            m3 = HAL_GetTick();
            run_tag = 0x0030;   // bug tag: 0x30 -> can send while-wait bug?
            obd_request_speed();
            obd_rx_speed_detect();
            speed_obd = (int32_t)obd_read_speed();

            run_tag = 0x0031;
            gnsspps_timestamp(&tv);
            run_tag = 0x0032;
            time_t timestamp = tv.tv_sec;
            uint32_t subsec = tv.tv_usec;
            spd_seq++;
            int spd_len = sens_sync_encode_wheelspd(&sens_sync, 
                timestamp, 
                subsec,
                speed_obd,
                spd_seq);

            run_tag = 0x0033;
            udp_transfer_raw_control(sens_sync.buff, spd_len);

            run_tag = 0x0034;
            memset(sens_sync.buff, 0, SENS_SYNC_PROTO_PARSEBUFFERLEN);
        }

        if (HAL_GetTick() - m1 >= 500) {
            m1 = HAL_GetTick();
            board_led_toggle(1);

            run_tag = 0x0040;
            // lwip timeout task core
            sys_check_timeouts();
            run_tag = 0x0050;
        }

        gnsspps_recalib();

    }
}

