#include "p2p_common.h"


/****************************************************************************
 * Sender 
 ****************************************************************************/
void p2p_config_sender_process(struct __p2p_obj *obj)
{
#if (!defined(LORAP2P_SAVE)) || (defined(LORAP2P_SAVE) && defined(P2P_ROLE_MASTER))
    uint8_t c;
    size_t rsz;
    uint8_t osz;
    int ret, i;

    struct __p2p_config_ack       cfg_ack;
    struct __p2p_config_datahead  cfg_data;
    uint8_t *raw_payload = 0;

    switch (obj->_substate) {
    case 0x21: {
            if (prb_is_empty(&obj->_prbuf)) {
                return;
            }

            rsz = prb_read(&obj->_prbuf, obj->_pbuffer, 255);
            if (rsz <= 0) {
                return;
            }

            for (i = 0; i < rsz; i++) {
                ret = p2p_proto_parser(&obj->_proto, obj->_pbuffer[i]);
                if (ret == P2P_ID_CONFIG_DATAHEAD) {
                    decode_config_data(&obj->_proto, &cfg_data, &raw_payload, &osz);
                    if (cfg_data.rcv_auth_key == obj->_id_ck.auth_key_board) {

                        if (cfg_data.magic_num == 0x2A &&
                            obj->_action == P2P_ACTION_IDLE) {
                            obj->_action = P2P_ACTION_STATE_TO_CONNECT;
                        }

                        obj->_cmd_seq++;

                        if (osz <= 0) {
                            p2p_info("WTF len error \r\n");
                        }

                        // handle payload
                        p2p_set_devtype(obj, cfg_data.dev_type);
                        p2p_set_cmdtype(obj, cfg_data.cmd_type);
                        obj->_fstream(raw_payload, osz);

                        obj->_substate = 0x22;
                        memset(&obj->_proto, 0, sizeof(struct __p2p_proto));
                        return;
                    } else {
                        p2p_info("Verify Auth-Key Error, %x %x\r\n", 
                            cfg_data.rcv_auth_key, obj->_id_ck.auth_key_board);
                    }

                    memset(&obj->_proto, 0, sizeof(struct __p2p_proto));
                }
            }
        }
        break;
    case 0x22: {
            // send CONNECTION_RESULT
            cfg_ack.snd_auth_key = obj->_id_ck.auth_key_obj;
            cfg_ack.magic_num = (obj->_action == P2P_ACTION_STATE_TO_CONNECT)?0x2B:0x00;

            obj->_fack_cfg(&cfg_ack.cmd_ret);

            rsz = encode_config_ack(&obj->_proto, &cfg_ack);

            p2p_set_standby(obj);

            p2p_send(obj, &obj->_proto.buff[0], rsz);

            obj->_substate = 0x23;
        }
        break;
    case 0x23: {
            // wait CONNECTION_RESULT send completed
            if (p2p_is_tx_done(obj)) {
                p2p_wait_to_idle(obj);
                prb_reset(&obj->_prbuf);

                p2p_rx(obj, 0);

                if (obj->_action == P2P_ACTION_STATE_TO_CONNECT) {
                    obj->_action = P2P_ACTION_IDLE;
                    p2p_state_to_link_established(obj);
                    return;
                }
                obj->_substate = 0x21;
            }
        }
        break;
    }
#endif

}

/****************************************************************************
 * Receiver 
 ****************************************************************************/
void p2p_config_receiver_process(struct __p2p_obj *obj)
{
#if (!defined(LORAP2P_SAVE)) || (defined(LORAP2P_SAVE) && defined(P2P_ROLE_SLAVE))
    uint8_t c;
    size_t rsz;
    uint8_t osz;
    int ret, i;

    struct __p2p_config_ack       cfg_ack;
    struct __p2p_config_datahead  cfg_data;
    switch (obj->_substate) {
    case 0x21: {

            // Add CONFIG_HEAD front of data
            cfg_data.rcv_auth_key = obj->_id_ck.auth_key_obj;
            cfg_data.magic_num = (obj->_action == P2P_ACTION_STATE_TO_CONNECT)?0x2A:0x00;
            osz = obj->_fstream(&obj->_pbuffer[0], 
                obj->_max_payload-(P2P_PROTO_NONPAYLOAD_LEN+P2P_FLEN_CONFIG_DATAHEADER));
            if (osz <= 0) {
                return;
            }

            cfg_data.cmd_type = obj->_cmd_type;
            cfg_data.dev_type = obj->_dev_type;

            rsz = encode_config_data(&obj->_proto, &cfg_data, &obj->_pbuffer[0], osz);

            p2p_send(obj, &obj->_proto.buff[0], rsz);

            obj->_substate = 0x22;
        }
        break;
    case 0x22: {
            if (p2p_is_tx_done(obj)) {
                p2p_wait_to_idle(obj);

                prb_reset(&obj->_prbuf);
                p2p_rx(obj, 0);
                obj->_substate = 0x23;

                if (obj->_cmd_ack_timeout_enable == 1) {
                    obj->_cmd_ack_timestamp = P2P_TIMESTAMP_GET();
                }
            }
        }
        break;
    case 0x23: {

            if (P2P_ELAPSED_TIME(obj->_cmd_ack_timestamp) > obj->_cmd_ack_timeout &&
                obj->_cmd_ack_timeout_enable == 1) {
                // re-enter
                p2p_state_to_link_config(obj);
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
                if (ret == P2P_ID_CONFIG_ACK) {
                    decode_config_ack(&obj->_proto, &cfg_ack);
                    if (cfg_ack.snd_auth_key == obj->_id_ck.auth_key_board) {

                        if (cfg_ack.magic_num == 0x2B &&
                            obj->_action == P2P_ACTION_STATE_TO_CONNECT) {
                            obj->_action = P2P_ACTION_IDLE;
                            p2p_state_to_link_established(obj);
                            return;
                        }

                        obj->_cmd_seq++;
                        obj->_fack_cfg(&cfg_ack.cmd_ret);

                        memset(&obj->_proto, 0, sizeof(struct __p2p_proto));
                        obj->_substate = 0x24;
                        return;
                    } else {
                        p2p_info("Ack AuthKey Error, %x %x\r\n", 
                            cfg_ack.snd_auth_key, obj->_id_ck.auth_key_board);
                    }

                    memset(&obj->_proto, 0, sizeof(struct __p2p_proto));
                }
            }
        }
        break;
    case 0x24: {
            p2p_set_standby(obj);
            obj->_substate = 0x21;
        }
        break;
    }
#endif

}

void p2p_config_process(struct __p2p_obj *obj)
{
#if !defined(LORAP2P_SAVE)
    if (obj->_role == P2P_SENDER) {
        p2p_config_sender_process(obj);
    } else if (obj->_role == P2P_RECEIVER) {
        p2p_config_receiver_process(obj);
    }
#else

#if defined(P2P_ROLE_SLAVE)
    p2p_config_receiver_process(obj);
#endif

#if defined(P2P_ROLE_MASTER)
    p2p_config_sender_process(obj);
#endif

#endif

}
