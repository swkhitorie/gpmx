#include "lorap2p.h"

#include <board_config.h>
#include <drv_flash.h>

struct __p2p_obj p2p_loraobj = {
    .hw = {
        .cad_done = false,
        .cad_busy = false,
        .tx_done = true,
        .rx_crc_error_occur = false,
        .rx_timeout_occur = false,
        .tx_timeout_occur = false,
        .rx_crc_error_cnt = 0,
        .rx_timeout_cnt = 0,
        .tx_timeout_cnt = 0,
    },
};

static void     p2p_antenna_switch(uint8_t idx);
static uint32_t p2p_key_generate(uint32_t *uid, uint32_t key);
static uint16_t p2p_forward_data_receiver(uint8_t *p, uint16_t len);
static void     p2p_cfg_ack_receiver(void *arg);

static uint16_t p2p_raw_trasmission(uint8_t *p, uint16_t len);
static uint16_t p2p_config_process(uint8_t *p, uint16_t len);
static void     p2p_ack_process(void *arg);

void lorap2p_init()
{
    uint32_t boardid[3];
    uint32_t rand_key;
    int role_board = board_get_role();

    board_get_uid(boardid);
    rand_key = board_rng_get();

    p2p_platform_obj_bind(&p2p_loraobj);

    p2p_setup(
        &p2p_loraobj, 
        P2P_RECEIVER,
        P2P_RAWACK_FHSS,
        LORA_REGION_EU868,
        p2p_forward_data_receiver,
        p2p_key_generate,
        p2p_antenna_switch,
        p2p_cfg_ack_receiver,
        boardid,
        rand_key,
        0
    );
}

void lorap2p_process()
{    
    static uint32_t mstart = HAL_GetTick();
    static bool mstart_flag = false;
    static bool mend_flag = false;

    if (board_elapsed_tick(mstart) > 1000*10 && !mstart_flag) {
        mstart_flag = true;

        p2p_action(&p2p_loraobj, P2P_ACTION_STATE_TO_CONFIG);
    }

    p2p_process(&p2p_loraobj);
}


/****************************************************************************
 * LoRaP2P CallBack
 ****************************************************************************/

uint32_t p2p_key_generate(uint32_t *uid, uint32_t key)
{
    return board_crc_key_get(uid, key);
}

void p2p_antenna_switch(uint8_t idx)
{
    switch (idx) {
    case 0:
        // printf("Antenna switch to 0\r\n");
        break;
    case 1:
        // printf("Antenna switch to 1\r\n");
        break;
    }
}

uint16_t p2p_forward_data_receiver(uint8_t *p, uint16_t len)
{
    int rsz = 0;

    switch (p2p_loraobj._state) {
    case P2P_LINK_ESTABLISHED:
        rsz = p2p_raw_trasmission(p, len);
        break;
    case P2P_LINK_CONFIG:
        rsz = p2p_config_process(p, len);
        break;
    }

    return rsz;
}

void p2p_cfg_ack_receiver(void *arg)
{
    p2p_ack_process(arg);
}



/****************************************************************************
 * LoRaP2P Mode Process
 ****************************************************************************/
#include "app_main.h"
// extern uint32_t            ybin_type;
// extern uint32_t            ybin_version;
// extern uint32_t            crc_file;
// extern int                 total_ysize;

static uint32_t ota_state = 0x00;
static uint32_t ota_bin_addr = 0x08020000;
static uint32_t ota_bin_size = 40*1024;
static uint32_t ota_bin_raddr = ota_bin_addr;

uint16_t p2p_raw_trasmission(uint8_t *p, uint16_t len)
{
    int rsz = 0;
    rsz = SERIAL_DMASEND(_tty_msg_out, p, len);

    return rsz;
}

uint16_t p2p_config_process(uint8_t *p, uint16_t len)
{
    int ssz = 0;
    int ret = 0;

    if (ota_state == 0x00) {
        p2p_set_devtype(&p2p_loraobj, DEV_LORAP2P_SLAVE);
        p2p_set_cmdtype(&p2p_loraobj, CMD_LORAP2P_OTA_START);
        memcpy(&p[0], &ybin_type, 4);
        memcpy(&p[4], &ybin_version, 4);
        memcpy(&p[8], &crc_file, 4);
        memcpy(&p[12], &total_ysize, 4);
        ota_bin_size = total_ysize;
        ssz = 16;
    }

    if (ota_state == 0x01) {
        p2p_set_devtype(&p2p_loraobj, DEV_LORAP2P_SLAVE);
        p2p_set_cmdtype(&p2p_loraobj, CMD_LORAP2P_OTA_PAYLOAD);

        if ((ota_bin_addr + ota_bin_size) > ota_bin_raddr) {

            int rszf = (ota_bin_addr + ota_bin_size - ota_bin_raddr);
            int wsz =  (rszf > 240)?240:rszf;  // 240 byte, 60 word, 30 doubleword

            ret = stm32_flash_read(ota_bin_raddr, p, wsz); 

            if (ret == wsz) {
                ssz = ret;
                ota_bin_raddr += wsz;
                printf("[cfg-master] payload snd: %08X \r\n", ota_bin_raddr);
            }
        } else {
            ota_state = 0x02;
        }
    }

    if (ota_state == 0x02) {
        p2p_set_devtype(&p2p_loraobj, DEV_LORAP2P_SLAVE);
        p2p_set_cmdtype(&p2p_loraobj, CMD_LORAP2P_OTA_END);
        p[0] = 0xFB;
        ssz = 1;
    }

    if (ota_state == 0x03) {
        ssz = 0;
    }

    return ssz;
}

void p2p_ack_process(void *arg)
{
    uint32_t cmd_ret = *((uint32_t *)arg);

    // handle cmd result from recving

    if (cmd_ret == CMD_LORAP2P_OTA_START_ACK) {
        ota_state = 0x01;
        ota_bin_raddr = ota_bin_addr;
        printf("[cfg-master] ota start\r\n");
    }

    if (cmd_ret == CMD_LORAP2P_OTA_PAYLOAD_ACK) {
    }

    if (cmd_ret == CMD_LORAP2P_OTA_END_ACK) {
        printf("[cfg-master] ota end\r\n");

        ota_state = 0x03;
    }

}
