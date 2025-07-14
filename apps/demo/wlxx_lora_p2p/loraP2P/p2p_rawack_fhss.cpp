#include "p2p_common.h"

/****************************************************************************
 * Sender 
 ****************************************************************************/
void p2p_raw_ackfhss_sender_process(p2p_obj_t *obj)
{
    size_t rsz, rsz2, ssize;
    int ret;

    connect_verify_t con_verify;
    connect_ret_t con_ret;

    switch (obj->sub_state) {
    case 0x11: {
            p2p_downchannelnext(obj, obj->channelgrp.down_rssi, obj->channelgrp.down_snr);

            // Add CONNECTION_VERIFY front of data
            con_verify.seq = obj->status.seq;
            con_verify.rcv_auth_key = obj->id.auth_key_obj;
            con_verify.down_freq_idx = obj->channelgrp.down_freq_idx;
            con_verify.up_freq_idx = 0;
            con_verify.rssi = obj->channelgrp.up_rssi;
            con_verify.snr = obj->channelgrp.up_snr;
            rsz = encode_connect_verify(&obj->rf_read[0], &con_verify);

            ssize = fcl_cal(obj, rsz); //obj->channelgrp.max_payload;
            if (ssize == 0) {
                return;
            }

            rsz2 = (*obj->hp)(&obj->rf_read[0]+rsz, ssize-rsz);
            if (rsz2 > 0) {

                obj->status.nbytes_send = rsz2+rsz;    //include verify
                obj->status.ack_timestamp = P2P_TIMESTAMP_GET();
                obj->status.lbt_back_time = 0;

                // lbt will cost 10ms at least
                if (p2p_lbt_send(obj, &obj->rf_read[0], rsz+rsz2, 
                        P2P_SEND_LBT_RSSI_THRESH,
                        P2P_SEND_LBT_SENSE_TIME, 
                        P2P_SEND_LBT_FREECNT_TIME,
                        P2P_SEND_LBT_TOTAL_TIME) == RADIO_STATUS_ERROR) {

                    P2P_DEBUG("Channel is too busy to send, please check Environment\r\n");

                    return;
                }
                // p2p_send(obj, &obj->rf_read[0], rsz+rsz2);

                obj->sub_state = 0x12;
            }
            break;
        }
    case 0x12: {
            if (p2p_is_tx_done(obj)) {
                p2p_wait_to_idle(obj);

                p2p_setchannel(obj, obj->channelgrp.uplist[obj->channelgrp.fup_idx].freq);

                rb_reset(&obj->rf_rxbuf);

                p2p_rx(obj, 0);

                memset(&obj->rf_read, 0, 255);
                obj->status.seq++;
                obj->status.nbytes_total_snd += obj->status.nbytes_send;

                obj->sub_state = 0x13;
                obj->ack_timeout_timestamp = P2P_TIMESTAMP_GET();
                obj->status.ack_wait_timestamp = P2P_TIMESTAMP_GET();
                fcl_sndbytes(obj);
            }
            break;
        }
    case 0x13: {
            // wait recv CONNECTION_RESULT
            rsz = rb_read(&obj->rf_rxbuf, obj->rf_read, 255);
            for (int i = 0; i < rsz; i++) {
                ret = input_rtcm3(&obj->rf_rtcm, obj->rf_read[i]);
                switch (ret) {
                case -1: break;
                case -2: {
                        if (obj->rf_rtcm.nbyte >= 3 && 
                            obj->rf_rtcm.len != (P2P_CONNECT_RESULT_ARRAYLEN-3)) {
                            memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));

                            P2P_DEBUG("RTCM Len ERROR in Result\n");

                        }
                        break;
                    }
                case -3: {
                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));

                        P2P_DEBUG("RTCM CRC ERROR in Result \n");

                        break;
                    }
                case 0: {
                        ret = decode_connect_result(&obj->rf_rtcm, &con_ret);

                        if (ret == 0 && con_ret.snd_auth_key == obj->id.auth_key_board) {

                            if ((obj->status.seq-con_ret.seq) == 1) {
                                obj->status.nbytes_actual_snd += obj->status.nbytes_send;
                            } else {

                                P2P_DEBUG("Unknown seq error\n");

                            }

                            obj->channelgrp.down_rssi = con_ret.rssi;
                            obj->channelgrp.down_snr = con_ret.snr;
                            obj->ack_timeout_cnter = 0;
                            obj->status.ack_completed_time = P2P_TIMESTAMP_GET() - obj->status.ack_timestamp;
                            obj->status.ack_wait_time = P2P_TIMESTAMP_GET() - obj->status.ack_wait_timestamp;

                            P2P_DEBUG("d:(%03d.%03d) Ack:%d, urssi:%d, drssi:%d, usnr:%d, dsnr:%d, snd:%d, lost:%d, acktime:%d, ackwait:%d, lbt time:%d, nxtfreq:(%03d.%03d)\r\n",
                                obj->channelgrp.current.freq/1000000,
                                (obj->channelgrp.current.freq/1000)%1000,
                                con_ret.seq, 
                                obj->channelgrp.up_rssi, obj->channelgrp.down_rssi,
                                obj->channelgrp.up_snr, obj->channelgrp.down_snr,
                                obj->status.nbytes_total_snd,
                                obj->status.nbytes_total_snd-obj->status.nbytes_actual_snd,
                                obj->status.ack_completed_time, obj->status.ack_wait_time,
                                obj->status.lbt_back_time,
                                obj->channelgrp.downlist[obj->channelgrp.down_freq_idx].freq/1000000,
                                (obj->channelgrp.downlist[obj->channelgrp.down_freq_idx].freq/1000)%1000);

                            memset(&obj->rf_read, 0, 255);
                            memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                            obj->sub_state = 0x14;
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

                    p2p_downchannelnext(obj, -100, -10);
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
            p2p_standby(obj);
            p2p_setchannel(obj, obj->channelgrp.downlist[obj->channelgrp.down_freq_idx].freq);
            obj->channelgrp.current.freq = obj->channelgrp.downlist[obj->channelgrp.down_freq_idx].freq;

            obj->sub_state = 0x11;
        }
        break;
    default: break;
    }
}

/****************************************************************************
 * Receiver 
 ****************************************************************************/
void p2p_raw_ackfhss_receiver_process(p2p_obj_t *obj)
{
    uint8_t c;
    size_t rsz;
    int ret;

    connect_verify_t con_verify;
    connect_ret_t con_ret;

    static int t1, t2;
    switch (obj->sub_state) {
    case 0x11: {
            // wait rcv CONNECT_VERIFY
            rsz = rb_read(&obj->rf_rxbuf, &c, 1);
            if (rsz > 0) {
                ret = input_rtcm3(&obj->rf_rtcm, c);
                switch (ret) {
                case -1: break;
                case -2: {
                        if (obj->rf_rtcm.nbyte >= 3 && 
                            obj->rf_rtcm.len != (P2P_CONNECT_VERIFY_ARRAYLEN-3)) {
                            memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));

                            P2P_DEBUG("RTCM Len ERROR in Verify\n");

                        }
                        break;
                    }
                case -3: {
                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));

                        P2P_DEBUG("RTCM CRC ERROR in Verify\n");

                        break;
                    }
                case 0: {
                        ret = decode_connect_verify(&obj->rf_rtcm, &con_verify);
                        if (ret == 0 && con_verify.rcv_auth_key == obj->id.auth_key_board) {

                            if (obj->status.seq == 0) {
                                // the first pack received
                                obj->status.seq_lst = con_verify.seq;
                            }
                            obj->status.seq = con_verify.seq;
                            if ((con_verify.seq - obj->status.seq_lst) > 0) {
                                // pack lost
                                obj->status.pack_lost += (con_verify.seq - obj->status.seq_lst) - 1;
                            } else if ((con_verify.seq - obj->status.seq_lst) == 0) {
                                // the first pack received, do nothing
                            } else {

                                P2P_DEBUG("Seq Reset \r\n");

                                // wtf: restart sender, then seq need restart
                                obj->status.seq_lst = con_verify.seq;
                                obj->status.seq = con_verify.seq;
                            }

                            obj->channelgrp.up_rssi = con_verify.rssi;
                            obj->channelgrp.up_snr = con_verify.snr;
                            obj->channelgrp.down_freq_idx = con_verify.down_freq_idx;
                            obj->status.rcv_ack_timestamp = P2P_TIMESTAMP_GET();
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

            if (P2P_ELAPSED_TIME(obj->link_failed_timestamp) > P2P_LINK_FAIL_TIMEOUT) {
                // re-enter
                p2p_state_to_linkfind(obj);
            }
        }
        break;
    case 0x12: {
            rsz = rb_read(&obj->rf_rxbuf, &obj->rf_read[0], obj->channelgrp.max_payload);
            if (rsz > 0) {
                // use  high efficient usart-dma
                (*obj->hp)(&obj->rf_read[0], rsz);
                obj->status.nbytes_total_recv += rsz;
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

            p2p_standby(obj);
            p2p_setchannel(obj, obj->channelgrp.uplist[obj->channelgrp.fup_idx].freq);

            p2p_send(obj, &obj->rf_rtcm.buff[0], rsz);

            obj->sub_state = 0x14;
            obj->status.seq_lst = obj->status.seq;

            P2P_DEBUG("d:(%03d.%03d) Ack:%d, urssi:%d, drssi:%d, usnr:%d, dsnr:%d, rcv:%d, lost:%d, ack-time(lst):%d\r\n", 
                obj->channelgrp.current.freq/1000000,
                (obj->channelgrp.current.freq/1000)%1000,
                obj->status.seq, 
                obj->channelgrp.up_rssi, obj->channelgrp.down_rssi,
                obj->channelgrp.up_snr, obj->channelgrp.down_snr,
                obj->status.nbytes_total_recv,
                obj->status.pack_lost,
                obj->status.rcv_ack_time);

        }
        break;
    case 0x14: {
            // wait CONNECTION_RESULT send completed
            if (p2p_is_tx_done(obj)) {
                p2p_wait_to_idle(obj);

                p2p_setchannel(obj, obj->channelgrp.downlist[obj->channelgrp.down_freq_idx].freq);
                obj->channelgrp.current.freq = obj->channelgrp.downlist[obj->channelgrp.down_freq_idx].freq;

                rb_reset(&obj->rf_rxbuf);

                p2p_rx(obj, 0);

                obj->status.rcv_ack_time = P2P_TIMESTAMP_GET() - obj->status.rcv_ack_timestamp;
                obj->sub_state = 0x11;
                obj->link_failed_timestamp = P2P_TIMESTAMP_GET();
                obj->ack_timeout_timestamp = P2P_TIMESTAMP_GET();
            }
        }
        break;
    default: break;
    }
}

void p2p_raw_ackfhss_process(p2p_obj_t *obj)
{
    if (obj->role == P2P_SENDER) {
        p2p_raw_ackfhss_sender_process(obj);
    } else if (obj->role == P2P_RECEIVER) {
        p2p_raw_ackfhss_receiver_process(obj);
    }
}
