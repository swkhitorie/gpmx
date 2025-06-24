#include "app_main.h"
#include "p2p_common.h"
#include "rtk_basemsg.h"

p2p_obj_t obj_p2p;

size_t p2p_forward_data_sender(uint8_t *p, size_t len)
{
    return SERIAL_RDBUF(_tty_msg_in, p, len);
}

size_t p2p_forward_data_receiver(uint8_t *p, size_t len)
{
    return SERIAL_SEND(_tty_msg_out, p, len);
}

uint32_t p2p_authkey_generate(uint32_t *uid, uint32_t key)
{
    return board_crc_key_get(uid, key);
}

// rtcm_t f_decode;
// __attribute__((section(".RAM2_region"))) static rtk_basemsg_t rtk_base;
// uint8_t buff[512];

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();
    board_rng_init();
    board_crc_init();

    board_subghz_init();
    p2p_objcallback_set(&obj_p2p);

    uint32_t boardid[3];
    uint32_t rand_key;
    board_get_uid(boardid);
    rand_key = board_rng_get();
    p2p_role_t role_board;
    forwarding_data handle_forward;
    authkey_generate handle_auth_key;

    if (board_get_role() == RADIO_BOARD_TRANSMITTER) {
        role_board = P2P_SENDER;
        handle_forward = p2p_forward_data_sender;
    } else if (board_get_role() == RADIO_BOARD_RECEIVER) {
        role_board = P2P_RECEIVER;
        handle_forward = p2p_forward_data_receiver;
    }
    handle_auth_key = p2p_authkey_generate;

    p2p_setup(
        &obj_p2p, 
        role_board,
        P2P_RAWACK,
        LORA_REGION_US915, 
        handle_forward, 
        handle_auth_key,
        boardid, 
        rand_key
    );

    // memset(&rtk_base, 0, sizeof(rtk_base));

    uint32_t m = HAL_GetTick();
    for (;;) {

        p2p_process(&obj_p2p);

        if (board_elapsed_tick(m) > 100) {
            m = HAL_GetTick();
            board_led_toggle(0);
        }

        // int sz = SERIAL_RDBUF(_tty_msg_in, buff, 512);
        // rtk_basemsg_decode(&rtk_base, &f_decode, buff, sz);

    }

}
