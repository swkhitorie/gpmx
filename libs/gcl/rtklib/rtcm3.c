#include "rtcm3.h"

int decode_msm7(rtcm_ut *rtcm, int sys)
{
    int staid,dow;
    double tow,tod;
    int i = 24;
    i += 12; //skip type

    if (i + 157 <= rtcm->len * 8) {
        staid = getbitu(rtcm->buff, i, 12); i += 12;

        if (sys == SYS_GLO) {
            dow =getbitu(rtcm->buff,i, 3); i+= 3;
            tod =getbitu(rtcm->buff,i,27)*0.001; i+=27;
            rtcm->tow = tod;
        } else if (sys == SYS_CMP) {
            tow  =getbitu(rtcm->buff,i,30)*0.001; i+=30;
            tow += 14.0; /* BDT -> GPST */
            rtcm->tow = tow;
        } else {
            tow  =getbitu(rtcm->buff,i,30)*0.001; i+=30;
            rtcm->tow = tow;
        }
    }

    return 0x10;
}

int decode_rtcm3(rtcm_ut *rtcm)
{
    int ret = 1;

    rtcm->type = getbitu(rtcm->buff,24,12);
    rtcm->subtype = getbitu(rtcm->buff,36,8);

    switch (rtcm->type) {
        case 1077: ret=decode_msm7(rtcm, SYS_GPS); break;
        case 1087: ret=decode_msm7(rtcm, SYS_GLO); break;
        case 1097: ret=decode_msm7(rtcm, SYS_GAL); break;
        case 1127: ret=decode_msm7(rtcm, SYS_CMP); break;
    }

    return ret;
}

int input_rtcm3(rtcm_ut *rtcm, unsigned char data)
{
    /* synchronize frame */
    if (rtcm->nbyte==0) {
        if (data!=RTCM3PREAMB) return 0;
        rtcm->buff[rtcm->nbyte++]=data;
        return 0;
    }
    rtcm->buff[rtcm->nbyte++]=data;

    if (rtcm->nbyte==3) {
        rtcm->len=getbitu(rtcm->buff,14,10)+3; /* length without parity */
    }
    if (rtcm->nbyte<3||rtcm->nbyte<rtcm->len+3) return -1;
    rtcm->nbyte=0;

    /* check parity */
    if (rtk_crc24q(rtcm->buff,rtcm->len)!=getbitu(rtcm->buff,rtcm->len*8,24)) {
        return -2;
    }

    /* decode rtcm3 message */
    return decode_rtcm3(rtcm);
}
