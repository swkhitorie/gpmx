#ifndef RTK_COMMON_H_
#define RTK_COMMON_H_

#include <stdint.h>
#include <time.h>

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

typedef struct {
    time_t time;
    double sec;
} gtime_t;

typedef struct __rtcm_t {        /* RTCM control struct type */
    int nbyte;          /* number of bytes in message buffer */ 
    int nbit;           /* number of bits in word buffer */ 
    int len;            /* message length (bytes) */
    unsigned char buff[1200]; /* message buffer */
    int type;
    int subtype;

    gtime_t time;       /* message time */
    gtime_t time_s;     /* message start time */
    double tow;

} rtcm_ut;

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

#ifdef __cplusplus
}
#endif


#endif
