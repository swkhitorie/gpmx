#ifndef GNSS_PPS_SYNC_H_
#define GNSS_PPS_SYNC_H_

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

void pps_pulse_irq(void);

void gnss_first_msg_handle(void);

/**
 * @param gnss_msg_utc_time: utc time, units in sec
 */
void gnss_timestamp_get_start_calib(time_t gnss_msg_utc_time);

void gnss_hrt_timestamp_get(struct timeval *now);

#ifdef __cplusplus
}
#endif



#endif
