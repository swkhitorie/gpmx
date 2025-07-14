#include "p2p_proto.h"

/****************************************************************************
 * Request-Connect 
 ****************************************************************************/
int decode_req_connect(rtcm_t *rtcm, req_connect_t *req_connect)
{
    int ret = 1;
    int payload_len = 17;

    int typid = getbitu(rtcm->buff, 24, 8);

    for (int i = 0; i < 12; i++) {
        req_connect->rcv_id[i] = rtcm->buff[4+i];
    }

    req_connect->rcv_key = getbitu(rtcm->buff, 128, 32);

    if (typid == 0x12 && rtcm->len == (payload_len+3)) {
        ret = 0;
    }

    return ret;
}

int encode_req_connect(uint8_t *p, req_connect_t *req_connect)
{
    int payload_len = 17;

    p[0] = RTCM3PREAMB;
    p[1] = 0x00;
    setbitu(&p[0], 14, 10, payload_len);

    setbitu(&p[0], 24, 8, 0x12); //req_connect->typid
    for (int i = 0; i < 12; i++) {
        p[4+i] = req_connect->rcv_id[i];
    }
    setbitu(&p[0], 128, 32, req_connect->rcv_key);

    setbitu(&p[0], 160, 24, 
        rtk_crc24q(&p[0], payload_len+3));

    return payload_len + 6;
}

/****************************************************************************
 * Request-Allow 
 ****************************************************************************/
int decode_req_allow(rtcm_t *rtcm, req_allow_t *req_allow)
{
    int ret = 1;
    int payload_len = 35;

    int typid = getbitu(rtcm->buff, 24, 8);
    req_allow->typid = typid;
    req_allow->down_freq_idx = getbitu(rtcm->buff, 32, 8);
    req_allow->up_freq_idx = getbitu(rtcm->buff, 40, 8);

    for (int i = 0; i < 12; i++) {
        req_allow->rcv_id[i] = rtcm->buff[6+i];
    }
    for (int i = 0; i < 12; i++) {
        req_allow->snd_id[i] = rtcm->buff[18+i];
    }

    req_allow->rcv_key = getbitu(rtcm->buff, 240, 32);
    req_allow->snd_key = getbitu(rtcm->buff, 272, 32);

    if (typid == 0x14 && rtcm->len == (payload_len+3)) {
        ret = 0;
    }

    return ret;
}

int encode_req_allow(uint8_t *p, req_allow_t *req_allow)
{
    int payload_len = 35;

    p[0] = RTCM3PREAMB;
    p[1] = 0x00;
    setbitu(&p[0], 14, 10, payload_len);

    setbitu(&p[0], 24, 8, 0x14); //req_allow->typid
    setbitu(&p[0], 32, 8, req_allow->down_freq_idx);
    setbitu(&p[0], 40, 8, req_allow->up_freq_idx);
    for (int i = 0; i < 12; i++) {
        p[6+i] = req_allow->rcv_id[i];
    }
    for (int i = 0; i < 12; i++) {
        p[18+i] = req_allow->snd_id[i];
    }
    setbitu(&p[0], 240, 32, req_allow->rcv_key);
    setbitu(&p[0], 272, 32, req_allow->snd_key);

    setbitu(&p[0], 304, 24, 
        rtk_crc24q(&p[0], payload_len+3));

    return payload_len + 6;
}

/****************************************************************************
 * Connect-Verify 
 ****************************************************************************/
int decode_connect_verify(rtcm_t *rtcm, connect_verify_t *con_verify)
{
    int ret = 1;
    int payload_len = 14;

    int typid = getbitu(rtcm->buff, 24, 8);
    con_verify->down_freq_idx = getbitu(rtcm->buff, 32, 8);
    con_verify->up_freq_idx = getbitu(rtcm->buff, 40, 8);
    con_verify->rssi = getbits(rtcm->buff, 48, 16);
    con_verify->snr = getbits(rtcm->buff, 64, 8);
    con_verify->seq = getbitu(rtcm->buff, 72, 32);
    con_verify->rcv_auth_key = getbitu(rtcm->buff, 104, 32);
    con_verify->typid = typid;

    if (typid == 0x24 && rtcm->len == (payload_len+3)) {
        ret = 0;
    }

    return ret;
}

int encode_connect_verify(uint8_t *p, connect_verify_t *con_verify)
{
    int payload_len = 14;

    p[0] = RTCM3PREAMB;
    p[1] = 0x00;
    setbitu(&p[0], 14, 10, payload_len);

    setbitu(&p[0], 24, 8, 0x24);  // typeid
    setbitu(&p[0], 32, 8, con_verify->down_freq_idx);
    setbitu(&p[0], 40, 8, con_verify->up_freq_idx);
    setbits(&p[0], 48, 16, con_verify->rssi);
    setbits(&p[0], 64, 8, con_verify->snr);
    setbitu(&p[0], 72, 32, con_verify->seq);
    setbitu(&p[0], 104, 32, con_verify->rcv_auth_key);

    setbitu(&p[0], 136, 24, 
        rtk_crc24q(&p[0], payload_len+3));

    return payload_len + 6;
}

/****************************************************************************
 * Connect-Result 
 ****************************************************************************/
int decode_connect_result(rtcm_t *rtcm, connect_ret_t *con_ret)
{
    int ret = 1;
    int payload_len = 12;

    int typid = getbitu(rtcm->buff, 24, 8);
    con_ret->rssi = getbits(rtcm->buff, 32, 16);
    con_ret->snr = getbits(rtcm->buff, 48, 8);
    con_ret->seq = getbitu(rtcm->buff, 56, 32);
    con_ret->snd_auth_key = getbitu(rtcm->buff, 88, 32);
    con_ret->typid = typid;

    if (typid == 0x22 && rtcm->len == (payload_len+3)) {
        ret = 0;
    }

    return ret;
}

int encode_connect_result(uint8_t *p, connect_ret_t *con_ret)
{
    int payload_len = 12;

    p[0] = RTCM3PREAMB;
    p[1] = 0x00;
    setbitu(&p[0], 14, 10, payload_len);

    setbitu(&p[0], 24, 8, 0x22);  // subid
    setbits(&p[0], 32, 16, con_ret->rssi);
    setbits(&p[0], 48, 8, con_ret->snr);
    setbitu(&p[0], 56, 32, con_ret->seq);
    setbits(&p[0], 88, 32, con_ret->snd_auth_key);

    setbitu(&p[0], 120, 24, 
        rtk_crc24q(&p[0], payload_len+3));

    return payload_len + 6;
}

