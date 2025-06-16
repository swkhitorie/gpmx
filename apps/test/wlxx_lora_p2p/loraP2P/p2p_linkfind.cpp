#include "lora_common.h"

/****************************************************************************
 * Receiver 
 ****************************************************************************/
static void lora_p2p_linkfind_recv(lora_state_t *obj)
{
    uint8_t c;
    size_t rsz;
    int ret;

    req_connect_t f_reqconnect;
    req_allow_t f_reqallow;

    switch (obj->sub_state) {
    case 0x01: {
            // start send REQUEST_CONNECT
            f_reqconnect.rcv_key = obj->id_key_board;
            util_id_set_to_array(&f_reqconnect.rcv_id[0], &obj->id_board[0]);
            rsz = encode_req_connect(&obj->rf_rtcm.buff[0], &f_reqconnect);

            obj->tx_done = false;
            Radio.Send(&obj->rf_rtcm.buff[0], rsz);

            obj->sub_state = 0x02;

            LORAP2P_DEBUG("P2P Wait Link-Allow\n");
        }
        break;
    case 0x02: {
            // wait REQUEST_CONNECT send completed, and turn to receiver REQUEST_ALLOW
            if (obj->tx_done && 
                board_subghz_tx_ready()) {

                LORAP2P_DEBUG("P2P Send Connect\n");
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
                        LORAP2P_DEBUG("CRC ERROR in Allow\n");
                        break;
                case 0:
                    {
                        ret = decode_req_allow(&obj->rf_rtcm, &f_reqallow);

                        if (ret == 0 &&
                            util_id_compare(&obj->id_board[0], &f_reqallow.rcv_id[0])) {

                            LORAP2P_DEBUG("Link-Find Completed OBJ-ID: %x %x %x\n",
                                *((uint32_t *)&f_reqallow.snd_id[0]),
                                *((uint32_t *)&f_reqallow.snd_id[4]),
                                *((uint32_t *)&f_reqallow.snd_id[8]));

                            util_id_set_to_uid(&obj->id_obj[0], &f_reqallow.snd_id[0]);
                            obj->sub_state = 0x04;
                            lora_p2p_state_to_link_established(obj);

                            return;
                        } else {
                            LORAP2P_DEBUG("Allow ERROR %d %d\n", ret, f_reqallow.typid);
                        }
                        break;
                    }
                }
            }

            if (board_elapsed_tick(obj->link_find_timestamp) > LORAP2P_WAIT_CONNECTING_TIMEOUT) {
                // re-enter
                lora_p2p_state_to_linkfind(obj);
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
static void lora_p2p_linkfind_send(lora_state_t *obj)
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
                        LORAP2P_DEBUG("CRC ERROR in Req\n");
                        break;
                case 0: {
                        ret = decode_req_connect(&obj->rf_rtcm, &f_reqconnect);

                        if (ret == 0) {
                            obj->sub_state = 0x02;

                            util_id_set_to_uid(&obj->id_obj[0], &f_reqconnect.rcv_id[0]);
                            obj->id_key_obj = f_reqconnect.rcv_key;

                            LORAP2P_DEBUG("OBJ-ID: %x %x %x\n",
                                *((uint32_t *)&f_reqconnect.rcv_id[0]),
                                *((uint32_t *)&f_reqconnect.rcv_id[4]),
                                *((uint32_t *)&f_reqconnect.rcv_id[8]));
                        }
                        break;
                    }
                }
            }

            if (board_elapsed_tick(obj->link_find_timestamp) > LORAP2P_WAIT_CONNECTING_TIMEOUT) {
                // re-enter
                lora_p2p_state_to_linkfind(obj);
            }
        }
        break;
    case 0x02:
        {
            // send REQUEST_ALLOW
            f_reqallow.rcv_key = obj->id_key_obj;
            f_reqallow.snd_key = obj->id_key_board;
            util_id_set_to_array(&f_reqallow.snd_id[0], &obj->id_board[0]);
            util_id_set_to_array(&f_reqallow.rcv_id[0], &obj->id_obj[0]);
            rsz = encode_req_allow(&obj->rf_rtcm.buff[0], &f_reqallow);

            Radio.Standby();
            while (Radio.GetStatus() != RF_IDLE);

            obj->tx_done = false;
            Radio.Send(&obj->rf_rtcm.buff[0], rsz);

            obj->sub_state = 0x03;
        }
        break;
    case 0x03:
        {
            if (obj->tx_done && 
                board_subghz_tx_ready()) {

                LORAP2P_DEBUG("Link-Find Completed \r\n");

                obj->sub_state = 0x04;
                lora_p2p_state_to_link_established(obj);
            }
        }
        break;
    case 0x04: 
        //idle, never run here
        break;
    default:break;
    }
}

void lora_p2p_linkfind(lora_state_t *obj)
{
    if (obj->role == LORA_SENDER) {
        lora_p2p_linkfind_send(obj);
    } else if (obj->role == LORA_RECEIVER) {
        lora_p2p_linkfind_recv(obj);
    }
}
