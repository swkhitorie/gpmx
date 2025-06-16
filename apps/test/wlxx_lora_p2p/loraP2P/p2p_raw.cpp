#include "lora_common.h"

/**
 * 1.4k/s dataflow (raw mode)->
 * if sf=5, bw=500khz, pack TOA=32ms, The radio-trans speed can catch up with dataflow speed
 * if sf=7, bw=250khz  pack TOA=189ms,The radio-trans speed slows down, unable to keep, data will lost
 */

/****************************************************************************
 * Sender 
 ****************************************************************************/
void lora_p2p_raw_sender_process(lora_state_t *obj)
{
    size_t rsz;
    switch (obj->sub_state) {
    case 0x11:
        {
            size_t rsz = (*obj->hp)(&obj->rf_read[0], obj->max_payload);
            if (rsz > 0) {

                obj->tx_done = false;
                Radio.Send(&obj->rf_read[0], rsz);
                obj->sub_state = 0x12;
            }
        }
        break;
    case 0x12:
        {
            if (obj->tx_done && 
                board_subghz_tx_ready()) {
                obj->sub_state = 0x11;
            }
        }
        break;

    default: break;
    }
}

/****************************************************************************
 * Receiver 
 ****************************************************************************/
void lora_p2p_raw_receiver_process(lora_state_t *obj)
{
    size_t rsz;
    switch (obj->sub_state) {
    case 0x11:
        {
            rsz = rb_read(&obj->rf_rxbuf, &obj->rf_read[0], obj->max_payload);
            if (rsz > 0) {

                // char tmp[50];
                // sprintf(tmp, "\r\n rssi: %d \r\n", obj->rssi);
                // (*obj->hp)((uint8_t *)tmp, strlen(tmp));

                (*obj->hp)(&obj->rf_read[0], rsz);
            }
        }
        break;

    default: break;
    }
}

void lora_p2p_raw_process(lora_state_t *obj)
{
    if (obj->role == LORA_SENDER) {
        lora_p2p_raw_sender_process(obj);
    } else if (obj->role == LORA_RECEIVER) {
        lora_p2p_raw_receiver_process(obj);
    }
}
