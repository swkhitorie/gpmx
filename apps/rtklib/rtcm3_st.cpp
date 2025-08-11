#include "rtkcmn.h"

// GPS Start time：1980/1/6 00:00:00 UTC
#define GPS_EPOCH_START 315964800  // Unix timestamp 1980-01-06 00:00:00

int decode_st_epvt(rtcm_t *rtcm)
{
    int ret=0;
    int idx=24;

    uint8_t quality = getbitu(rtcm->buff,idx+38,4);
    uint8_t sat_use = getbitu(rtcm->buff,idx+46,8);
    uint8_t sat_view = getbitu(rtcm->buff,idx+54,8);
    uint8_t time_id = getbitu(rtcm->buff,idx+137,4);
    uint32_t epochtime = getbitu(rtcm->buff,idx+145,30);
    int week_number = getbitu(rtcm->buff,idx+175,16);
    int leap_sec = getbitu(rtcm->buff,idx+191,8);
    float epochnow;
    time_t total_sec;

    if (epochtime != 0x3FFFFFFF) {
        epochnow = epochtime*1e-3f;
        time_t total_sec = GPS_EPOCH_START + 
                        (week_number*7*24*3600) + 
                        (time_t)epochnow;
        total_sec -= leap_sec;
        rtcm->st.now.time = total_sec;
        rtcm->st.now.sec = (epochnow*1e3f - ((time_t)epochnow)*1e3f);
        ret=21;
    } else {
        ret=0xff;
    }

    rtcm->st.quality = quality;
    rtcm->st.sat_use = sat_use;
    rtcm->st.sat_view = sat_view;
    rtcm->st.time_id = time_id;
    rtcm->st.epoch_time = epochtime;
    rtcm->st.week_num = week_number;
    rtcm->st.leap_sec = leap_sec;

    return ret;
}

int decode_typest(rtcm_t *rtcm)
{
    int ret=0xff;
    int i=24+12;
    int subtype=getbitu(rtcm->buff,i,8);

    switch (subtype) {

        case 21: ret=decode_st_epvt(rtcm); break;

    }

    return ret;
}
