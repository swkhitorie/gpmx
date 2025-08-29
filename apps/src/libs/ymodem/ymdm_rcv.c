#include "ymdm_rcv.h"

static int ymodem_parse_dframe(struct __ymodem_rcv_parse *msg, uint8_t c, uint8_t type)
{
    int header = YMODEM_SOH;
    int payload_len = YMODEM_PAYLOADLEN_SOH;

    if (type == 1) {
        header = YMODEM_STX;
        payload_len = YMODEM_PAYLOADLEN_STX;
    }

    if (msg->nbyte == 0) {
        if (c != header) return -1;
        msg->buff[msg->nbyte++] = c;
        return -2;
    }

    msg->buff[msg->nbyte++] = c;

    if (msg->nbyte == 2) {
        msg->seq = msg->buff[1];
    }

    if (msg->nbyte == 3) {
        msg->seq_xor = msg->buff[2];
    }

    if (msg->nbyte < YMODEM_NONPAYLOADLEN + payload_len) {
        return -3;
    }

    msg->nbyte = 0;

    if (crc16_xmodem(&msg->buff[YMODEM_HEADLEN], payload_len+2) != 0) {
        return -4;
    }

    return 0;
}

static int ymodem_parser(struct __ymodem_receiver *yrcv, uint8_t c, int type)
{
    int ret = 0xff;

    switch (type) {
    case YMODEM_SOH:
        ret = ymodem_parse_dframe(&yrcv->proto, c, 0);
        break;
    case YMODEM_STX:
        ret = ymodem_parse_dframe(&yrcv->proto, c, 1);
        break;
    case YMODEM_EOT:
        ret = (c==YMODEM_EOT)?0:-5;
        break;
    }

    return ret;
}

static int ymodem_parser_buff(struct __ymodem_receiver *yrcv, const uint8_t *p, uint16_t len, int type)
{
    int i = 0, j;
    int ret = 0xff;
    for (i = 0; i < len; i++) {
        ret = ymodem_parser(yrcv, p[i], type);
        if (ret == 0) {
            return 0;
        }

        if (ret == -1) {
            memset(&yrcv->proto, 0, sizeof(struct __ymodem_rcv_parse));
        }
    }

    return ret;
}

void ymodem_cfg_snd_interface(struct __ymodem_receiver *yrcv, itf_ymodem_receiver_sndbyte intf)
{
    yrcv->_fsnd = intf;
}

void ymodem_rx_process(struct __ymodem_receiver *yrcv, const uint8_t *p, uint16_t len)
{
    int ret = 0xff;

    switch(yrcv->state) {

    case YMODEM_SYNC_WAIT:
        ret = ymodem_parser_buff(yrcv, p, len, YMODEM_SOH);
        if (ret == 0) {
            if (yrcv->proto.seq == 0x00 && yrcv->proto.seq_xor == 0xff) {
                if (0 == y_filename_size_get(
                    &yrcv->proto.buff[YMODEM_HEADLEN],
                    YMODEM_PAYLOADLEN_SOH,
                    &yrcv->fname[0],
                    &yrcv->fsize)) {

                    yrcv->_fsnd(YMODEM_ACK);
                    yrcv->_fsnd(YMODEM_C);
                    yrcv->state = YMODEM_RCV_PAYLOAD;
                    yrcv->seq_lst = 0;

                    YM_LOG("YMODEM Start: file: %s, sz: %d\r\n", yrcv->fname, yrcv->fsize);
                }
            }
        }
        break;

    case YMODEM_RCV_PAYLOAD:
        ret = ymodem_parser_buff(yrcv, p, len, YMODEM_STX);
        if (ret == 0) {
            if (((yrcv->proto.seq | yrcv->proto.seq_xor) == 0xff) &&
                ((yrcv->proto.seq - yrcv->seq_lst == 1) || (yrcv->proto.seq==0xff && yrcv->seq_lst==0x00))
                ) {
                yrcv->seq_lst = yrcv->proto.seq;
                yrcv->fsize_cal += YMODEM_PAYLOADLEN_STX;

                YM_LOG("ACK Payload %d \r\n", yrcv->fsize_cal);
                yrcv->_fsnd(YMODEM_ACK);

                if (yrcv->fsize_cal >= yrcv->fsize) {
                    yrcv->state = YMODEM_RCV_EOTA;

                    YM_LOG("Final, Wait EOT\r\n");
                }
            } else {

            }
        } else {

        }
        break;

    case YMODEM_RCV_EOTA:
        ret = ymodem_parser_buff(yrcv, p, len, YMODEM_EOT);
        if (ret == 0) {
            yrcv->_fsnd(YMODEM_NAK);
            yrcv->state = YMODEM_RCV_EOTB;

            YM_LOG("NAK\r\n");
        }
        break;

    case YMODEM_RCV_EOTB:
        ret = ymodem_parser_buff(yrcv, p, len, YMODEM_EOT);
        if (ret == 0) {
            yrcv->_fsnd(YMODEM_ACK);
            yrcv->_fsnd(YMODEM_C);
            yrcv->state = YMODEM_RCV_ENDTRANSMISSION;

            YM_LOG("ACK C\r\n");
        }
        break;

    case YMODEM_RCV_ENDTRANSMISSION:
        ret = ymodem_parser_buff(yrcv, p, len, YMODEM_SOH);
        if (ret == 0) {
            yrcv->_fsnd(YMODEM_ACK);
            yrcv->state = YMODEM_IDLE;

            YM_LOG("FINAL SOH\r\n");
        }
        break;
    }
}
