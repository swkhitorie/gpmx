#ifndef RTCM3_ST_PROPRIETARY_H_
#define RTCM3_ST_PROPRIETARY_H_

#include <stdint.h>
#include <time.h>

typedef struct __timeg {
    time_t time;        /* time (s) expressed by standard time_t from 1970-01-01 */
    double sec;         /* fraction of second under 1 s */
} ntime_t;

struct rtcm3_st_proprietary {
    ntime_t now;

    uint8_t time_id;
    uint32_t epoch_time;
    uint16_t week_num;
    uint8_t leap_sec;

    uint8_t sat_use;
    uint8_t sat_view;
    uint8_t quality;
};


#endif
