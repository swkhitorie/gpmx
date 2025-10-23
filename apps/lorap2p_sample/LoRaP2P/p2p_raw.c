#include "p2p_common.h"

/**
 * 1.4k/s dataflow (raw mode)->
 * if sf=5, bw=500khz, pack TOA=32ms, The radio-trans speed can catch up with dataflow speed
 * if sf=7, bw=250khz  pack TOA=189ms,The radio-trans speed slows down, unable to keep, data will lost
 */

/****************************************************************************
 * Sender 
 ****************************************************************************/
void p2p_raw_sender_process(struct __p2p_obj *obj)
{
#if (!defined(LORAP2P_SAVE)) || (defined(LORAP2P_SAVE) && defined(P2P_ROLE_MASTER) && defined(P2P_MODE_RAW))
    size_t rsz;
    switch (obj->_substate) {
    case 0x11: {
            size_t rsz = obj->_fstream(&obj->_pbuffer[0], obj->_max_payload);
            if (rsz > 0) {
                p2p_send(obj, &obj->_pbuffer[0], rsz);
                obj->_substate = 0x12;
            }
            break;
        }
    case 0x12: {
            if (p2p_is_tx_done(obj)) {
                obj->_substate = 0x11;
            }
            break;
        }
    default: break;
    }
#endif

}

/****************************************************************************
 * Receiver 
 ****************************************************************************/
void p2p_raw_receiver_process(struct __p2p_obj *obj)
{
#if (!defined(LORAP2P_SAVE)) || (defined(LORAP2P_SAVE) && defined(P2P_ROLE_SLAVE) && defined(P2P_MODE_RAW))
    size_t rsz;
    switch (obj->_substate) {
    case 0x11: {
            if (prb_is_empty(&obj->_prbuf)) {
                return;
            }

            rsz = prb_read(&obj->_prbuf, obj->_pbuffer, 255);
            if (rsz <= 0) {
                return;
            }

            obj->_fstream(&obj->_pbuffer[0], rsz);
            p2p_info("rssi: %d\r\n", obj->_dw_ch_rssi);
            break;
        }
    default: break;
    }
#endif
}

void p2p_raw_process(struct __p2p_obj *obj)
{

#if !defined(LORAP2P_SAVE)
    if (obj->_role == P2P_SENDER) {
        p2p_raw_sender_process(obj);
    } else if (obj->_role == P2P_RECEIVER) {
        p2p_raw_receiver_process(obj);
    }
#else

#if defined(P2P_ROLE_SLAVE) && defined(P2P_MODE_RAW)
    p2p_raw_receiver_process(obj);
#endif

#if defined(P2P_ROLE_MASTER) && defined(P2P_MODE_RAW)
    p2p_raw_sender_process(obj);
#endif

#endif

}
