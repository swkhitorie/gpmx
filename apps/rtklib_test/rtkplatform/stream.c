#include <ctype.h>
#include "rtklib.h"

#include <device/serial.h>
#define dev_t  uart_dev_t *
static uart_dev_t default_null_port = {0};

#define TINTACT             200         /* period for stream active (ms) */

typedef struct __stream_serial {
    dev_t dev;
    int error;
    int state;
    lock_t lock;
} serial_t;

static int tirate = 1000;  /* averaging time for data rate (ms) */
#include <board_config.h>

static serial_t *openserial(const char *path, int mode, char *msg)
{
    serial_t *serial;

    tracet(3, "openserial: path=%s mode=%d\n", path, mode);

    if (!(serial = (serial_t *)rtkmalloc(sizeof(serial_t)))) {
        return NULL;
    }

    serial->dev = dn_bind(path);

    if (serial->dev == NULL) {
        serial->dev = &default_null_port;
        board_printf("%s bind to null \r\n", path);
        return serial;
    }
    board_printf("bind success %s \r\n", path);

    return serial;
}

static void closeserial(serial_t *serial)
{
    tracet(3, "closeserial: dev=%d\n", serial->dev);

    if (!serial) {
        return;
    }

    rtkfree(serial);
}

static int readserial(serial_t *serial, uint8_t *buff, int n, char *msg)
{
    int rsz = 0;

    tracet(4, "readserial: dev=%d n=%d\n", serial->dev, n);

    if (!serial->dev || serial->dev == &default_null_port) {
        // pr_debug(": serial->dev is NULL\n");
        return 0;
    }

    rsz = SERIAL_RDBUF(serial->dev, buff, n);

    tracet(5, "readserial: exit dev=%d nr=%d\n", serial->dev, n);

    return rsz;
}

/* write serial --------------------------------------------------------------*/
static int writeserial(serial_t *serial, uint8_t *buff, int n, char *msg)
{
    int ns = 0;

    tracet(3, "writeserial: dev=%d n=%d\n", serial->dev, n);

    if (!serial->dev || serial->dev == &default_null_port) {
        return 0;
    }

    SERIAL_SEND(serial->dev, (uint8_t *)buff, n);

    tracet(5, "writeserial: exit dev=%d ns=%d\n", serial->dev, ns);
    return ns;
}

static int stateserial(serial_t *serial)
{
    return !serial ? 0 : (serial->error ? -1 : 2);
}

extern void strinit(stream_t *stream)
{
    tracet(3, "strinit:\n");

    stream->type = 0;
    stream->mode = 0;
    stream->state = 0;
    stream->inb = stream->inr = stream->outb = stream->outr = 0;
    stream->tick_i = stream->tick_o = stream->tact = stream->inbt = stream->outbt = 0;

    initlock(stream->lock);

    stream->port = NULL;
    stream->path[0] = '\0';
    stream->msg [0] = '\0';
}

extern int stropen(stream_t *stream, int type, int mode, const char *path)
{
    tracet(3, "stropen: type=%d mode=%d path=%s\n", type, mode, path);

    stream->type = type;
    stream->mode = mode;

    stream->inb = stream->inr = stream->outb = stream->outr = 0;
    stream->tick_i = stream->tick_o = tickget();
    stream->inbt = stream->outbt = 0;
    stream->msg[0] = '\0';
    stream->port = NULL;

    switch (type) {
    case STR_SERIAL:
        stream->port = openserial(path, mode, stream->msg);
        break;
    default:
        stream->state = 0;
        return 1;
    }
    stream->state = !stream->port ? -1 : 1;
    return stream->port != NULL;
}

/* close stream ----------------------------------------------------------------
* close stream
* args   : stream_t *stream IO  stream
* return : none
*-----------------------------------------------------------------------------*/
extern void strclose(stream_t *stream)
{
    tracet(3, "strclose: type=%d mode=%d\n", stream->type, stream->mode);

    strlock(stream);

    if (stream->port) {
        switch (stream->type) {
            case STR_SERIAL:
            closeserial((serial_t *)stream->port);
            break;
        }
    } else {
        trace(3, "no port to close stream: type=%d\n", stream->type);
    }

    stream->type = 0;
    stream->mode = 0;
    stream->state = 0;
    stream->inr = stream->outr = 0;
    stream->path[0] = '\0';
    stream->msg[0] = '\0';
    stream->port = NULL;

    strunlock(stream);
}

extern void strsync(stream_t *stream1, stream_t *stream2)
{

}

extern void strlock(stream_t *stream)
{
    lock(stream->lock);
}

extern void strunlock(stream_t *stream)
{
    unlock(stream->lock);
}

/* read stream -----------------------------------------------------------------
* read data from stream (unblocked)
* args   : stream_t *stream I  stream
*          unsinged char *buff O data buffer
*          int    n         I  maximum data length
* return : read data length
* notes  : if no data, return immediately with no data
*-----------------------------------------------------------------------------*/
extern int strread(stream_t *stream, uint8_t *buff, int n)
{
    uint32_t tick = tickget();
    char *msg = stream->msg;
    int nr = 0, tt;

    tracet(4, "strread: n=%d\n", n);

    strlock(stream);

    switch (stream->type) {
    case STR_SERIAL:
        nr = readserial((serial_t *)stream->port, buff, n, msg);
        break;
    default:
        strunlock(stream);
        return 0;
    }

    if (nr > 0) {
        stream->inb += nr;
        stream->tact = tick;
    }

    tt = (int)(tick - stream->tick_i);
    if (tt >= tirate) {
        stream->inr =
            (uint32_t)((double)((stream->inb - stream->inbt) * 8) / (tt * 0.001));
        stream->tick_i = tick;
        stream->inbt = stream->inb;
    }

    strunlock(stream);
    return nr;
}

/* write stream ----------------------------------------------------------------
* write data to stream (unblocked)
* args   : stream_t *stream I   stream
*          unsinged char *buff I data buffer
*          int    n         I   data length
* return : status (0:error,1:ok)
* notes  : write data to buffer and return immediately
*-----------------------------------------------------------------------------*/
extern int strwrite(stream_t *stream, uint8_t *buff, int n)
{
    uint32_t tick = tickget();
    char *msg = stream->msg;
    int ns, tt;

    tracet(4, "strwrite: n=%d\n", n);

    if (!(stream->mode & STR_MODE_W) || !stream->port) { 
        return 0; 
    }

    strlock(stream);

    switch (stream->type) {
    case STR_SERIAL:
        // board_printf("here 1 %d \r\n", n);
        ns = writeserial((serial_t *)stream->port, buff, n, msg);
        break;
    default:
        strunlock(stream);
        return 0;
    }

    if (ns > 0) {
        stream->outb += ns;
        stream->tact = tick;
    }

    tt = (int)(tick - stream->tick_o);
    if (tt > tirate) {
        stream->outr =
            (uint32_t)((double)((stream->outb - stream->outbt) * 8) / (tt * 0.001));
        stream->tick_o = tick;
        stream->outbt = stream->outb;
    }

    strunlock(stream);
    return ns;
}

/* get stream status -----------------------------------------------------------
* get stream status
* args   : stream_t *stream I   stream
*          char   *msg      IO  status message (NULL: no output)
* return : status (-1:error,0:close,1:wait,2:connect,3:active)
*-----------------------------------------------------------------------------*/
extern int strstat(stream_t *stream, char *msg)
{
    int state = 0;

    tracet(4, "strstat:\n");

    strlock(stream);

    if (msg) {
        strncpy(msg, stream->msg, MAXSTRMSG - 1);
        msg[MAXSTRMSG - 1] = '\0';
    }

    if (!stream->port) {
        strunlock(stream);
        return stream->state;
    }

    switch (stream->type) {
    case STR_SERIAL:
        state = stateserial((serial_t *)stream->port);
        break;
    default:
        strunlock(stream);
        return 0;
    }

    if (state == 2 && (int)(tickget() - stream->tact) <= TINTACT) {
        state = 3; 
    }

    strunlock(stream);
    return state;
}

/* get stream statistics summary -----------------------------------------------
* get stream statistics summary
* args   : stream_t *stream I   stream
*          int    *inb      IO   bytes of input  (NULL: no output)
*          int    *inr      IO   bps of input    (NULL: no output)
*          int    *outb     IO   bytes of output (NULL: no output)
*          int    *outr     IO   bps of output   (NULL: no output)
* return : none
*-----------------------------------------------------------------------------*/
extern void strsum(stream_t *stream, int *inb, int *inr, int *outb, int *outr)
{
    tracet(4, "strsum:\n");

    strlock(stream);
    if (inb) { *inb = stream->inb; }
    if (inr) { *inr = stream->inr; }
    if (outb) { *outb = stream->outb; }
    if (outr) { *outr = stream->outr; }
    strunlock(stream);
}

/* set global stream options ---------------------------------------------------
* set global stream options
* args   : int    *opt      I   options
*              opt[0]= inactive timeout (ms) (0: no timeout)
*              opt[1]= interval to reconnect (ms)
*              opt[2]= averaging time of data rate (ms)
*              opt[3]= receive/send buffer size (bytes);
*              opt[4]= file swap margin (s)
*              opt[5]= reserved
*              opt[6]= reserved
*              opt[7]= reserved
* return : none
*-----------------------------------------------------------------------------*/
extern void strsetopt(const int *opt)
{
    tracet(3, "strsetopt: opt=%d %d %d %d %d %d %d %d\n", opt[0], opt[1], opt[2],
            opt[3], opt[4], opt[5], opt[6], opt[7]);

    tirate     = opt[2] < 100 ? 100 : opt[2]; /* >=0.1s */
}

extern gtime_t strgettime(stream_t *stream)
{
    gtime_t t = {0};
    return t;
}

extern void strsendnmea(stream_t *stream, const sol_t *sol)
{
    uint8_t buff[1024];
    int n;

    tracet(3, "strsendnmea: rr=%.3f %.3f %.3f\n", sol->rr[0], sol->rr[1], sol->rr[2]);

    n = outnmea_gga(buff, sol);
    strwrite(stream, buff, n);
}

extern void strsendcmd(stream_t *str, const char *cmd)
{
    const char *p = cmd, *q;
    char msg[1024], cmdend[] = "\r\n";
    int n;

    tracet(3, "strsendcmd: cmd=%s\n", cmd);

    for (;;) {
        for (q = p;; q++) {
            if (*q == '\r' || *q == '\n' || *q == '\0') { 
                break; 
            }
        }

        n = (int)(q - p);
        strncpy(msg, p, n);
        msg[n] = '\0';

        if (!*msg || *msg == '#')  {
            /* null or comment */
        } else if (*msg == '!')  {
            /* binary escape */
        } else {
            strcat(msg, cmdend);
            strwrite(str, (uint8_t *)msg, n + 2);
        }

        if (*q == '\0') {
            break; 
        } else {
            p = q + 1;
        } 
    }
}
