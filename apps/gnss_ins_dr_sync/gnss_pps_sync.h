#ifndef GNSS_PPS_SYNC_H_
#define GNSS_PPS_SYNC_H_

#include <stdint.h>
#include <time.h>

typedef struct __gnss_timestamp {
    time_t now;
    uint32_t subsec;
} gnss_time_t;

#ifdef __cplusplus
extern "C" {
#endif

void pps_pulse_irq(void);

void gnss_first_msg_handle(void);

/**
 * @param gnss_msg_utc_time: utc time, units in sec
 */
void gnss_timestamp_get_start_calib(time_t gnss_msg_utc_time);

void gnss_hrt_timestamp_get(gnss_time_t *now);

uint32_t gnss_subsec_get(struct timeval *tv);

#ifdef __cplusplus
}
#endif



#endif
