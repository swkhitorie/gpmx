#include "p2p_common.h"

/****************************************************************************
 * Sender 
 ****************************************************************************/
void p2p_raw_ack_sender_process(struct __p2p_obj *obj)
{
    size_t calsz;
    size_t rsz, osz;
    int ret, i;

    struct __p2p_connect_ack connect_ack;
    struct __p2p_connect_dataheader connect_dataheader;

    switch (obj->_substate) {
    case 0x11: {

            calsz = fcl_cal(obj, rsz); //obj->channelgrp.max_payload;
            if (calsz == 0) {
                return;
            }

            // Add CONNECTION_VERIFY front of data
            connect_dataheader.seq = obj->_status.seq;
            connect_dataheader.rcv_auth_key = obj->_id_ck.auth_key_obj;
            connect_dataheader.dw_fq_idx = 0;
            connect_dataheader.up_fq_idx = 0;
            connect_dataheader.rssi = obj->_up_ch_rssi;
            connect_dataheader.snr = obj->_up_ch_snr;

            osz = obj->_fstream(&obj->_pbuffer[0], calsz);
            if (osz <= 0) {
                return;
            }

            rsz = encode_connect_data(&obj->_proto, &connect_dataheader, &obj->_pbuffer[0], osz);

            obj->_status.nbytes_send = rsz;
            obj->_status.ack_timestamp = P2P_TIMESTAMP_GET();
            obj->_status.lbt_back_time = 0;

            p2p_send(obj, &obj->_proto.buff[0], rsz);

            obj->_substate = 0x12;
            break;
        }
    case 0x12: {
            if (p2p_is_tx_done(obj)) {
                p2p_wait_to_idle(obj);

                prb_reset(&obj->_prbuf);

                p2p_rx(obj, 0);

                obj->_status.seq++;
                obj->_status.nbytes_total_snd += obj->_status.nbytes_send;

                obj->_substate = 0x13;
                obj->_ack_fail_timestamp = P2P_TIMESTAMP_GET();
                obj->_status.ack_wait_timestamp = P2P_TIMESTAMP_GET();

                fcl_sndbytes(obj);
            }
            break;
        }
    case 0x13: {
            // wait recv CONNECTION_RESULT
            if (P2P_ELAPSED_TIME(obj->_ack_fail_timestamp) > P2P_ACK_TIMEOUT) {
                // re enter
                obj->_ack_fail_retry_cnter++;

                if (obj->_ack_fail_retry_cnter > P2P_ACK_TIMEOUT_MAX_TRY_TIMES) {
                    obj->_ack_fail_retry_cnter = 0;

                    p2p_info("Ack Link Error, Return to Find\r\n");

                    p2p_state_to_linkfind(obj);

                } else {

                    p2p_info("Ack Retry, try times: %d \r\n", obj->_ack_fail_retry_cnter);

                    obj->_substate = 0x11;
                    return;
                }
            }

            if (prb_is_empty(&obj->_prbuf)) {
                return;
            }

            rsz = prb_read(&obj->_prbuf, obj->_pbuffer, 255);
            if (rsz <= 0) {
                return;
            }

            for (i = 0; i < rsz; i++) {
                ret = p2p_proto_parser(&obj->_proto, obj->_pbuffer[i]);
                if (ret == P2P_CONNECT_ACK) {
                    decode_connect_ack(&obj->_proto, &connect_ack);
                    if (connect_ack.snd_auth_key == obj->_id_ck.auth_key_board) {
                        if ((obj->_status.seq-connect_ack.seq) == 1) {
                            obj->_status.nbytes_actual_snd += obj->_status.nbytes_send;
                        } else {
                            p2p_info("Unknown seq error\n");
                        }

                        obj->_dw_ch_rssi = connect_ack.rssi;
                        obj->_dw_ch_snr = connect_ack.snr;
                        obj->_ack_fail_retry_cnter = 0;
                        obj->_status.ack_completed_time = P2P_TIMESTAMP_GET() - obj->_status.ack_timestamp;
                        obj->_status.ack_wait_time = P2P_TIMESTAMP_GET() - obj->_status.ack_wait_timestamp;

                        p2p_info("d:(%03d.%03d) Ack:%d, urssi:%d, drssi:%d, usnr:%d, dsnr:%d, snd:%d, lost:%d, acktime:%d, ackwait:%d, lbt time:%d\r\n",
                            obj->_ch_grp.current.freq/1000000,
                            (obj->_ch_grp.current.freq/1000)%1000,
                            connect_ack.seq, 
                            obj->_up_ch_rssi, obj->_dw_ch_rssi,
                            obj->_up_ch_snr, obj->_dw_ch_snr,
                            obj->_status.nbytes_total_snd,
                            obj->_status.nbytes_total_snd-obj->_status.nbytes_actual_snd,
                            obj->_status.ack_completed_time, obj->_status.ack_wait_time,
                            obj->_status.lbt_back_time);

                        memset(&obj->_proto, 0, sizeof(struct __p2p_proto));
                        obj->_substate = 0x14;
                        return;
                    } else {

                        p2p_info("Ack AuthKey Error, %x %x\r\n", 
                            connect_ack.snd_auth_key, obj->_id_ck.auth_key_board);
                    }

                    memset(&obj->_proto, 0, sizeof(struct __p2p_proto));
                }
            }
        }
        break;
    case 0x14: {
            p2p_set_standby(obj);
            obj->_substate = 0x11;
        }
        break;
    default: break;
    }
}

/****************************************************************************
 * Receiver 
 ****************************************************************************/
void p2p_raw_ack_receiver_process(struct __p2p_obj *obj)
{
    uint8_t c;
    size_t rsz;
    uint8_t osz;
    int ret, i;

    struct __p2p_connect_ack connect_ack;
    struct __p2p_connect_dataheader connect_dataheader;
    uint8_t *raw_payload = 0;

    switch (obj->_substate) {
    case 0x11: {
            if (P2P_ELAPSED_TIME(obj->_link_failed_timestamp) > P2P_LINK_FAIL_TIMEOUT) {
                // re-enter
                p2p_state_to_linkfind(obj);
            }

            if (prb_is_empty(&obj->_prbuf)) {
                return;
            }

            rsz = prb_read(&obj->_prbuf, obj->_pbuffer, 255);
            if (rsz <= 0) {
                return;
            }

            for (i = 0; i < rsz; i++) {
                ret = p2p_proto_parser(&obj->_proto, obj->_pbuffer[i]);
                if (ret == P2P_CONNECT_DATAHEAD) {
                    decode_connect_data(&obj->_proto, &connect_dataheader, &raw_payload, &osz);
                    if (connect_dataheader.rcv_auth_key == obj->_id_ck.auth_key_board) {

                        if (osz <= 0) {
                            p2p_info("WTF len error \r\n");
                        }

                        if (obj->_status.seq == 0) {
                            // the first pack received
                            obj->_status.seq_lst = connect_dataheader.seq;
                        }
                        obj->_status.seq = connect_dataheader.seq;
                        if ((connect_dataheader.seq - obj->_status.seq_lst) > 0) {
                            // pack lost
                            obj->_status.pack_lost += (connect_dataheader.seq - obj->_status.seq_lst) - 1;
                        } else if ((connect_dataheader.seq - obj->_status.seq_lst) == 0) {
                            // the first pack received, do nothing
                        } else {

                            p2p_info("Seq Reset \r\n");

                            // wtf: restart sender, then seq need restart
                            obj->_status.seq_lst = connect_dataheader.seq;
                            obj->_status.seq = connect_dataheader.seq;
                        }

                        obj->_up_ch_rssi = connect_dataheader.rssi;
                        obj->_up_ch_snr = connect_dataheader.snr;
                        obj->_ch_grp.dw_fq_idx = connect_dataheader.dw_fq_idx;
                        obj->_substate = 0x12;
                        obj->_status.rcv_ack_timestamp = P2P_TIMESTAMP_GET();

                        // handle payload, directly output
                        obj->_fstream(raw_payload, osz);
                        obj->_status.nbytes_total_recv += osz;

                        memset(&obj->_proto, 0, sizeof(struct __p2p_proto));
                        return;
                    } else {
                        p2p_info("Verify Auth-Key Error, %x %x\r\n", 
                            connect_dataheader.rcv_auth_key, obj->_id_ck.auth_key_board);
                    }

                    memset(&obj->_proto, 0, sizeof(struct __p2p_proto));
                }
            }
        }
        break;
    case 0x12: {
            // send CONNECTION_RESULT
            connect_ack.rssi = obj->_dw_ch_rssi;
            connect_ack.snr = obj->_dw_ch_snr;
            connect_ack.snd_auth_key = obj->_id_ck.auth_key_obj;
            connect_ack.seq = obj->_status.seq;
            rsz = encode_connect_ack(&obj->_proto, &connect_ack);

            p2p_set_standby(obj);

            p2p_send(obj, &obj->_proto.buff[0], rsz);

            obj->_substate = 0x13;
            obj->_status.seq_lst = obj->_status.seq;

            p2p_info("d:(%03d.%03d) Ack:%d, urssi:%d, drssi:%d, usnr:%d, dsnr:%d, rcv:%d, lost:%d, ack-time(lst):%d\r\n", 
                obj->_ch_grp.current.freq/1000000,
                (obj->_ch_grp.current.freq/1000)%1000,
                obj->_status.seq, 
                obj->_up_ch_rssi, obj->_dw_ch_rssi,
                obj->_up_ch_snr, obj->_dw_ch_snr,
                obj->_status.nbytes_total_recv,
                obj->_status.pack_lost,
                obj->_status.rcv_ack_time);

        }
        break;
    case 0x13: {
            // wait CONNECTION_RESULT send completed
            if (p2p_is_tx_done(obj)) {
                p2p_wait_to_idle(obj);

                prb_reset(&obj->_prbuf);

                p2p_rx(obj, 0);

                obj->_status.rcv_ack_time = P2P_TIMESTAMP_GET() - obj->_status.rcv_ack_timestamp;
                obj->_substate = 0x11;
                obj->_link_failed_timestamp = P2P_TIMESTAMP_GET();
                obj->_ack_fail_timestamp = P2P_TIMESTAMP_GET();
            }
        }
        break;
    default: break;
    }
}

void p2p_raw_ack_process(struct __p2p_obj *obj)
{
    if (obj->_role == P2P_SENDER) {
        p2p_raw_ack_sender_process(obj);
    } else if (obj->_role == P2P_RECEIVER) {
        p2p_raw_ack_receiver_process(obj);
    }
}
