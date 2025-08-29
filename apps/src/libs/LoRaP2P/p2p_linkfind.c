#include "p2p_common.h"

/****************************************************************************
 * Receiver 
 ****************************************************************************/
static void p2p_linkfind_recv(struct __p2p_obj *obj)
{
#if (!defined(LORAP2P_SAVE)) || (defined(LORAP2P_SAVE) && defined(P2P_ROLE_SLAVE))
    size_t rsz;
    int ret, i;

    struct __p2p_req_connect  frame_req_con;
    struct __p2p_req_allow    frame_req_allow;
    struct __p2p_antenna_test frame_ant;

    switch (obj->_substate) {
    case 0x01: {
            // send REQUEST_CONNECT
            frame_req_con.rcv_key = obj->_id_ck.rand_key_board;
            util_id_set_to_array(&frame_req_con.rcv_id[0], &obj->_id_ck.uid_board[0]);
            rsz = encode_req_connect(&obj->_proto, &frame_req_con);

            p2p_send(obj, &obj->_proto.buff[0], rsz);

            obj->_substate = 0x02;
        }
        break;
    case 0x02: {
            if (p2p_is_tx_done(obj)) {
                p2p_wait_to_idle(obj);

                prb_reset(&obj->_prbuf);

                p2p_rx(obj, 0);

                obj->_substate = 0x03;
            }
        }
        break;
    case 0x03: {

            if (P2P_ELAPSED_TIME(obj->_link_find_timestamp) > P2P_WAIT_CONNECTING_TIMEOUT) {
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
                if (ret == P2P_ID_REQ_ALLOW) {
                    decode_req_allow(&obj->_proto, &frame_req_allow);
                    if (util_id_compare(&obj->_id_ck.uid_board[0], &frame_req_allow.rcv_id[0])) {

                        obj->_ch_grp.dw_fq_idx_fix = frame_req_allow.dw_fq_idx;
                        obj->_ch_grp.up_fq_idx_fix = frame_req_allow.up_fq_idx;

                        // switch to down channel, and ready
                        obj->_ch_grp.current.freq = obj->_ch_grp.dw_list[obj->_ch_grp.dw_fq_idx_fix].freq;

                        util_id_set_to_uid(&obj->_id_ck.uid_obj[0], &frame_req_allow.snd_id[0]);
                        obj->_id_ck.rand_key_obj = frame_req_allow.snd_key;
                        obj->_id_ck.auth_key_obj = obj->_fauth_gen(obj->_id_ck.uid_obj, obj->_id_ck.rand_key_obj);

                        memset(&obj->_proto, 0, sizeof(struct __p2p_proto));
                        prb_reset(&obj->_prbuf);

                        p2p_info("Link-Find Completed Freq: %d, %d, OBJ-ID: %x %x %x\n",
                            obj->_ch_grp.current.freq, obj->_ch_grp.up_fq_idx_fix,
                            *((uint32_t *)&frame_req_allow.snd_id[0]),
                            *((uint32_t *)&frame_req_allow.snd_id[4]),
                            *((uint32_t *)&frame_req_allow.snd_id[8]));

                        obj->_substate = 0x04;
#if !defined(LORAP2P_ANTENNA_LINKTEST)
                        p2p_state_to_link_established(obj);
#else
                        if (obj->ant_snder_selection_completed) {
                            p2p_state_to_link_established(obj);
                        } else {
                            p2p_set_standby(obj);
                            p2p_info("Start Antenna-test\n");
                        }
                        // p2p_state_to_link_established(obj);
                        return;
#endif
                    }
                }
            }

            break;
        }
    case 0x04: 
        /**
         * ************************************************************
         * ******************** Antenna Test Phase ********************
         * ************************************************************
         */
#if defined(LORAP2P_ANTENNA_LINKTEST)
        frame_ant.tag = 0xA0;
        rsz = encode_antenna_test(&obj->_proto, &frame_ant);

        p2p_set_standby(obj);
        p2p_send(obj, &obj->_proto.buff[0], rsz);

        obj->_substate = 0x05;
        break;
    case 0x05: {
            if (p2p_is_tx_done(obj)) {
                p2p_wait_to_idle(obj);

                prb_reset(&obj->_prbuf);

                p2p_rx(obj, 0);

                obj->_link_find_timestamp = P2P_TIMESTAMP_GET();
                obj->_substate = 0x06;
            }
        }
        break;
    case 0x06: {
            if (P2P_ELAPSED_TIME(obj->_link_find_timestamp) > P2P_WAIT_CONNECTING_TIMEOUT) {
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

            // wait receive Antenna-test 0xA1/0xA2
            for (i = 0; i < rsz; i++) {
                ret = p2p_proto_parser(&obj->_proto, obj->_pbuffer[i]);
                if (ret == P2P_ID_ANTENNA_TEST) {
                    decode_antenna_test(&obj->_proto, &frame_ant);
                    if (frame_ant.tag == 0xA2 || frame_ant.tag == 0xA1) {
                        memset(&obj->_proto, 0, sizeof(struct __p2p_proto));
                        obj->_substate = 0x04;
                        if (frame_ant.tag == 0xA2) {
                            p2p_info("Antenna-test Phase Completed \n");
                            obj->ant_snder_selection_completed = true;
                            p2p_state_to_link_established(obj);
                            return;
                        } else if (frame_ant.tag == 0xA1) {
                            p2p_info("Recv Antenna-test Ack \n");
                        }
                    }
                }
            }
        }
        break;
#endif
    default:break;
    }
#endif

}

/****************************************************************************
 * Sender 
 ****************************************************************************/
static void p2p_linkfind_send(struct __p2p_obj *obj)
{
#if (!defined(LORAP2P_SAVE)) || (defined(LORAP2P_SAVE) && defined(P2P_ROLE_MASTER))
    size_t rsz;
    int ret, i;

    struct __p2p_req_connect frame_req_con;
    struct __p2p_req_allow   frame_req_allow;
    struct __p2p_antenna_test frame_ant;

    switch (obj->_substate) {
    case 0x01: {

            if (P2P_ELAPSED_TIME(obj->_link_find_timestamp) > P2P_WAIT_CONNECTING_TIMEOUT) {
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
                if (ret == P2P_ID_REQ_CONNECT) {
                    decode_req_connect(&obj->_proto, &frame_req_con);

                    if (obj->_connect_bond) {
                        if (!util_id_compare(&obj->_id_ck.uid_obj[0], &frame_req_con.rcv_id[0])) {
                            memset(&obj->_proto, 0, sizeof(struct __p2p_proto));
                            p2p_info("Rcv-ID wrong \r\n");
                            return;
                        } else {
                            obj->_id_ck.rand_key_obj = frame_req_con.rcv_key;
                            obj->_id_ck.auth_key_obj = obj->_fauth_gen(obj->_id_ck.uid_obj, obj->_id_ck.rand_key_obj);
                        }
                    } else {
                            util_id_set_to_uid(&obj->_id_ck.uid_obj[0], &frame_req_con.rcv_id[0]);
                            obj->_id_ck.rand_key_obj = frame_req_con.rcv_key;
                            obj->_id_ck.auth_key_obj = obj->_fauth_gen(obj->_id_ck.uid_obj, obj->_id_ck.rand_key_obj);
                            obj->_connect_bond = true;
                            p2p_info("Rcv-Key %x %x\r\n", obj->_id_ck.rand_key_obj, obj->_id_ck.auth_key_obj);
                    }

                    obj->_substate = 0x02;
                    memset(&obj->_proto, 0, sizeof(struct __p2p_proto));

                    p2p_info("OBJ-ID: %x %x %x\n",
                        *((uint32_t *)&frame_req_con.rcv_id[0]),
                        *((uint32_t *)&frame_req_con.rcv_id[4]),
                        *((uint32_t *)&frame_req_con.rcv_id[8]));
                }
            }
        }
        break;
    case 0x02: {
            // send REQUEST_ALLOW
            frame_req_allow.rcv_key = obj->_id_ck.rand_key_obj;
            frame_req_allow.snd_key = obj->_id_ck.rand_key_board;
            if (obj->_status.seq > 0 && obj->_mode == P2P_RAWACK_FHSS) {
                frame_req_allow.dw_fq_idx = obj->_ch_grp.dw_fq_idx;
            } else {
                frame_req_allow.dw_fq_idx = obj->_ch_grp.dw_fq_idx_fix;
            }
            frame_req_allow.up_fq_idx = obj->_ch_grp.up_fq_idx_fix;
            util_id_set_to_array(&frame_req_allow.snd_id[0], &obj->_id_ck.uid_board[0]);
            util_id_set_to_array(&frame_req_allow.rcv_id[0], &obj->_id_ck.uid_obj[0]);
            rsz = encode_req_allow(&obj->_proto, &frame_req_allow);

            p2p_set_standby(obj);

            p2p_send(obj, &obj->_proto.buff[0], rsz);

            obj->_substate = 0x03;
        }
        break;
    case 0x03: {
            if (p2p_is_tx_done(obj)) {
                p2p_wait_to_idle(obj);

                // switch to down channel, and ready
                if (obj->_status.seq > 0 && obj->_mode == P2P_RAWACK_FHSS) {
                    obj->_ch_grp.current.freq = obj->_ch_grp.dw_list[obj->_ch_grp.dw_fq_idx].freq;
                } else {
                    obj->_ch_grp.current.freq = obj->_ch_grp.dw_list[obj->_ch_grp.dw_fq_idx_fix].freq;
                }

                p2p_info("Link-Find Completed \r\n");

                obj->_substate = 0x04;

#if !defined(LORAP2P_ANTENNA_LINKTEST)
                p2p_state_to_link_established(obj);
#else
                if (obj->ant_snder_selection_completed) {
                    p2p_state_to_link_established(obj);
                } else {
                    p2p_set_standby(obj);
                    p2p_rx(obj, 0);

                    obj->_link_find_timestamp = P2P_TIMESTAMP_GET();

                    obj->ant_now = 0;
                    obj->_fant(obj->ant_now);
                    p2p_info("Start Antenna-test: %d\n", obj->ant_now);
                }
#endif
            }
        }
        break;
#if defined(LORAP2P_ANTENNA_LINKTEST)
    case 0x04: {
        /**
         * ************************************************************
         * ******************** Antenna Test Phase ********************
         * ************************************************************
         */
            // wait receive Antenna-test 0xA0
            if (P2P_ELAPSED_TIME(obj->_link_find_timestamp) > P2P_WAIT_CONNECTING_TIMEOUT) {
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

            // wait receive Antenna-test 0xA1/0xA2
            for (i = 0; i < rsz; i++) {
                ret = p2p_proto_parser(&obj->_proto, obj->_pbuffer[i]);
                if (ret == P2P_ID_ANTENNA_TEST) {
                    decode_antenna_test(&obj->_proto, &frame_ant);
                    if (frame_ant.tag == 0xA0) {

                        p2p_info("Recv Antenna-test %d, rssi:%d \r\n", obj->ant_now, obj->_up_ch_rssi);

                        memset(&obj->_proto, 0, sizeof(struct __p2p_proto));
                        obj->_substate = 0x05;
                        return;
                    }
                }
            }
        }
        break;
    case 0x05: {
            uint8_t tag = 0xA1;
            {
                /**
                 * Antenna Array Position Analyse
                 */

                obj->ant_test_cnter++;
                if (obj->ant_test_cnter >= (obj->ant_test_times-1)) {
                    obj->ant_test_cnter = 0;

                    int16_t rssi_sum;
                    int16_t rssi_ant_aver;
                    for (int i = 0; i < P2P_ANTENNA_ARRAY_RSSI_LEN; i++) {
                        rssi_sum += obj->ant_rssi[obj->ant_now][i];
                    }
                    rssi_ant_aver = rssi_sum / P2P_ANTENNA_ARRAY_RSSI_LEN;

                    if (rssi_ant_aver > obj->ant_rssi_best) {
                        obj->ant_rssi_best = rssi_ant_aver;
                        obj->ant_idx_best = obj->ant_now;
                    }

                    obj->ant_now++;
                    if (obj->ant_now >= P2P_ANTENNA_ARRAY) {

                        obj->ant_now = obj->ant_idx_best;
                        obj->_fant(obj->ant_now);

                        obj->ant_snder_selection_completed = true;
                        tag = 0xA2;
                    } else {
                        p2p_info("Antenna switch to idx: %d \r\n", obj->ant_now);
                        obj->_fant(obj->ant_now);
                    }
                }
            }

            frame_ant.tag = tag;
            rsz = encode_antenna_test(&obj->_proto, &frame_ant);

            p2p_set_standby(obj);
            p2p_send(obj, &obj->_proto.buff[0], rsz);

            obj->_substate = 0x06;
        }
        break;
    case 0x06: {
            if (p2p_is_tx_done(obj)) {
                p2p_wait_to_idle(obj);

                prb_reset(&obj->_prbuf);
                p2p_rx(obj, 0);

                obj->_link_find_timestamp = P2P_TIMESTAMP_GET();
                if (!obj->ant_snder_selection_completed) {
                    obj->_substate = 0x04;
                    obj->_link_find_timestamp = P2P_TIMESTAMP_GET();
                } else {
                    p2p_info("Antenna test completed: %d \n", obj->ant_now);
                    p2p_state_to_link_established(obj);
                }
            }
        }
        break;
#endif

    default:break;
    }
#endif

}

void p2p_linkfind(struct __p2p_obj *obj)
{
#if !defined(LORAP2P_SAVE)
    if (obj->_role == P2P_SENDER) {
        p2p_linkfind_send(obj);
    } else if (obj->_role == P2P_RECEIVER) {
        p2p_linkfind_recv(obj);
    }
#else

#if defined(P2P_ROLE_SLAVE)
    p2p_linkfind_recv(obj);
#endif

#if defined(P2P_ROLE_MASTER)
    p2p_linkfind_send(obj);
#endif

#endif

}
