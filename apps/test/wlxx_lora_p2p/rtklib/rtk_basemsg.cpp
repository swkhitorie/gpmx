#include "rtk_basemsg.h"
#include <string.h>
#include <stdio.h>

void rtk_basemsg_decode(rtk_basemsg_t *baseobj, rtcm_t *rtcm, uint8_t *buffer, uint32_t len)
{
    int i = 0, ret;
    for (i = 0; i < len; i++) {
        ret = input_rtcm3(rtcm, buffer[i]);
        switch (ret) {
        case -1: break;
        case -2: break;
        case -3: memset(rtcm, 0, sizeof(rtcm_t)); break;
        case 0: {
                ret = decode_rtcm3(rtcm);

                if (ret==1005 || ret==1006) {
                    baseobj->len_arp = rtcm->len+3;
                    memcpy(baseobj->msg_arp, rtcm->buff, baseobj->len_arp);
                }

                if (ret==1013) {
                    baseobj->len_1013 = rtcm->len+3;
                    memcpy(baseobj->msg_1013, rtcm->buff, baseobj->len_1013);
                }

                if (rtcm->sys!=SYS_NONE) {
                    rtk_basemsg_msm_decode(baseobj, rtcm);
                }

                memset(rtcm, 0, sizeof(rtcm_t));
                break;
            }

        }
    }
}

void rtk_basemsg_msm_decode(rtk_basemsg_t *baseobj, rtcm_t *rtcm)
{
    int clen = baseobj->decoding_msm.len;
    int itow = rtcm->tow;
    uint8_t *msmbuffer = &baseobj->decoding_msm.buffer[clen];

    int ilen = rtcm->len+3;
    memcpy(msmbuffer, &rtcm->buff, ilen);
    if (baseobj->decoding_msm.tow == 0) {
        baseobj->decoding_msm.tow = itow;
        baseobj->decoding_msm.len += ilen;

        if (rtcm->sync == 0) {
            basemsmbuffer_fifoin(&baseobj->decoding_msm, &baseobj->msg[0], RTK_BASEMSG_BUFFER_LEN);

            memset(&baseobj->decoding_msm, 0, sizeof(rtk_msm_t));
            basemsmbuffer_print(&baseobj->msg[0], RTK_BASEMSG_BUFFER_LEN);
        }
    } else if (baseobj->decoding_msm.tow < itow) {
        // error1
        memset(&baseobj->decoding_msm, 0, sizeof(rtk_msm_t));
        // BOARD_DEBUG("Tow Error1\r\n");
    } else if (baseobj->decoding_msm.tow > itow) {
        // error2
        // BOARD_DEBUG("Tow Error2\r\n");
    } else if (baseobj->decoding_msm.tow == itow) {
        baseobj->decoding_msm.len += ilen;
        if (rtcm->sync == 0) {
            basemsmbuffer_fifoin(&baseobj->decoding_msm, &baseobj->msg[0], RTK_BASEMSG_BUFFER_LEN);

            memset(&baseobj->decoding_msm, 0, sizeof(rtk_msm_t));
            basemsmbuffer_print(&baseobj->msg[0], RTK_BASEMSG_BUFFER_LEN);
        }
    }
}


/****************************************************************************
 * Basestation MSM Buffer Handle 
 ****************************************************************************/
uint8_t basemsmbuffer_size(rtk_msm_t *buff, uint8_t capacity)
{
    uint8_t sz = 0;
    for (int i = 0; i < capacity; i++) {
        if (buff[i].tow != 0) {
            sz++;
        }
    }

    return sz;
}

void basemsmbuffer_clear(rtk_msm_t *buff, uint8_t capacity)
{
    for (int i = 0; i < capacity; i++) {
        memset(&buff[i], 0, sizeof(rtk_msm_t));
    }
}

void basemsmbuffer_peek(rtk_msm_t *buff, uint8_t capacity)
{
    int size = 0;
    int i = 0;

    size = basemsmbuffer_size(buff, capacity);

    memset(&buff[0], 0, sizeof(rtk_msm_t));
    for (int i=1; i<size; i++) {
        memcpy(&buff[i-1], &buff[i], sizeof(rtk_msm_t));
    }
    memset(&buff[size-1], 0, sizeof(rtk_msm_t));
}

bool basemsmbuffer_add(rtk_msm_t *node, rtk_msm_t *buff, uint8_t capacity)
{
    int idx = 0;
    for (idx = 0; idx < capacity; idx++) {
        if (buff[idx].tow == 0) {
            memcpy(&buff[idx], node, sizeof(rtk_msm_t));
            break;
        }
    }

    if (idx == capacity) {
        return false;
    }

    return true;
}

bool basemsmbuffer_fifoin(rtk_msm_t *node, rtk_msm_t *buff, uint8_t capacity)
{
    if (capacity == basemsmbuffer_size(buff, capacity)) {
        basemsmbuffer_peek(buff, capacity);
    }

    basemsmbuffer_add(node, buff, capacity);
    return true;
}

bool basemsmbuffer_newest(rtk_msm_t *node, rtk_msm_t *buff, uint8_t capacity)
{
    int sz = basemsmbuffer_size(buff, capacity);

    if (sz == 0) {
        return false;
    }

    memcpy(node, &buff[sz-1], sizeof(rtk_msm_t));
    basemsmbuffer_clear(buff, capacity);
    return true;
}

bool basemsmbuffer_towmatch(rtk_msm_t *node, uint32_t tow, rtk_msm_t *buff, uint8_t capacity)
{
    int i;
    int i_match;
    int peek_times;
    int sz = basemsmbuffer_size(buff, capacity);

    if (sz == 0) {
        return false; //no data
    }

    for (i = 0; i < sz; i++) {
        if (buff[i].tow == tow) {
            i_match = i;
            break;
        }
    }

    if (i == sz) {
        return false; //no tow match data
    }

    memcpy(node, &buff[i_match], sizeof(rtk_msm_t));

    peek_times = i_match;

    for (int j = 0; j < peek_times; j++) {
        basemsmbuffer_peek(buff, capacity);
    }

    return true;
}

void basemsmbuffer_print(rtk_msm_t *buff, uint8_t capacity)
{
    printf("base msm: \r\n");
    for (int i = 0; i < capacity; i++) {
        printf("Tow: %d, Len: %d \r\n", buff[i].tow, buff[i].len);
    }

    printf("\r\n");
}

