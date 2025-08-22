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

static uint16_t p2p_forward_data_sender(uint8_t *p, uint16_t len);
static uint16_t p2p_forward_data_receiver(uint8_t *p, uint16_t len);
static uint32_t p2p_key_generate(uint32_t *uid, uint32_t key);

static uint16_t p2p_fcl_bytes = 0;

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
    enum __p2p_role role_board;
    p2p_forwarding_data handle_forward;
    p2p_authkey_generate handle_auth_key;

    if (board_get_role() == RADIO_BOARD_TRANSMITTER) {
        role_board = P2P_SENDER;
        handle_forward = p2p_forward_data_sender;
    } else if (board_get_role() == RADIO_BOARD_RECEIVER) {
        role_board = P2P_RECEIVER;
        handle_forward = p2p_forward_data_receiver;
    }
    handle_auth_key = p2p_key_generate;

    p2p_setup(
        &obj_p2p, 
        role_board,
        P2P_RAWACK_FHSS,
        LORA_REGION_EU868,
        handle_forward,
        handle_auth_key,
        boardid,
        rand_key,
        p2p_fcl_bytes
    );

    uint32_t m = HAL_GetTick();
    for (;;) {

        p2p_process(&obj_p2p);

        if (board_elapsed_tick(m) > 100 && obj_p2p._state == P2P_LINK_ESTABLISHED) {
            m = HAL_GetTick();
            board_led_toggle(0);
        }
    }

}

/****************************************************************************
 * RAW Data Mode 
 ****************************************************************************/
uint16_t p2p_forward_data_sender(uint8_t *p, uint16_t len)
{
    return SERIAL_RDBUF(_tty_msg_in, p, len);
}

uint16_t p2p_forward_data_receiver(uint8_t *p, uint16_t len)
{
    return SERIAL_DMASEND(_tty_msg_out, p, len);
}

uint32_t p2p_key_generate(uint32_t *uid, uint32_t key)
{
    return board_crc_key_get(uid, key);
}
