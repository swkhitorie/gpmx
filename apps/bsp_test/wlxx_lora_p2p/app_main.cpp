#include "app_main.h"
#include "p2p_common.h"
#include "rtk_basemsg.h"

#define P2P_FLOWCONTROL_BYTES_ONESEC    (0)

p2p_obj_t obj_p2p;
__attribute__((section(".RAM2_region"))) static rtcm_t f_decode;
__attribute__((section(".RAM2_region"))) static rtk_basemsg_t rtk_base;
__attribute__((section(".RAM2_region"))) static uint8_t buff[512];

static size_t p2p_forward_rtk_sender(uint8_t *p, size_t len);
static size_t p2p_forward_data_sender(uint8_t *p, size_t len);
static size_t p2p_forward_data_receiver(uint8_t *p, size_t len);
static uint32_t p2p_authkey_generate(uint32_t *uid, uint32_t key);

static uint16_t p2p_fcl_bytes = P2P_FLOWCONTROL_BYTES_ONESEC;

//2025/07/01-15:54:26, completed
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
#if (P2P_FLOWCONTROL_BYTES_ONESEC > 0)
        handle_forward = p2p_forward_rtk_sender;
#else
        handle_forward = p2p_forward_data_sender;
#endif
    } else if (board_get_role() == RADIO_BOARD_RECEIVER) {
        role_board = P2P_RECEIVER;
        handle_forward = p2p_forward_data_receiver;
    }
    handle_auth_key = p2p_authkey_generate;

    p2p_setup(
        &obj_p2p, 
        role_board,
        P2P_RAWACK_FHSS,
        LORA_REGION_US915, 
        handle_forward, 
        handle_auth_key,
        boardid, 
        rand_key,
        p2p_fcl_bytes
    );

#if (P2P_FLOWCONTROL_BYTES_ONESEC > 0)
    memset(buff, 0, 512);
    memset(&rtk_base, 0, sizeof(rtk_base));
    memset(&f_decode, 0, sizeof(rtcm_t));
#endif

    uint32_t m = HAL_GetTick();
    for (;;) {

#if (P2P_FLOWCONTROL_BYTES_ONESEC > 0)
        int sz = SERIAL_RDBUF(_tty_msg_in, buff, 512);
        rtk_basemsg_decode(&rtk_base, &f_decode, buff, sz);
#endif

        p2p_process(&obj_p2p);

        if (board_elapsed_tick(m) > 100 && obj_p2p.state == P2P_LINK_ESTABLISHED) {
            m = HAL_GetTick();
            board_led_toggle(0);
        }
    }

}

/****************************************************************************
 * RAW Data Mode 
 ****************************************************************************/
size_t p2p_forward_data_sender(uint8_t *p, size_t len)
{
    return SERIAL_RDBUF(_tty_msg_in, p, len);
}

size_t p2p_forward_data_receiver(uint8_t *p, size_t len)
{
    return SERIAL_DMASEND(_tty_msg_out, p, len);
}

uint32_t p2p_authkey_generate(uint32_t *uid, uint32_t key)
{
    return board_crc_key_get(uid, key);
}

/****************************************************************************
 * RTK Data Mode 
 ****************************************************************************/
size_t p2p_forward_rtk_sender(uint8_t *p, size_t len)
{
    if (rtk_base.current_msm.snd_bytes == 0 &&
        rtk_base.snd_bytes_arp == 0 &&
        rtk_base.snd_bytes_1013 == 0) {
        // step1: first enter, begin to send
        if (!basemsmbuffer_newest(&rtk_base.current_msm, &rtk_base.msg[0], RTK_BASEMSG_BUFFER_LEN)) {
            return 0;
        }
        rtk_base.current_msm.snd_bytes = 0;

        // can't enter here normally
        if (len >= rtk_base.current_msm.len) {
            for (int i = 0; i < rtk_base.current_msm.len; i++) {
                p[i] = rtk_base.current_msm.buffer[i];
            }
            rtk_base.current_msm.snd_bytes += rtk_base.current_msm.len;
            return rtk_base.current_msm.len;
        } else {
            for (int i = 0; i < len; i++) {
                p[i] = rtk_base.current_msm.buffer[i];
            }
            rtk_base.current_msm.snd_bytes += len;
            return len;
        }
    }

    if (rtk_base.current_msm.snd_bytes > 0 && 
        rtk_base.current_msm.snd_bytes < rtk_base.current_msm.len &&
        rtk_base.snd_bytes_arp == 0 &&
        rtk_base.snd_bytes_1013 == 0) {
        // step2: sending msm

        int rsz = rtk_base.current_msm.len - rtk_base.current_msm.snd_bytes;
        if (rsz >= len) {
            for (int i = 0; i < len; i++) {
                p[i] = rtk_base.current_msm.buffer[rtk_base.current_msm.snd_bytes+i];
            }
            rtk_base.current_msm.snd_bytes += len;
            return len;
        } else {
            for (int i = 0; i < rsz; i++) {
                p[i] = rtk_base.current_msm.buffer[rtk_base.current_msm.snd_bytes+i];
            }
            rtk_base.current_msm.snd_bytes += rsz;
            return rsz;
        }
    }

    if (rtk_base.current_msm.snd_bytes == rtk_base.current_msm.len &&
        rtk_base.snd_bytes_arp == 0 &&
        rtk_base.snd_bytes_1013 == 0) {
        // step2: sending arp

        if (len >= (rtk_base.len_1013 + rtk_base.len_arp)) {
            int i = 0, j = 0;
            for (i = 0; i < rtk_base.len_arp; i++) {
                p[i] = rtk_base.msg_arp[i];
            }
            for (j = 0; j < rtk_base.len_1013; j++) {
                p[i+j] = rtk_base.msg_1013[j];
            }
            rtk_base.snd_bytes_arp = rtk_base.len_arp;
            rtk_base.snd_bytes_1013 = rtk_base.len_1013;

            memset(&rtk_base.current_msm, 0, sizeof(rtk_msm_t));
            rtk_base.snd_bytes_arp = 0;
            rtk_base.snd_bytes_1013 = 0;

            return (rtk_base.len_1013 + rtk_base.len_arp);
        } else {
            // no nessary to send 
            memset(&rtk_base.current_msm, 0, sizeof(rtk_msm_t));
            rtk_base.snd_bytes_arp = 0;
            rtk_base.snd_bytes_1013 = 0;
            // BOARD_DEBUG("hhh \r\n");
            return 0;
        }
    }

    return 0;
}
