#include "app_main.h"
#include "p2p_common.h"

struct __p2p_obj obj_p2p = {
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

static uint16_t _g_p2p_fcl_bytes = 0;

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();
    board_rng_init();
    board_crc_init();
    board_subghz_init();

    p2p_platform_obj_bind(&obj_p2p);

    uint32_t boardid[3];
    uint32_t rand_key;
    board_get_uid(boardid);
    rand_key = board_rng_get();

    enum __p2p_role       _t_role_board;
    p2p_forwarding_data   _t_handle_forward;
    p2p_authkey_generate  _t_handle_auth_key;
    p2p_ant_selection     _t_handle_ant_select;
    p2p_config_ack        _t_handle_cfg_ack;

    _t_handle_auth_key = p2p_key_generate;
    _t_handle_ant_select = p2p_antenna_switch;

    if (board_get_role() == RADIO_BOARD_TRANSMITTER) {
        _t_role_board = P2P_SENDER;
        _t_handle_forward = p2p_forward_data_sender;
        _t_handle_cfg_ack = p2p_cfg_ack_sender;
    } else if (board_get_role() == RADIO_BOARD_RECEIVER) {
        _t_role_board = P2P_RECEIVER;
        _t_handle_forward = p2p_forward_data_receiver;
        _t_handle_cfg_ack = p2p_cfg_ack_receiver;
    }

    p2p_setup(
        &obj_p2p, 
        _t_role_board,
        P2P_RAWACK_FHSS,
        LORA_REGION_EU868,
        _t_handle_forward,
        _t_handle_auth_key,
        _t_handle_ant_select,
        _t_handle_cfg_ack,
        boardid,
        rand_key,
        _g_p2p_fcl_bytes
    );

    uint32_t m = HAL_GetTick();
    uint32_t ac = 0;
    for (;;) {

        p2p_process(&obj_p2p);

        // test
        if (board_get_role() == RADIO_BOARD_RECEIVER) {
            if (HAL_GetTick() >= 20*1000 && ac == 0) {
                ac = 1;
                p2p_action(&obj_p2p, P2P_ACTION_STATE_TO_CONFIG);
            }

            if (HAL_GetTick() >= 40*1000 && ac == 1) {
                ac = 2;
                p2p_action(&obj_p2p, P2P_ACTION_STATE_TO_CONNECT);
            }
        }

        if (board_elapsed_tick(m) > 100 && obj_p2p._state == P2P_LINK_ESTABLISHED) {
            m = HAL_GetTick();
            board_led_toggle(0);
        }
    }

}

/****************************************************************************
 * RAW Data Mode 
 ****************************************************************************/
uint32_t tick_p2p;
uint16_t p2p_forward_data_sender(uint8_t *p, uint16_t len)
{
    int rsz = 0;
    switch (obj_p2p._state) {
    case P2P_LINK_ESTABLISHED:
        rsz = SERIAL_RDBUF(_tty_msg_in, p, len);
        break;
    case P2P_LINK_CONFIG:
        tick_p2p = HAL_GetTick();
        printf("rcv cmd: ");
        for (int i = 0; i < len; i++) {
            printf("%02X ", p[i]);
        }
        printf("\r\n");
        break;
    }

    return rsz;
}

uint16_t p2p_forward_data_receiver(uint8_t *p, uint16_t len)
{
    int rsz = 0;
    uint8_t test_cmd[5] = {0xc1, 0xc2, 0xc3, 0xc4, 0xc5};

    switch (obj_p2p._state) {
    case P2P_LINK_ESTABLISHED:
        rsz = SERIAL_DMASEND(_tty_msg_out, p, len);
        break;
    case P2P_LINK_CONFIG:
        if (HAL_GetTick() - tick_p2p > 1000) {
            tick_p2p = HAL_GetTick();
            rsz = 5;
            memcpy(p, test_cmd, 5);
        }
        break;
    }

    return rsz;
}

void p2p_cfg_ack_sender(void *arg)
{
    uint32_t *val = (uint32_t *)arg;
    *val = HAL_GetTick();
    printf("send ack: %d \r\n", *val);
}

void p2p_cfg_ack_receiver(void *arg)
{
    uint32_t rt = *((uint32_t *)arg);
    printf("ACK: %d \r\n", rt);
}

uint32_t p2p_key_generate(uint32_t *uid, uint32_t key)
{
    return board_crc_key_get(uid, key);
}

void p2p_antenna_switch(uint8_t idx)
{
    switch (idx) {
    case 0:
        printf("Antenna switch to 0\r\n");
        break;
    case 1:
        printf("Antenna switch to 1\r\n");
        break;
    }
}
