#include "rtkcmn.h"

static uint32_t glonass_to_gps_tow(uint32_t tod, uint16_t dow, uint8_t leap_sec)
{
    // GLONASS -> UTC
    uint32_t utc_seconds = tod + dow * 86400 - 10800;

    // UTC -> GPS time(+leap second)
    uint32_t gps_tow = utc_seconds + leap_sec;

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

static int decode_type1013(rtcm_t *rtcm)
{
    int i=24+12;

    i+=50;
    rtcm->sys_leapsec = getbitu(rtcm->buff,i, 8);

    return 1013;
}

static int decode_type1019(rtcm_t *rtcm)
{
    int i=24+12,prn,sat,week,sys=SYS_GPS;

    if (i+476<=rtcm->len*8) {
        prn       =getbitu(rtcm->buff,i, 6);              i+= 6;
        week      =getbitu(rtcm->buff,i,10);              i+=10;
    }

    return 1019;
}

static int decode_type1033(rtcm_t *rtcm)
{
    // do nothing
    return 1033;
}

/* decode type 1042/63: beidou ephemerides -----------------------------------*/
static int decode_type1042(rtcm_t *rtcm)
{
    int i=24+12,prn,sat,week,sys=SYS_CMP;


    if (i+499<=rtcm->len*8) {
        prn       =getbitu(rtcm->buff,i, 6);              i+= 6;
        week      =getbitu(rtcm->buff,i,13);              i+=13;
    }

    rtcm->bds_week = week;

    return 1042;
}

/* decode type 1046: galileo I/NAV satellite ephemerides (ref [17]) ----------*/
static int decode_type1046(rtcm_t *rtcm)
{
    int i=24+12,prn,week,sys=SYS_GAL;

    if (i+476<=rtcm->len*8) {
        prn       =getbitu(rtcm->buff,i, 6);  i+= 6;
        week      =getbitu(rtcm->buff,i,13);  i+=13;
    }

    return 1046;
}

static int decode_type1230(rtcm_t *rtcm)
{
    // do nothing
    return 1230;
}

static int decode_msm_head(rtcm_t *rtcm, int sys)
{
    double tow,tod;
    int i=24,dow,staid,type,sync;
    gtime_t bds_time;


    type=getbitu(rtcm->buff,i,12); i+=12;

    if (i+157<=rtcm->len*8) {
        staid     =getbitu(rtcm->buff,i,12);       i+=12;
        
        if (sys==SYS_GLO) {
            dow   =getbitu(rtcm->buff,i, 3);       i+= 3;
            tod   =getbitu(rtcm->buff,i,27)*0.001; i+=27;
            tow =glonass_to_gps_tow(tod, dow, 18);
        }
        else if (sys==SYS_CMP) {
            tow   =getbitu(rtcm->buff,i,30)*0.001; i+=30;
            tow+=14.0; /* BDT -> GPST */

            if (rtcm->bds_week > 0) {
                bds_time = bdt2time(rtcm->bds_week, rtcm->tow);
                rtcm->bds_now = bds_time.time - rtcm->sys_leapsec;

                if (rtcm->sys_leapsec > 0 &&
                    rtcm->sys_leapsec < 30) {
                    rtcm->bds_time_sync = SYS_CMP;
                }
            }
        }
        else {
            tow   =getbitu(rtcm->buff,i,30)*0.001; i+=30;
        }
        sync      =getbitu(rtcm->buff,i, 1);       i+= 1;

    } else {
        return -11;
    }

    rtcm->tow = tow;
    rtcm->sys = sys;
    rtcm->sync = sync;

    return type;
}

static int decode_msm0(rtcm_t *rtcm, int sys)
{
    return decode_msm_head(rtcm,sys);
}

static int decode_msm4(rtcm_t *rtcm, int sys)
{
    return decode_msm_head(rtcm,sys);
}

static int decode_msm5(rtcm_t *rtcm, int sys)
{
    return decode_msm_head(rtcm,sys);
}

static int decode_msm6(rtcm_t *rtcm, int sys)
{
    return decode_msm_head(rtcm,sys);
}

static int decode_msm7(rtcm_t *rtcm, int sys)
{
    return decode_msm_head(rtcm,sys);
}

int decode_rtcm3(rtcm_t *rtcm)
{
    int ret=0xff;
    int type=getbitu(rtcm->buff,24,12);

    switch (type) {

        case 999:  ret=decode_typest(rtcm);   break;
        case 1005: ret=decode_type1005(rtcm); break;
        case 1006: ret=decode_type1006(rtcm); break;
        case 1013: ret=decode_type1013(rtcm); break;
        case 1019: ret=decode_type1019(rtcm); break;
        case 1033: ret=decode_type1033(rtcm); break;
        case 1042: ret=decode_type1042(rtcm); break;
        case 1046: ret=decode_type1046(rtcm); break;

        case 1071: ret=decode_msm0(rtcm,SYS_GPS); break; /* not supported */
        case 1072: ret=decode_msm0(rtcm,SYS_GPS); break; /* not supported */
        case 1073: ret=decode_msm0(rtcm,SYS_GPS); break; /* not supported */
        case 1074: ret=decode_msm4(rtcm,SYS_GPS); break;
        case 1075: ret=decode_msm5(rtcm,SYS_GPS); break;
        case 1076: ret=decode_msm6(rtcm,SYS_GPS); break;
        case 1077: ret=decode_msm7(rtcm,SYS_GPS); break;

        case 1081: ret=decode_msm0(rtcm,SYS_GLO); break; /* not supported */
        case 1082: ret=decode_msm0(rtcm,SYS_GLO); break; /* not supported */
        case 1083: ret=decode_msm0(rtcm,SYS_GLO); break; /* not supported */
        case 1084: ret=decode_msm4(rtcm,SYS_GLO); break;
        case 1085: ret=decode_msm5(rtcm,SYS_GLO); break;
        case 1086: ret=decode_msm6(rtcm,SYS_GLO); break;
        case 1087: ret=decode_msm7(rtcm,SYS_GLO); break;

        case 1091: ret=decode_msm0(rtcm,SYS_GAL); break; /* not supported */
        case 1092: ret=decode_msm0(rtcm,SYS_GAL); break; /* not supported */
        case 1093: ret=decode_msm0(rtcm,SYS_GAL); break; /* not supported */
        case 1094: ret=decode_msm4(rtcm,SYS_GAL); break;
        case 1095: ret=decode_msm5(rtcm,SYS_GAL); break;
        case 1096: ret=decode_msm6(rtcm,SYS_GAL); break;
        case 1097: ret=decode_msm7(rtcm,SYS_GAL); break;

        case 1101: ret=decode_msm0(rtcm,SYS_SBS); break; /* not supported */
        case 1102: ret=decode_msm0(rtcm,SYS_SBS); break; /* not supported */
        case 1103: ret=decode_msm0(rtcm,SYS_SBS); break; /* not supported */
        case 1104: ret=decode_msm4(rtcm,SYS_SBS); break;
        case 1105: ret=decode_msm5(rtcm,SYS_SBS); break;
        case 1106: ret=decode_msm6(rtcm,SYS_SBS); break;
        case 1107: ret=decode_msm7(rtcm,SYS_SBS); break;

        case 1111: ret=decode_msm0(rtcm,SYS_QZS); break; /* not supported */
        case 1112: ret=decode_msm0(rtcm,SYS_QZS); break; /* not supported */
        case 1113: ret=decode_msm0(rtcm,SYS_QZS); break; /* not supported */
        case 1114: ret=decode_msm4(rtcm,SYS_QZS); break;
        case 1115: ret=decode_msm5(rtcm,SYS_QZS); break;
        case 1116: ret=decode_msm6(rtcm,SYS_QZS); break;
        case 1117: ret=decode_msm7(rtcm,SYS_QZS); break;

        case 1121: ret=decode_msm0(rtcm,SYS_CMP); break; /* not supported */
        case 1122: ret=decode_msm0(rtcm,SYS_CMP); break; /* not supported */
        case 1123: ret=decode_msm0(rtcm,SYS_CMP); break; /* not supported */
        case 1124: ret=decode_msm4(rtcm,SYS_CMP); break;
        case 1125: ret=decode_msm5(rtcm,SYS_CMP); break;
        case 1126: ret=decode_msm6(rtcm,SYS_CMP); break;
        case 1127: ret=decode_msm7(rtcm,SYS_CMP); break;

        case 1230: ret=decode_type1230(rtcm);     break; /* not supported */

    }

    return ret;
}
