#include "p2p_common.h"

void p2p_state_to_linkfind(p2p_obj_t *obj)
{
    rb_reset(&obj->rf_rxbuf);
    memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
    memset(obj->rf_read, 0, 255);
    obj->id.rand_key_obj = 0;
    obj->id.auth_key_obj = 0;
    obj->flowctrl_bytes_snd = 0;
    p2p_channelstate_reset(obj);

    do {
        // wait last onTxDone trigger
        if (p2p_is_tx_done(obj)) {
            break;
        }
    } while(1);

    // exit other mode, return to standby mode
    Radio.Standby();
    while (Radio.GetStatus() != RF_IDLE);

    // switch to ping channel
    Radio.SetChannel(obj->channelgrp.ping.freq);

    if (obj->role == P2P_SENDER) {
        // start to recv link request
        Radio.Rx(0);
    } else if (obj->role == P2P_RECEIVER) {
        // do nothing
    }

    obj->state = P2P_LINK_FIND;
    obj->sub_state = 0x01;

    obj->link_find_timestamp = P2P_TIMESTAMP_GET();

    P2P_DEBUG("Link Find \r\n");
}

void p2p_state_to_link_established(p2p_obj_t *obj)
{
    rb_reset(&obj->rf_rxbuf);
    memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
    rand_lcg_seed_set(obj->id.rand_key_board);

    obj->channelgrp.down_freq_idx = 0;
    obj->channelgrp.up_freq_idx = 0;

    Radio.Standby();

    // scan first channel
    Radio.SetChannel(obj->channelgrp.current.freq);

    if (obj->role == P2P_SENDER) {
        // do nothing
    } else if (obj->role == P2P_RECEIVER) {
        // start to recv data
        Radio.Rx(0);
    }

    obj->state = P2P_LINK_ESTABLISHED;
    obj->sub_state = 0x11;

    obj->link_failed_timestamp = P2P_TIMESTAMP_GET();

    P2P_DEBUG("Established \r\n");
}

