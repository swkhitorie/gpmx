#ifndef RTK_COMMON_H_
#define RTK_COMMON_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "rtcm3_st.h"

#define RTCM3PREAMB (0xD3)

#define SYS_NONE    0x00                /* navigation system: none */
#define SYS_GPS     0x01                /* navigation system: GPS */
#define SYS_SBS     0x02                /* navigation system: SBAS */
#define SYS_GLO     0x04                /* navigation system: GLONASS */
#define SYS_GAL     0x08                /* navigation system: Galileo */
#define SYS_QZS     0x10                /* navigation system: QZSS */
#define SYS_CMP     0x20                /* navigation system: BeiDou */
#define SYS_IRN     0x40                /* navigation system: IRNSS */
#define SYS_LEO     0x80                /* navigation system: LEO */
#define SYS_ALL     0xFF                /* navigation system: all */

typedef struct __gtime {        /* time struct */
    time_t time;        /* time (s) expressed by standard time_t from 1970-01-01 */
    double sec;         /* fraction of second under 1 s */
} gtime_t;

typedef struct __rtcm_t {        /* RTCM control struct type */
    int nbyte;          /* number of bytes in message buffer */ 
    int nbit;           /* number of bits in word buffer */ 
    int len;            /* message length (bytes) */
    unsigned char buff[1200]; /* message buffer */

    int sys;
    int sync;
    double tow;

    int sys_leapsec;

    int bds_week;
    time_t bds_now;

    int bds_time_sync;

    /** GNSS RTCM3 ST Proprietary */
    struct rtcm3_st_proprietary st;

} rtcm_t;

#ifdef __cplusplus
extern "C" {
#endif

unsigned int rtk_crc24q(const unsigned char *buff, int len);
uint8_t      xor_sum_check(const uint8_t *buf);

unsigned int getbitu(const unsigned char *buff, int pos, int len);
int          getbits(const unsigned char *buff, int pos, int len);
void         setbitu(unsigned char *buff, int pos, int len, unsigned int data);
void         setbits(unsigned char *buff, int pos, int len, int data);
uint8_t      getbitscnt(uint16_t value);
void         getbitspos(uint16_t value, uint8_t *pos, uint8_t *len);

gtime_t  epoch2time(const double *ep);
void     time2epoch(gtime_t t, double *ep);
gtime_t  gpst2time(int week, double sec);
double   time2gpst(gtime_t t, int *week);
gtime_t  gst2time(int week, double sec);
double   time2gst(gtime_t t, int *week);
gtime_t  bdt2time(int week, double sec);
double   time2bdt(gtime_t t, int *week);

gtime_t  timeadd(gtime_t t, double sec);
double   timediff(gtime_t t1, gtime_t t2);
gtime_t  bdt2gpst(gtime_t t);
/**
 * return ->
 * -1: no head
 * -2: no enough data
 * -3: crc error
 *  0: success
 */
int  input_rtcm3(rtcm_t *rtcm, unsigned char data);
int  decode_rtcm3(rtcm_t *rtcm);
int  decode_typest(rtcm_t *rtcm);

#ifdef __cplusplus
}
#endif


#endif
