#include "p2p_common.h"

/**
 * 1.4k/s dataflow (raw mode)->
 * if sf=5, bw=500khz, pack TOA=32ms, The radio-trans speed can catch up with dataflow speed
 * if sf=7, bw=250khz  pack TOA=189ms,The radio-trans speed slows down, unable to keep, data will lost
 */

/****************************************************************************
 * Sender 
 ****************************************************************************/
void p2p_raw_sender_process(p2p_obj_t *obj)
{
    size_t rsz;
    switch (obj->sub_state) {
    case 0x11: {
            size_t rsz = (*obj->hp)(&obj->rf_read[0], obj->channelgrp.max_payload);
            if (rsz > 0) {
                p2p_send(obj, &obj->rf_read[0], rsz);
                obj->sub_state = 0x12;
            }
            break;
        }
    case 0x12: {
            if (p2p_is_tx_done(obj)) {
                obj->sub_state = 0x11;
            }
            break;
        }
    default: break;
    }
}

/****************************************************************************
 * Receiver 
 ****************************************************************************/
void p2p_raw_receiver_process(p2p_obj_t *obj)
{
    size_t rsz;
    switch (obj->sub_state) {
    case 0x11: {
            rsz = rb_read(&obj->rf_rxbuf, &obj->rf_read[0], obj->channelgrp.max_payload);
            if (rsz > 0) { 
                (*obj->hp)(&obj->rf_read[0], rsz);

                P2P_DEBUG("rssi: %d\r\n", obj->channelgrp.down_rssi);

            }
            break;
        }
    default: break;
    }
}

void p2p_raw_process(p2p_obj_t *obj)
{
    if (obj->role == P2P_SENDER) {
        p2p_raw_sender_process(obj);
    } else if (obj->role == P2P_RECEIVER) {
        p2p_raw_receiver_process(obj);
    }
}
