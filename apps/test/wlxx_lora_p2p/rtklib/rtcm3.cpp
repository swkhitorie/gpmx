#include "rtkcmn.h"

static uint32_t glonass_to_gps_tow(uint32_t tod, uint16_t dow)
{
    // GLONASS -> UTC
    uint32_t utc_seconds = tod + dow * 86400 - 10800;

    // UTC -> GPS time(+leap second)
    uint32_t gps_tow = utc_seconds + 18;

    return gps_tow % 604800;
}

static int decode_type1005(rtcm_t *rtcm)
{
    // do nothing
    return 1005;
}

static int decode_type1006(rtcm_t *rtcm)
{
    // do nothing
    return 1006;
}

static int decode_type1033(rtcm_t *rtcm)
{
    // do nothing
    return 1033;
}

static int decode_msm_head(rtcm_t *rtcm, int sys)
{
    double tow,tod;
    int i=24,dow,staid,type;

    type=getbitu(rtcm->buff,i,12); i+=12;

    if (i+157<=rtcm->len*8) {
        staid     =getbitu(rtcm->buff,i,12);       i+=12;
        
        if (sys==SYS_GLO) {
            dow   =getbitu(rtcm->buff,i, 3);       i+= 3;
            tod   =getbitu(rtcm->buff,i,27)*0.001; i+=27;
            tow=glonass_to_gps_tow(tod, dow);
        }
        else if (sys==SYS_CMP) {
            tow   =getbitu(rtcm->buff,i,30)*0.001; i+=30;
            tow+=14.0; /* BDT -> GPST */
        }
        else {
            tow   =getbitu(rtcm->buff,i,30)*0.001; i+=30;
        }
    } else {
        return -11;
    }

    rtcm->tow = tow;
    rtcm->sys = sys;

    return type;
}

static int decode_msm4(rtcm_t *rtcm, int sys)
{
    return decode_msm_head(rtcm, sys);
}

static int decode_msm5(rtcm_t *rtcm, int sys)
{
    return decode_msm_head(rtcm, sys);
}

static int decode_msm6(rtcm_t *rtcm, int sys)
{
    return decode_msm_head(rtcm, sys);
}

static int decode_msm7(rtcm_t *rtcm, int sys)
{
    return decode_msm_head(rtcm, sys);
}

int decode_rtcm3(rtcm_t *rtcm)
{
    int ret=0xff;
    int type=getbitu(rtcm->buff,24,12);

    switch (type) {
        case 1005: ret=decode_type1005(rtcm); break;
        case 1006: ret=decode_type1006(rtcm); break;
        case 1033: ret=decode_type1033(rtcm); break;

        case 1074: ret=decode_msm4(rtcm,SYS_GPS); break;
        case 1075: ret=decode_msm5(rtcm,SYS_GPS); break;
        case 1076: ret=decode_msm6(rtcm,SYS_GPS); break;
        case 1077: ret=decode_msm7(rtcm,SYS_GPS); break;

        case 1084: ret=decode_msm4(rtcm,SYS_GLO); break;
        case 1085: ret=decode_msm5(rtcm,SYS_GLO); break;
        case 1086: ret=decode_msm6(rtcm,SYS_GLO); break;
        case 1087: ret=decode_msm7(rtcm,SYS_GLO); break;

        case 1094: ret=decode_msm4(rtcm,SYS_GAL); break;
        case 1095: ret=decode_msm5(rtcm,SYS_GAL); break;
        case 1096: ret=decode_msm6(rtcm,SYS_GAL); break;
        case 1097: ret=decode_msm7(rtcm,SYS_GAL); break;

        case 1104: ret=decode_msm4(rtcm,SYS_SBS); break;
        case 1105: ret=decode_msm5(rtcm,SYS_SBS); break;
        case 1106: ret=decode_msm6(rtcm,SYS_SBS); break;
        case 1107: ret=decode_msm7(rtcm,SYS_SBS); break;

        case 1114: ret=decode_msm4(rtcm,SYS_QZS); break;
        case 1115: ret=decode_msm5(rtcm,SYS_QZS); break;
        case 1116: ret=decode_msm6(rtcm,SYS_QZS); break;
        case 1117: ret=decode_msm7(rtcm,SYS_QZS); break;

        case 1124: ret=decode_msm4(rtcm,SYS_CMP); break;
        case 1125: ret=decode_msm5(rtcm,SYS_CMP); break;
        case 1126: ret=decode_msm6(rtcm,SYS_CMP); break;
        case 1127: ret=decode_msm7(rtcm,SYS_CMP); break;
    }

    return ret;
}
