#include "p2p_common.h"

/****************************************************************************
 * Sender 
 ****************************************************************************/
void p2p_raw_ack_sender_process(p2p_obj_t *obj)
{
    uint8_t c;
    size_t rsz, rsz2;
    int ret;

    connect_verify_t con_verify;
    connect_ret_t con_ret;

    switch (obj->sub_state) {
    case 0x11: {
            // send CONNECTION_VERIFY head, front of data
            con_verify.seq = obj->status.seq;
            con_verify.rcv_auth_key = obj->id.auth_key_obj;
            con_verify.down_freq_idx = 0;
            con_verify.up_freq_idx = 0;
            rsz = encode_connect_verify(&obj->rf_read[0], &con_verify);

            rsz2 = (*obj->hp)(&obj->rf_read[0]+rsz, obj->channelgrp.max_payload-rsz);
            if (rsz2 > 0) {

                obj->status.nbytes_send = rsz2;
                obj->status.ack_timestamp = P2P_TIMESTAMP_GET();

                if (p2p_lbt_send(obj, &obj->rf_read[0], rsz+rsz2, -70, 5, 5, 50) == RADIO_STATUS_ERROR) {
                    P2P_DEBUG("Channel Busy in Data-Send\r\n");
                    return;
                }
                // p2p_send(obj, &obj->rf_read[0], rsz+rsz2);

                obj->sub_state = 0x12;
            }
        }
        break;
    case 0x12: {
            if (p2p_is_tx_done(obj)) {

                //Radio.Standby();
                //Radio.SetChannel(915000000);
                while (Radio.GetStatus() != RF_IDLE);
                rb_reset(&obj->rf_rxbuf);
                Radio.Rx(0);

                obj->sub_state = 0x13;
                obj->ack_timeout_timestamp = P2P_TIMESTAMP_GET();
            }
        }
        break;
    case 0x13: {
            // wait CONNECTION_RESULT
            rsz = rb_read(&obj->rf_rxbuf, &c, 1);
            if (rsz > 0) {
                ret = input_rtcm3(&obj->rf_rtcm, c);
                switch (ret) {
                case -1: break;
                case -2: break;
                case -3: {
                        P2P_DEBUG("CRC ERROR in Result \n");
                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        break;
                    }
                case 0: {
                        ret = decode_connect_result(&obj->rf_rtcm, &con_ret);

                        if (ret == 0 && con_ret.snd_auth_key == obj->id.auth_key_board) {

                            if (obj->status.seq != con_ret.seq) {
                                obj->status.nbytes_total_lost += obj->status.nbytes_send;
                            } else {
                                obj->status.nbytes_total_snd += obj->status.nbytes_send;
                            }

                            obj->sub_state = 0x14;
                            obj->ack_timeout_cnter = 0;
                            if (obj->status.seq >= 255) {
                                obj->status.seq = 0;
                            } else {
                                obj->status.seq++;
                            }

                            obj->status.ack_completed_time = P2P_TIMESTAMP_GET() - obj->status.ack_timestamp;

                            P2P_DEBUG("Ack tick:%d, rssi:%d, snr:%d, total snd:%d, total acktime: %d\r\n",
                                obj->status.seq, obj->channelgrp.up_rssi, obj->channelgrp.up_snr, obj->status.nbytes_total_snd,
                                obj->status.ack_completed_time);

                            memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                            return;

                        } else if (ret == 1) {
                            // P2P_DEBUG("Allow ERROR %d %d\n", ret, f_reqallow.typid);
                        } else if (ret == 0) {
                            P2P_DEBUG("Ack AuthKey Error, %d %x %x\r\n", ret, 
                                con_ret.snd_auth_key, obj->id.auth_key_board);
                        }

                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        break;
                    }
                }
            }

            if (board_elapsed_tick(obj->ack_timeout_timestamp) > P2P_ACK_TIMEOUT) {
                // re enter
                obj->ack_timeout_cnter++;

                if (obj->ack_timeout_cnter > P2P_ACK_TIMEOUT_MAX_TRY_TIMES) {
                    obj->ack_timeout_cnter = 0;
                    P2P_DEBUG("Ack Link Error, Return to Find\r\n");

                    p2p_state_to_linkfind(obj);

                } else {

                    P2P_DEBUG("Ack Retry, try times: %d \r\n", obj->ack_timeout_cnter);

                    obj->sub_state = 0x11;
                    return;
                }
            }
        }
        break;
    case 0x14: {
            Radio.Standby();
            // Radio.SetChannel(obj->channelgrp.current.freq);
            while (Radio.GetStatus() != RF_IDLE);

            obj->sub_state = 0x11;
        }
        break;
    default: break;
    }
}

/****************************************************************************
 * Receiver 
 ****************************************************************************/
void p2p_raw_ack_receiver_process(p2p_obj_t *obj)
{
    uint8_t c;
    size_t rsz;
    int ret;

    connect_verify_t con_verify;
    connect_ret_t con_ret;

    switch (obj->sub_state) {
    case 0x11: {
            // rcv CONNECT_VERIFY
            rsz = rb_read(&obj->rf_rxbuf, &c, 1);
            if (rsz > 0) {
                ret = input_rtcm3(&obj->rf_rtcm, c);
                switch (ret) {
                case -1: break;
                case -2: break;
                case -3: {
                        P2P_DEBUG("CRC ERROR in Verify\n");
                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        break;
                    }
                case 0: {

                        ret = decode_connect_verify(&obj->rf_rtcm, &con_verify);

                        if (ret == 0 && con_verify.rcv_auth_key == obj->id.auth_key_board) {

                            obj->status.seq = con_verify.seq;
                            obj->sub_state = 0x12;

                            memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                            return;

                        } else if (ret == 1) {
                            // P2P_DEBUG("Allow ERROR %d %d\n", ret, f_reqallow.typid);
                        } else if (ret == 0) {
                            P2P_DEBUG("Verify Auth-Key Error, %d %x %x\r\n", ret, 
                                con_verify.rcv_auth_key, obj->id.auth_key_board);
                        }

                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        break;
                    }
                }
            }

            if (board_elapsed_tick(obj->link_failed_timestamp) > P2P_LINK_FAIL_TIMEOUT) {
                // re-enter
                p2p_state_to_linkfind(obj);
            }

        }
        break;
    case 0x12: {
            rsz = rb_read(&obj->rf_rxbuf, &obj->rf_read[0], obj->channelgrp.max_payload);
            if (rsz > 0) {
                (*obj->hp)(&obj->rf_read[0], rsz);
                obj->sub_state = 0x13;
            }
        }
        break;
    case 0x13: {
            // send CONNECTION_RESULT
            con_ret.rssi = obj->channelgrp.down_rssi;
            con_ret.snr = obj->channelgrp.down_snr;
            con_ret.snd_auth_key = obj->id.auth_key_obj;
            con_ret.seq = obj->status.seq;
            rsz = encode_connect_result(&obj->rf_rtcm.buff[0], &con_ret);

            Radio.Standby();
            // Radio.SetChannel(915000000);
            while (Radio.GetStatus() != RF_IDLE);
            int t1 = HAL_GetTick();

            p2p_send(obj, &obj->rf_rtcm.buff[0], rsz);
            int t2 = HAL_GetTick();
            P2P_DEBUG("ack send: %d \r\n", t2-t1);
            obj->sub_state = 0x14;

            P2P_DEBUG("Send Ack %d\r\n", con_ret.seq);
        }
        break;
    case 0x14: {
            // wait CONNECTION_RESULT send completed
            if (p2p_is_tx_done(obj)) {
                // Radio.Standby();
                //Radio.SetChannel(obj->channelgrp.current.freq);
                while (Radio.GetStatus() != RF_IDLE);
                rb_reset(&obj->rf_rxbuf);
                Radio.Rx(0);

                obj->sub_state = 0x11;
                obj->link_failed_timestamp = P2P_TIMESTAMP_GET();
            }
        }
        break;
    default: break;
    }
}

void p2p_raw_ack_process(p2p_obj_t *obj)
{
    if (obj->role == P2P_SENDER) {
        p2p_raw_ack_sender_process(obj);
    } else if (obj->role == P2P_RECEIVER) {
        p2p_raw_ack_receiver_process(obj);
    }
}
