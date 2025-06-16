#include "lora_common.h"

void lora_p2p_state_to_linkfind(lora_state_t *obj)
{
    rb_reset(&obj->rf_rxbuf);
    memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
    memset(obj->rf_read, 0, 255);

    do {
        // wait last onTxDone trigger
        if (obj->tx_done &&
            board_subghz_tx_ready()) {
            break;
        }
    } while(1);

    // exit other mode, return to standby mode
    Radio.Standby();
    if (obj->role == LORA_SENDER) {
        // start to recv link request
        Radio.Rx(0);
    } else if (obj->role == LORA_RECEIVER) {
        // do nothing
    }

    obj->state = P2P_LINK_FIND;
    obj->sub_state = 0x01;

    obj->link_find_timestamp = LORA_TIMESTAMP_GET();

    LORAP2P_DEBUG("Link Find \r\n");
}

void lora_p2p_state_to_link_established(lora_state_t *obj)
{
    rb_reset(&obj->rf_rxbuf);
    memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));

    Radio.Standby();
    if (obj->role == LORA_SENDER) {
        // do nothing
    } else if (obj->role == LORA_RECEIVER) {
        // start to recv data
        Radio.Rx(0);
    }

    obj->state = P2P_LINK_ESTABLISHED;
    obj->sub_state = 0x11;

    LORAP2P_DEBUG("Established \r\n");
}

