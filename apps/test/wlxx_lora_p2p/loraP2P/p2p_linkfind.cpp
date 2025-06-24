#include "p2p_common.h"

/****************************************************************************
 * Receiver 
 ****************************************************************************/
static void p2p_linkfind_recv(p2p_obj_t *obj)
{
    uint8_t c;
    size_t rsz;
    int ret;

    req_connect_t f_reqconnect;
    req_allow_t f_reqallow;

    switch (obj->sub_state) {
    case 0x01: {
            // start send REQUEST_CONNECT
            f_reqconnect.rcv_key = obj->id.rand_key_board;
            util_id_set_to_array(&f_reqconnect.rcv_id[0], &obj->id.uid_board[0]);
            rsz = encode_req_connect(&obj->rf_rtcm.buff[0], &f_reqconnect);

            p2p_send(obj, &obj->rf_rtcm.buff[0], rsz);

            obj->sub_state = 0x02;

            P2P_DEBUG("P2P Wait Link-Allow\n");
        }
        break;
    case 0x02: {
            // wait REQUEST_CONNECT send completed, and turn to receiver REQUEST_ALLOW
            if (p2p_is_tx_done(obj)) {
                while (Radio.GetStatus() != RF_IDLE);

                rb_reset(&obj->rf_rxbuf);
                Radio.Rx(0);
                obj->sub_state = 0x03;
            }
        }
        break;
    case 0x03: {
            rsz = rb_read(&obj->rf_rxbuf, &c, 1);
            if (rsz > 0) {
                ret = input_rtcm3(&obj->rf_rtcm, c);
                switch (ret) {
                case -1: break;
                case -2: break;
                case -3:
                        P2P_DEBUG("CRC ERROR in Allow\n");
                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        break;
                case 0:
                    {
                        ret = decode_req_allow(&obj->rf_rtcm, &f_reqallow);

                        if (ret == 0 &&
                            util_id_compare(&obj->id.uid_board[0], &f_reqallow.rcv_id[0])) {

                            obj->channelgrp.current.freq = obj->channelgrp.ch_list[f_reqallow.first_freq_idx].freq;

                            P2P_DEBUG("Link-Find Completed Freq: %d, OBJ-ID: %x %x %x\n",
                                obj->channelgrp.current.freq,
                                *((uint32_t *)&f_reqallow.snd_id[0]),
                                *((uint32_t *)&f_reqallow.snd_id[4]),
                                *((uint32_t *)&f_reqallow.snd_id[8]));

                            util_id_set_to_uid(&obj->id.uid_obj[0], &f_reqallow.snd_id[0]);
                            obj->id.rand_key_obj = f_reqallow.snd_key;
                            obj->id.auth_key_obj = (*obj->hauth)(obj->id.uid_obj, obj->id.rand_key_obj);
                            // LORAP2P_DEBUG("snd key %x %x\r\n", obj->id_key_obj, obj->auth_key_obj);

                            obj->sub_state = 0x04;
                            memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));

                            p2p_state_to_link_established(obj);
                            return;
                        } else if (ret == 1) {
                            // LORAP2P_DEBUG("Allow ERROR %d %d\n", ret, f_reqallow.typid);
                        } else if (ret == 0) {
                            // LORAP2P_DEBUG("Allow Id Error\n");
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
    uint8_t c;
    size_t rsz;
    int ret;

    req_connect_t f_reqconnect;
    req_allow_t f_reqallow;

    switch (obj->sub_state) {
    case 0x01:
        {
            // start to recv REQUEST_CONNECT
            rsz = rb_read(&obj->rf_rxbuf, &c, 1);
            if (rsz > 0) {
                ret = input_rtcm3(&obj->rf_rtcm, c);
                switch (ret) {
                case -1: break;
                case -2: break;
                case -3: 
                        P2P_DEBUG("CRC ERROR in Req\n");
                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        break;
                case 0: {
                        ret = decode_req_connect(&obj->rf_rtcm, &f_reqconnect);

                        if (ret == 0) {
                            obj->sub_state = 0x02;

                            util_id_set_to_uid(&obj->id.uid_obj[0], &f_reqconnect.rcv_id[0]);
                            obj->id.rand_key_obj = f_reqconnect.rcv_key;
                            obj->id.auth_key_obj = (*obj->hauth)(obj->id.uid_obj, obj->id.rand_key_obj);
                            // LORAP2P_DEBUG("rcv key %x %x\r\n", obj->id_key_obj, obj->auth_key_obj);

                            P2P_DEBUG("OBJ-ID: %x %x %x\n",
                                *((uint32_t *)&f_reqconnect.rcv_id[0]),
                                *((uint32_t *)&f_reqconnect.rcv_id[4]),
                                *((uint32_t *)&f_reqconnect.rcv_id[8]));
                            memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
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
    case 0x02:
        {
            // send REQUEST_ALLOW
            f_reqallow.rcv_key = obj->id.rand_key_obj;
            f_reqallow.snd_key = obj->id.rand_key_board;
            if (obj->status.first_enter) {
                f_reqallow.first_freq_idx = obj->channelgrp.first_idx;
            } else {
                f_reqallow.first_freq_idx = 0;
            }
            util_id_set_to_array(&f_reqallow.snd_id[0], &obj->id.uid_board[0]);
            util_id_set_to_array(&f_reqallow.rcv_id[0], &obj->id.uid_obj[0]);
            rsz = encode_req_allow(&obj->rf_rtcm.buff[0], &f_reqallow);

            Radio.Standby();
            while (Radio.GetStatus() != RF_IDLE);

            p2p_send(obj, &obj->rf_rtcm.buff[0], rsz);

            obj->sub_state = 0x03;
        }
        break;
    case 0x03:
        {
            if (p2p_is_tx_done(obj)) {
                while (Radio.GetStatus() != RF_IDLE);

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
