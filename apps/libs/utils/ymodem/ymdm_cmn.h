#ifndef _YMODEM_COMMON_H_
#define _YMODEM_COMMON_H_

#include <stdint.h>

#define YMODEM_SOH              (0x01)
#define YMODEM_STX              (0x02)
#define YMODEM_EOT              (0x04)
#define YMODEM_ACK              (0x06)
#define YMODEM_NAK              (0x15)
#define YMODEM_CA               (0x18)
#define YMODEM_C                (0x43)
#define YMODEM_ABORT1           (0x41)
#define YMODEM_ABORT2           (0x61)

#define YMODEM_FILLMAGIC        (0x1A)

#define YMODEM_HEADLEN          (3)
#define YMODEM_CRCLEN           (2)
#define YMODEM_NONPAYLOADLEN    (YMODEM_HEADLEN+YMODEM_CRCLEN)

#define YMODEM_PAYLOADLEN_SOH   (128)
#define YMODEM_PAYLOADLEN_STX   (1024)

#define YMODEM_TYPE_SOH         (0x01)
#define YMODEM_TYPE_STX         (0x02)

struct __packet_ystx {
    uint8_t __pad[3];
    uint8_t header;
    uint8_t seq;
    uint8_t seqxor;
    uint8_t crca;
    uint8_t crcb;
    uint8_t buff[YMODEM_PAYLOADLEN_STX];
};

struct __packet_ysoh {
    uint8_t __pad[3];
    uint8_t header;
    uint8_t seq;
    uint8_t seqxor;
    uint8_t crca;
    uint8_t crcb;
    uint8_t buff[YMODEM_PAYLOADLEN_SOH];
};

#ifdef __cplusplus
extern "C" {
#endif

unsigned short crc16_xmodem(unsigned char *data,unsigned int datalen);
int y_atoi(const char* nptr);
int y_filename_size_get(const unsigned char *p, int len, char* fname, unsigned int *fsize);

#ifdef __cplusplus
}
#endif

#endif
