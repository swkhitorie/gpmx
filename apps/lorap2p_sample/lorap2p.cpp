#include "lorap2p.h"

#include <board_config.h>

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
static uint16_t p2p_forward_data_receiver(uint8_t *p, uint16_t len);
static void     p2p_cfg_ack_sender(void *arg);
static void     p2p_cfg_ack_receiver(void *arg);

void lorap2p_init()
{
    enum __p2p_role       _t_role_board;
    p2p_forwarding_data   _t_handle_forward;
    p2p_authkey_generate  _t_handle_auth_key;
    p2p_ant_selection     _t_handle_ant_select;
    p2p_config_ack        _t_handle_cfg_ack;

    uint32_t boardid[3];
    uint32_t rand_key;
    int role_board = board_get_role();

    board_get_uid(boardid);
    rand_key = board_rng_get();

    p2p_platform_obj_bind(&p2p_loraobj);

    _t_handle_auth_key   = p2p_key_generate;
    _t_handle_ant_select = p2p_antenna_switch;

    if (role_board == RADIO_BOARD_TRANSMITTER) {
        _t_role_board = P2P_SENDER;
        _t_handle_forward = p2p_forward_data_sender;
        _t_handle_cfg_ack = p2p_cfg_ack_sender;
    } else if (role_board == RADIO_BOARD_RECEIVER) {
        _t_role_board = P2P_RECEIVER;
        _t_handle_forward = p2p_forward_data_receiver;
        _t_handle_cfg_ack = p2p_cfg_ack_receiver;
    }

    p2p_setup(
        &p2p_loraobj, 
        _t_role_board,
        P2P_RAWACK_FHSS,
        LORA_REGION_EU868,
        _t_handle_forward,
        _t_handle_auth_key,
        _t_handle_ant_select,
        _t_handle_cfg_ack,
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
        rsz = SERIAL_RDBUF(_tty_msg_in, p, len);
        break;
    case P2P_LINK_CONFIG:
        break;
    }

    return rsz;
}

uint16_t p2p_forward_data_receiver(uint8_t *p, uint16_t len)
{
    int rsz = 0;

    switch (p2p_loraobj._state) {
    case P2P_LINK_ESTABLISHED:
        rsz = SERIAL_DMASEND(_tty_msg_out, p, len);
        break;
    case P2P_LINK_CONFIG:
        break;
    }

    return rsz;
}

void p2p_cfg_ack_sender(void *arg)
{
    uint32_t *val = (uint32_t *)arg;

}

void p2p_cfg_ack_receiver(void *arg)
{
    uint32_t rt = *((uint32_t *)arg);

}



