#include "lora_common.h"

/****************************************************************************
 * Sender 
 ****************************************************************************/
void lora_p2p_raw_ackfhss_sender_process(lora_state_t *obj)
{
    uint8_t c;
    size_t rsz, rsz2;
    int ret;

    connect_verify_t con_verify;
    connect_ret_t con_ret;

    switch (obj->sub_state) {
    case 0x11: {
            // send CONNECTION_VERIFY head, front of data
            lora_p2p_downchannelnext(obj);

            con_verify.tick = obj->tick;
            con_verify.rcv_auth_key = obj->auth_key_obj;
            con_verify.down_freq_idx = obj->down_freq_idx;
            con_verify.up_freq_idx = obj->up_freq_idx;
            rsz = encode_connect_verify(&obj->rf_read[0], &con_verify);

            rsz2 = (*obj->hp)(&obj->rf_read[0]+rsz, obj->max_payload-rsz);
            if (rsz2 > 0) {

                obj->tx_done = false;
                Radio.Send(&obj->rf_read[0], rsz+rsz2);

                obj->sub_state = 0x12;

                obj->msg_nbytes_send = rsz2;

                obj->pack_ack_completed_t1 = LORA_TIMESTAMP_GET();
            }
        }
        break;
    case 0x12: {
            if (obj->tx_done && 
                board_subghz_tx_ready()) {

                while (Radio.GetStatus() != RF_IDLE);
                rb_reset(&obj->rf_rxbuf);
                Radio.Rx(0);

                obj->sub_state = 0x13;
                obj->ack_timeout_timestamp = LORA_TIMESTAMP_GET();
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
                        obj->crc_error_cnt++;
                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        break;
                    }
                case 0: {
                        ret = decode_connect_result(&obj->rf_rtcm, &con_ret);

                        if (ret == 0 && con_ret.snd_auth_key == obj->auth_key_board) {

                            if (obj->tick != con_ret.tick) {
                                obj->msg_nbytes_total_lost += obj->msg_nbytes_send;
                            } else {
                                obj->msg_nbytes_total_snd += obj->msg_nbytes_send;
                            }

                            obj->sub_state = 0x14;
                            obj->ack_timeout_cnter = 0;
                            if (obj->tick >= 255) {
                                obj->tick = 0;
                                obj->tick_h++;
                            } else {
                                obj->tick++;
                            }

                            obj->pack_ack_completed_time = LORA_TIMESTAMP_GET() - obj->pack_ack_completed_t1;

                            LORAP2P_DEBUG("Ack drssi:%d, urssi:%d, snr:%d, snd:%d tsnd:%d, tlost:%d tacktime:%d, next-freq:%d\r\n",
                                con_ret.rssi, obj->rssi, obj->snr, obj->msg_nbytes_send, 
                                obj->msg_nbytes_total_snd, obj->msg_nbytes_total_lost,
                                obj->pack_ack_completed_time, obj->region_grp.channels[obj->down_freq_idx]);

                            memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                            return;

                        } else if (ret == 1) {
                            // LORAP2P_DEBUG("Allow ERROR %d %d\n", ret, f_reqallow.typid);
                        } else if (ret == 0) {
                            LORAP2P_DEBUG("Ack AuthKey Error, %d %x %x\r\n", ret, 
                                con_ret.snd_auth_key, obj->auth_key_board);
                        }

                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        break;
                    }
                }
            }

            if (board_elapsed_tick(obj->ack_timeout_timestamp) > LORAP2P_ACK_TIMEOUT) {
                // re enter
                obj->ack_timeout_cnter++;

                if (obj->ack_timeout_cnter > LORAP2P_ACK_TIMEOUT_MAX_TRY_TIMES) {
                    obj->ack_timeout_cnter = 0;
                    LORAP2P_DEBUG("Ack Link Error, Return to Find\r\n");

                    lora_p2p_state_to_linkfind(obj);

                } else {

                    LORAP2P_DEBUG("Ack Retry, try times: %d \r\n", obj->ack_timeout_cnter);

                    obj->sub_state = 0x11;
                    return;
                    // lora_p2p_state_to_link_established(obj);
                }
            }
        }
        break;
    case 0x14: {
            Radio.Standby();
            while (Radio.GetStatus() != RF_IDLE);
            Radio.SetChannel(obj->region_grp.channels[obj->down_freq_idx].freq);

            obj->sub_state = 0x11;
        }
        break;
    default: break;
    }
}

/****************************************************************************
 * Receiver 
 ****************************************************************************/
void lora_p2p_raw_ackfhss_receiver_process(lora_state_t *obj)
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
                        obj->crc_error_cnt++;
                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        break;
                    }
                case 0: {

                        ret = decode_connect_verify(&obj->rf_rtcm, &con_verify);

                        if (ret == 0 && con_verify.rcv_auth_key == obj->auth_key_board) {
                            obj->down_freq_idx = con_verify.down_freq_idx;
                            obj->tick = con_verify.tick;
                            obj->sub_state = 0x12;

                            memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                            return;

                        } else if (ret == 1) {
                            // LORAP2P_DEBUG("Allow ERROR %d %d\n", ret, f_reqallow.typid);
                        } else if (ret == 0) {
                            LORAP2P_DEBUG("Verify Auth-Key Error, %d %x %x\r\n", ret, 
                                con_verify.rcv_auth_key, obj->auth_key_board);
                        }

                        memset(&obj->rf_rtcm, 0, sizeof(rtcm_t));
                        break;
                    }
                }
            }

            if (obj->msg_nbytes_total_recv > 0 &&
                board_elapsed_tick(obj->link_failed_timestamp) > LORAP2P_LINK_FAIL_TIMEOUT) {
                // re-enter
                lora_p2p_state_to_linkfind(obj);
            }

        }
        break;
    case 0x12: {
            rsz = rb_read(&obj->rf_rxbuf, &obj->rf_read[0], obj->max_payload);
            if (rsz > 0) {
                (*obj->hp)(&obj->rf_read[0], rsz);

                obj->msg_nbytes_total_recv += rsz;
                obj->sub_state = 0x13;
            }
        }
        break;
    case 0x13: {
            // send CONNECTION_RESULT
            con_ret.rssi = obj->rssi;
            con_ret.snr = obj->snr;
            con_ret.snd_auth_key = obj->auth_key_obj;
            con_ret.tick = obj->tick;
            rsz = encode_connect_result(&obj->rf_rtcm.buff[0], &con_ret);

            Radio.Standby();
            while (Radio.GetStatus() != RF_IDLE);

            obj->tx_done = false;
            Radio.Send(&obj->rf_rtcm.buff[0], rsz);

            obj->sub_state = 0x14;

            LORAP2P_DEBUG("Send Ack %d, total rcv:%d, next freq: %d\r\n", 
                con_ret.tick, obj->msg_nbytes_total_recv,
                obj->region_grp.channels[obj->down_freq_idx]);
        }
        break;
    case 0x14: {
            // wait CONNECTION_RESULT send completed
            if (obj->tx_done && 
                board_subghz_tx_ready()) {

                Radio.Standby();
                while (Radio.GetStatus() != RF_IDLE);
                Radio.SetChannel(obj->region_grp.channels[obj->down_freq_idx].freq);

                rb_reset(&obj->rf_rxbuf);
                Radio.Rx(0);

                obj->sub_state = 0x11;
                obj->link_failed_timestamp = HAL_GetTick();
            }
        }
        break;
    default: break;
    }
}

void lora_p2p_raw_ackfhss_process(lora_state_t *obj)
{
    if (obj->role == LORA_SENDER) {
        lora_p2p_raw_ackfhss_sender_process(obj);
    } else if (obj->role == LORA_RECEIVER) {
        lora_p2p_raw_ackfhss_receiver_process(obj);
    }
}
