#include "canspeed_obd.h"

#include <device/can.h>

static struct can_dev_s* obdbus;
static int8_t vehicle_speed = 0;

void obd_bus_init()
{
    obdbus = can_bus_get(2);
    if (!obdbus) {
        printf("[CAN] OBD Bus init failed \r\n");
        return;
    }
    printf("[can] OBD bus init completed \r\n");
}

void obd_request_speed()
{
    struct can_msg_s msg_test;
    memset(&msg_test, 0, sizeof(struct can_msg_s));
    msg_test.cm_hdr.ch_extid = 0;
    msg_test.cm_hdr.ch_rtr = 0;
    msg_test.cm_hdr.ch_id = 0x7DFul;
    msg_test.cm_hdr.ch_dlc = 8;
    msg_test.cm_data[0] = 0x02;
    msg_test.cm_data[1] = 0x01;
    msg_test.cm_data[2] = 0x0D;
    msg_test.cm_data[3] = 0x00;
    msg_test.cm_data[4] = 0x00;
    msg_test.cm_data[5] = 0x00;
    msg_test.cm_data[6] = 0x00;
    msg_test.cm_data[7] = 0x00;

    obdbus->cd_ops->co_send(obdbus, &msg_test);
}

static struct can_msg_s rmsg;
void obd_rx_speed_detect()
{
    if (can_rxfifo_get(&obdbus->cd_rxfifo, &rmsg) == DTRUE &&
        rmsg.cm_hdr.ch_id == 0x7E8) {

        vehicle_speed = (int8_t)(rmsg.cm_data[3]);  // byte3 [2]: 0D
    } else if (can_rxfifo_get(&obdbus->cd_rxfifo, &rmsg) == DTRUE) {
        // printf("id: %x \r\n ", rmsg.cm_hdr.ch_id);
    }
}

int8_t obd_read_speed()
{
    int8_t spd = vehicle_speed;

    return spd;
}
