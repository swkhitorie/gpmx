#include "p2p_common.h"

void p2p_state_to_linkfind(struct __p2p_obj *obj)
{
    do {
        // wait last onTxDone trigger
        if (p2p_is_tx_done(obj)) {
            break;
        }
    } while(1);

    // exit other mode, return to standby mode
    p2p_set_standby(obj);

    // switch to ping channel
    p2p_set_channel(obj, obj->_ch_grp.ping.freq);

    if (obj->_role == P2P_SENDER) {
        // start to recv link request
        p2p_rx(obj, 0);
    } else if (obj->_role == P2P_RECEIVER) {
        // do nothing
    }

    obj->_state = P2P_LINK_FIND;
    obj->_substate = 0x01;

    /** clear some state */
    prb_reset(&obj->_prbuf);

    obj->_id_ck.rand_key_obj = 0;
    obj->_id_ck.auth_key_obj = 0;

    obj->_fctrl.bytes_snd = 0;

    p2p_channel_grp_reset(obj);

    obj->_link_find_timestamp = P2P_TIMESTAMP_GET();

    p2p_info("Link Find in:[%d.%d]\r\n",
        obj->_ch_grp.ping.freq/1000000,
        (obj->_ch_grp.ping.freq/1000)%1000);

}

void p2p_state_to_link_established(struct __p2p_obj *obj)
{
    p2p_set_standby(obj);

    // scan first channel
    p2p_set_channel(obj, obj->_ch_grp.current.freq);

    if (obj->_role == P2P_SENDER) {
        // do nothing
    } else if (obj->_role == P2P_RECEIVER) {
        // start to recv data
        p2p_rx(obj, 0);
    }

    obj->_state = P2P_LINK_ESTABLISHED;
    obj->_substate = 0x11;

    /** clear some state */
    prb_reset(&obj->_prbuf);

    obj->_ch_grp.dw_fq_idx = 0;
    obj->_ch_grp.up_fq_idx = 0;

    rand_lcg_seed_set(obj->_id_ck.rand_key_board);

    obj->_link_failed_timestamp = P2P_TIMESTAMP_GET();

    p2p_info("Established, next:[%d.%d]\r\n",
        obj->_ch_grp.current.freq/1000000,
        (obj->_ch_grp.current.freq/1000)%1000);

}

