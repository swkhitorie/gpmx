#ifndef GNSS_PPS_SYNC_H_
#define GNSS_PPS_SYNC_H_

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

void pps_pulse_irq();

void gnsspps_reset();
void gnsspps_afterpps_fstmsg();
void gnsspps_calibhrt_timestamp(time_t now);
void gnsspps_timestamp(struct timeval *now);
void gnsspps_recalib();

uint32_t gnss_get_dbg1();

uint32_t gnss_get_dbg2();

#ifdef __cplusplus
}
#endif



#endif
