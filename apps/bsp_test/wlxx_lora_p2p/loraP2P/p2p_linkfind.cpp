#include "p2p_common.h"

/****************************************************************************
 * Receiver 
 ****************************************************************************/
static void p2p_linkfind_recv(p2p_obj_t *obj)
{
    size_t rsz;
    int ret;

    req_connect_t f_reqconnect;
    req_allow_t f_reqallow;

    switch (obj->sub_state) {
    case 0x01: {
            // send REQUEST_CONNECT
            f_reqconnect.rcv_key = obj->id.rand_key_board;
            util_id_set_to_array(&f_reqconnect.rcv_id[0], &obj->id.uid_board[0]);
            rsz = encode_req_connect(&obj->rf_rtcm.buff[0], &f_reqconnect);

            p2p_send(obj, &obj->rf_rtcm.buff[0], rsz);

            obj->sub_state = 0x02;
        }
        break;
    case 0x02: {
            if (p2p_is_tx_done(obj)) {
                while (Radio.GetStatus() != RF_IDLE);

                rb_reset(&obj->rf_rxbuf);
                Radio.Rx(0);
                obj->sub_state = 0x03;

                P2P_DEBUG("P2P Wait Link-Allow\n");
            }
        }
        break;
    case 0x03: {
            // wait receive REQUEST_ALLOW
            rsz = rb_read(&obj->rf_rxbuf, obj->rf_read, 255);
            for (int i = 0; i < rsz; i++) {
                ret = input_rtcm3(&obj->rf_rtcm, obj->rf_read[i]);
                switch (ret) {
                case -1: break;
                case -2: {
                        if (obj->rf_rtcm.nbyte >= 3 && 
                            obj->rf_rtcm.len != (P2P_REQUEST_ALLOW_ARRAYLEN-3)) {
                            memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                            P2P_DEBUG("RTCM Len ERROR in Allow\n");
                        }
                        break;
                    }
                case -3: {
                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        P2P_DEBUG("CRC ERROR in Allow\n");
                        break;
                    }
                case 0:  {
                        ret = decode_req_allow(&obj->rf_rtcm, &f_reqallow);

                        if (ret == 0 &&
                            util_id_compare(&obj->id.uid_board[0], &f_reqallow.rcv_id[0])) {

                            obj->channelgrp.fdown_idx = f_reqallow.down_freq_idx;
                            obj->channelgrp.fup_idx = f_reqallow.up_freq_idx;

                            // switch to down channel, and ready
                            obj->channelgrp.current.freq = obj->channelgrp.downlist[obj->channelgrp.fdown_idx].freq;

                            util_id_set_to_uid(&obj->id.uid_obj[0], &f_reqallow.snd_id[0]);
                            obj->id.rand_key_obj = f_reqallow.snd_key;
                            obj->id.auth_key_obj = (*obj->hauth)(obj->id.uid_obj, obj->id.rand_key_obj);

                            obj->sub_state = 0x04;
                            memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));

                            P2P_DEBUG("Link-Find Completed Freq: %d, %d, OBJ-ID: %x %x %x\n",
                                obj->channelgrp.current.freq, obj->channelgrp.fup_idx,
                                *((uint32_t *)&f_reqallow.snd_id[0]),
                                *((uint32_t *)&f_reqallow.snd_id[4]),
                                *((uint32_t *)&f_reqallow.snd_id[8]));

                            p2p_state_to_link_established(obj);
                            return;
                        } else if (ret == 1) {
                            // P2P_DEBUG("Allow ERROR %d %d\n", ret, f_reqallow.typid);
                        } else if (ret == 0) {
                            // P2P_DEBUG("Allow Rcv Id Error\n");
                        }
                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        break;
                    }
                }
            }

            if (board_elapsed_tick(obj->link_find_timestamp) > P2P_WAIT_CONNECTING_TIMEOUT) {
                // re-enter
                p2p_state_to_linkfind(obj);
            }
        }
        break;
    case 0x04: 
        //idle, never run here
        break;
    default:break;
    }
}

/****************************************************************************
 * Sender 
 ****************************************************************************/
static void p2p_linkfind_send(p2p_obj_t *obj)
{
    size_t rsz;
    int ret;

    req_connect_t f_reqconnect;
    req_allow_t f_reqallow;

    switch (obj->sub_state) {
    case 0x01: {
            // wait receive REQUEST_CONNECT
            rsz = rb_read(&obj->rf_rxbuf, obj->rf_read, 255);
            for (int i = 0; i < rsz; i++) {
                ret = input_rtcm3(&obj->rf_rtcm, obj->rf_read[i]);
                switch (ret) {
                case -1: break;
                case -2: {
                        if (obj->rf_rtcm.nbyte >= 3 && 
                            obj->rf_rtcm.len != (P2P_REQUEST_CONNECT_ARRAYLEN-3)) {
                            P2P_DEBUG("RTCM Len ERROR in Req\n");
                            memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        }
                        break;
                    }
                case -3: {
                        P2P_DEBUG("RTCM CRC ERROR in Req\n");
                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        break;
                    }
                case 0: {
                        ret = decode_req_connect(&obj->rf_rtcm, &f_reqconnect);
                        if (ret == 0) {
                            if (obj->isbond) {
                                if (!util_id_compare(&obj->id.uid_obj[0], &f_reqconnect.rcv_id[0])) {
                                    memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                                    P2P_DEBUG("Rcv-ID wrong \r\n");
                                    return;
                                } else {
                                    obj->id.rand_key_obj = f_reqconnect.rcv_key;
                                    obj->id.auth_key_obj = (*obj->hauth)(obj->id.uid_obj, obj->id.rand_key_obj);
                                }
                            } else {
                                util_id_set_to_uid(&obj->id.uid_obj[0], &f_reqconnect.rcv_id[0]);
                                obj->id.rand_key_obj = f_reqconnect.rcv_key;
                                obj->id.auth_key_obj = (*obj->hauth)(obj->id.uid_obj, obj->id.rand_key_obj);
                                obj->isbond = true;

                                P2P_DEBUG("Rcv-Key %x %x\r\n", obj->id.rand_key_obj, obj->id.auth_key_obj);
                            }

                            obj->sub_state = 0x02;
                            memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));

                            P2P_DEBUG("OBJ-ID: %x %x %x\n",
                                *((uint32_t *)&f_reqconnect.rcv_id[0]),
                                *((uint32_t *)&f_reqconnect.rcv_id[4]),
                                *((uint32_t *)&f_reqconnect.rcv_id[8]));
                        }
                        break;
                    }
                }
            }

            if (board_elapsed_tick(obj->link_find_timestamp) > P2P_WAIT_CONNECTING_TIMEOUT) {
                // re-enter
                p2p_state_to_linkfind(obj);
            }
        }
        break;
    case 0x02: {
            // send REQUEST_ALLOW
            f_reqallow.rcv_key = obj->id.rand_key_obj;
            f_reqallow.snd_key = obj->id.rand_key_board;
            f_reqallow.down_freq_idx = obj->channelgrp.fdown_idx;
            f_reqallow.up_freq_idx = obj->channelgrp.fup_idx;
            util_id_set_to_array(&f_reqallow.snd_id[0], &obj->id.uid_board[0]);
            util_id_set_to_array(&f_reqallow.rcv_id[0], &obj->id.uid_obj[0]);
            rsz = encode_req_allow(&obj->rf_rtcm.buff[0], &f_reqallow);

            Radio.Standby();
            while (Radio.GetStatus() != RF_IDLE);

            p2p_send(obj, &obj->rf_rtcm.buff[0], rsz);

            obj->sub_state = 0x03;
        }
        break;
    case 0x03: {
            if (p2p_is_tx_done(obj)) {
                while (Radio.GetStatus() != RF_IDLE);
                // switch to down channel, and ready
                obj->channelgrp.current.freq = obj->channelgrp.downlist[obj->channelgrp.fdown_idx].freq;

                P2P_DEBUG("Link-Find Completed \r\n");

                obj->sub_state = 0x04;
                p2p_state_to_link_established(obj);
            }
        }
        break;
    case 0x04: 
        //idle, never run here
        break;
    default:break;
    }
}

void p2p_linkfind(p2p_obj_t *obj)
{
    if (obj->role == P2P_SENDER) {
        p2p_linkfind_send(obj);
    } else if (obj->role == P2P_RECEIVER) {
        p2p_linkfind_recv(obj);
    }
}
