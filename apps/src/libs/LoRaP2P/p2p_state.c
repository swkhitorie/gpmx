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

#if !defined(LORAP2P_SAVE)
    if (obj->_role == P2P_SENDER) {
        // start to recv link request
        p2p_rx(obj, 0);
    } else if (obj->_role == P2P_RECEIVER) {
        // do nothing
    }
#else

#if defined(P2P_ROLE_SLAVE)
    // do nothing
#endif

#if defined(P2P_ROLE_MASTER)
    // start to recv link request
    p2p_rx(obj, 0);
#endif

#endif

    obj->_state = P2P_LINK_FIND;
    obj->_substate = 0x01;

    /** clear some state */
    prb_reset(&obj->_prbuf);
    memset(&obj->_proto, 0, sizeof(struct __p2p_proto));

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

#if !defined(LORAP2P_SAVE)
    if (obj->_role == P2P_SENDER) {
        // do nothing
        P2P_TIME_DELAY(20);
    } else if (obj->_role == P2P_RECEIVER) {
        // start to recv data
        p2p_rx(obj, 0);
    }
#else

#if defined(P2P_ROLE_SLAVE)
    // start to recv data
    p2p_rx(obj, 0);
#endif

#if defined(P2P_ROLE_MASTER)
    // do nothing
    P2P_TIME_DELAY(20);
#endif

#endif

    obj->_state = P2P_LINK_ESTABLISHED;
    obj->_substate = 0x11;

    /** clear some state */
    prb_reset(&obj->_prbuf);
    memset(&obj->_proto, 0, sizeof(struct __p2p_proto));

    obj->_ch_grp.dw_fq_idx = 0;
    obj->_ch_grp.up_fq_idx = 0;

    rand_lcg_seed_set(obj->_id_ck.rand_key_board);

    obj->_link_failed_timestamp = P2P_TIMESTAMP_GET();

    p2p_info("Established, next:[%d.%d]\r\n",
        obj->_ch_grp.current.freq/1000000,
        (obj->_ch_grp.current.freq/1000)%1000);

}

void p2p_state_to_link_config(struct __p2p_obj *obj)
{
    p2p_set_standby(obj);

#if !defined(LORAP2P_SAVE)
    if (obj->_role == P2P_SENDER) {
        // start recv config data
        p2p_rx(obj, 0);
    } else if (obj->_role == P2P_RECEIVER) {
        // do nothing
    }
#else

#if defined(P2P_ROLE_SLAVE)
    // do nothing
#endif

#if defined(P2P_ROLE_MASTER)
    // start recv config data
    p2p_rx(obj, 0);
#endif

#endif
    P2P_TIME_DELAY(20);

    obj->_state = P2P_LINK_CONFIG;
    obj->_substate = 0x21;
    obj->_cmd_seq = 0x00;

    obj->_cmd_ack_timeout_enable = 0;
    obj->_cmd_ack_timestamp = 0;
    obj->_cmd_ack_timeout = 0;

    /** clear some state */
    prb_reset(&obj->_prbuf);
    memset(&obj->_proto, 0, sizeof(struct __p2p_proto));

    obj->_link_failed_timestamp = P2P_TIMESTAMP_GET();

    p2p_info("start to config (%d.%d)\r\n",
        obj->_ch_grp.current.freq/1000000,
        (obj->_ch_grp.current.freq/1000)%1000);
}
