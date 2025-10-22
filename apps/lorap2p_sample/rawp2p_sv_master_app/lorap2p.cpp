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
static uint16_t p2p_forward_data_sender(uint8_t *p, uint16_t len);
static void     p2p_cfg_ack_sender(void *arg);

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
        P2P_SENDER,
        P2P_RAWACK_FHSS,
        LORA_REGION_EU868,
        p2p_forward_data_sender,
        p2p_key_generate,
        p2p_antenna_switch,
        p2p_cfg_ack_sender,
        boardid,
        rand_key,
        0
    );


}

void lorap2p_process()
{
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

uint16_t p2p_forward_data_sender(uint8_t *p, uint16_t len)
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


void p2p_cfg_ack_sender(void *arg)
{
    p2p_ack_process(arg);
}



/****************************************************************************
 * LoRaP2P Mode Process
 ****************************************************************************/
#include "app_main.h"
static uint32_t ota_state = 0x00;
static uint32_t ota_bin_addr = 0;
static uint32_t ota_bin_size = 0;
static uint32_t ota_bin_laddr = 0;

static uint32_t ota_version = 0;
static uint32_t ota_size = 0;

static uint32_t crc_rcv = 0;
static uint32_t crc_cal = 0;

uint16_t p2p_raw_trasmission(uint8_t *p, uint16_t len)
{
    int rsz = 0;
    rsz = SERIAL_RDBUF(_tty_msg_in, p, len);

    return rsz;
}

uint16_t p2p_config_process(uint8_t *p, uint16_t len)
{
    int ret = 0;
    uint8_t dev_type = p2p_get_devtype(&p2p_loraobj);
    uint8_t cmd_type = p2p_get_cmdtype(&p2p_loraobj);

    if (dev_type == DEV_LORAP2P_SLAVE) {

        if (cmd_type == CMD_LORAP2P_OTA_START) {

            if (0x01 == *(uint32_t *)(&p[0])) {
                ota_bin_addr = APP_A_LOAD_FLASH_ADDR;
                ota_bin_laddr = ota_bin_addr;
            }

            if (0x02 == *(uint32_t *)(&p[0])) {
                ota_bin_addr = APP_B_LOAD_FLASH_ADDR;
                ota_bin_laddr = ota_bin_addr;
            }
            ota_version = *(uint32_t *)(&p[4]);
            crc_rcv = *(uint32_t *)(&p[8]);
            ota_size = *(uint32_t *)(&p[12]);

            stm32_flash_erase(ota_bin_addr, ota_size);
            printf("[cfg-slave] ota start load:%08x, version:%08x, crc:%08x, size:%08x\r\n",
                ota_bin_addr, ota_version, crc_rcv, ota_size);

            ota_state = 0x01;
        }

        if (cmd_type == CMD_LORAP2P_OTA_PAYLOAD) {

            printf("[cfg-slave] ota payload \r\n");
            ret = stm32_flash_write(ota_bin_laddr, p, len);

            if (ret == len) {
                ota_bin_laddr += len;
            }

            ota_state = 0x02;
        }

        if (cmd_type == CMD_LORAP2P_OTA_END) {

            if (APP_A_LOAD_FLASH_ADDR == ota_bin_addr) {
                _g_nvm.app_a.load_flag = 0x00000000;
                _g_nvm.app_a.version = ota_version;
                _g_nvm.app_a.crc = crc_rcv;
                _g_nvm.app_a.size = ota_size;
            }

            if (APP_B_LOAD_FLASH_ADDR == ota_bin_addr) {
                ota_bin_addr = APP_B_LOAD_FLASH_ADDR;
                _g_nvm.app_b.load_flag = 0x00000000;
                _g_nvm.app_b.version = ota_version;
                _g_nvm.app_b.crc = crc_rcv;
                _g_nvm.app_b.size = ota_size;
            }

            nvm_write(&_g_nvm);

            printf("[cfg-slave] ota end\r\n");
            ota_state = 0x03;
        }

    }


    return 0;
}

void p2p_ack_process(void *arg)
{
    uint32_t *cmd_ret = (uint32_t *)arg;

    // set cmd result

    if (ota_state == 0x01) {
        *cmd_ret = CMD_LORAP2P_OTA_START_ACK;
    }

    if (ota_state == 0x02) {
        *cmd_ret = CMD_LORAP2P_OTA_PAYLOAD_ACK;
    }

    if (ota_state == 0x03) {
        *cmd_ret = CMD_LORAP2P_OTA_END_ACK;
    }

}
