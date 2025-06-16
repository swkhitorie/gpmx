#include "app_main.h"
#include "lora_common.h"

lora_state_t obj_lora;

size_t lora_forward_data_sender(uint8_t *p, size_t len)
{
    return SERIAL_RDBUF(_tty_msg_in, p, len);
}

size_t lora_forward_data_receiver(uint8_t *p, size_t len)
{
    return SERIAL_SEND(_tty_msg_out, p, len);
}

int main(int argc, char *argv[])
{
    board_init();
    board_bsp_init();
    board_rng_init();
    board_subghz_init();

    lora_p2p_objcallback_set(&obj_lora);

    uint32_t boardid[3];
    board_get_uid(boardid);
    radio_p2p_role_t role_board;
    forwarding_data handle_forward;

#if (RADIO_BOARD_ROLE == RADIO_BOARD_TRANSMITTER)
    role_board = LORA_SENDER;
    handle_forward = lora_forward_data_sender;
#elif (RADIO_BOARD_ROLE == RADIO_BOARD_RECEIVER)
    role_board = LORA_RECEIVER;
    handle_forward = lora_forward_data_receiver;
#endif

    lora_p2p_setup(&obj_lora, role_board, 
        LORA_REGION_EU868, handle_forward, boardid, 0);

    uint32_t m = HAL_GetTick();
    for (;;) {

        lora_p2p_process(&obj_lora, 0);

        if (board_elapsed_tick(m) > 100) {
            m = HAL_GetTick();
            board_led_toggle(0);
        }
    }

}
