#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "rtklib.h"

#include "ff.h"
#include <gpm/serial/serial.h>

#define TINTACT             200      /* period for stream active (ms) */
#define TIMETAGH_LEN        64
#define DEFAULT_MEMBUF_SIZE 4096
#define MAXSTRPATH          1024
#define MAXSTRMSG           1024

#define MIN(x,y)            ((x)<(y)?(x):(y))

#define CONFIG_DISABLE_STREAM_TAG_TIME  (0)

/* 1: call f_sync when every writefile; 0: call f_sync only in close()/swap() */
#define STREAM_FATFS_SYNC_ON_WRITE  1   

typedef struct {
    FIL      fp;              /* data file */
    FIL      fp_tag;          /* data time tag file */
    FIL      fp_tmp;          /* temp swap data file */
    FIL      fp_tag_tmp;      /* temp swap data time tag file */
    int      fp_valid;
    int      fp_tag_valid;
    int      fp_tmp_valid;
    int      fp_tag_tmp_valid;
    char     path[MAXSTRPATH];     /* file path (including :: ) */
    char     openpath[MAXSTRPATH]; /* real file path  */
    int      mode;            /* STR_MODE_R / STR_MODE_W / STR_MODE_RW */
    int      timetag;         /* 0:off, 1:on */
    int      repmode;         /* 0:master, 1:slave */
    int      offset;          /* slave offset of  master tick (ms) */
    int      size_fpos;       /* 4 or 8 */
    gtime_t  time;            /* start (GPST) */
    gtime_t  wtime;           /* last write time (GPST) */
    uint32_t tick;            /* replay start tick */
    uint32_t tick_f;          /* tag first data tick */
    long     fpos_n;          /* end pos of next data */
    uint32_t tick_n;          /* next data tick */
    double   start;           /* start time offset (s) */
    double   speed;           /* replay speed */
    double   swapintv;        /* swap interval hour */
    int      tag_absolute;    /* tag = 1 -> absolute tick */
    lock_t   lock;
} file_t;

typedef struct __stream_serial {
    uart_dev_t * dev;
    int error;
    int state;
    lock_t lock;
} serial_t;

typedef struct {
    int state, wp, rp;
    int bufsize;
    lock_t lock;
    uint8_t *buf;
} membuf_t;

static int      buffsize    = 32768;
static int      tirate      = 1000;
static uint32_t tick_master = 0;
static int      fswapmargin = 30;

static BYTE fatfs_mode_from_rtklib(int mode)
{
    int has_r = (mode & STR_MODE_R) != 0;
    int has_w = (mode & STR_MODE_W) != 0;

    if (has_r) {
        return FA_READ | FA_OPEN_EXISTING;
    } else if (has_w) {
        return FA_WRITE | FA_CREATE_ALWAYS;
    }
    return 0;
}

static int fatfs_fopen(FIL *fp, const char *path, int mode)
{
    return f_open(fp, path, fatfs_mode_from_rtklib(mode)) == FR_OK;
}

static long fatfs_tell(FIL *fp) { return (long)f_tell(fp); }
static void fatfs_seek(FIL *fp, long pos) { f_lseek(fp, (FSIZE_t)pos); }
static long fatfs_size(FIL *fp) { return (long)f_size(fp); }

/* ------------------------------------------------------------------------ */
/* Interface - File                                                         */
/* ------------------------------------------------------------------------ */

static int openfile_(file_t *file, gtime_t time, char *msg)
{
    char tagpath[MAXSTRPATH + 4] = "";
    char tagh[TIMETAGH_LEN + 1]  = "";
    double time_sec;
    uint32_t time_time;
    uint64_t time_time2;
    UINT     nr = 0;

    tracet(3, "openfile_: path=%s time=%s\n", file->path, time_str(time, 0));

    file->time   = utc2gpst(timeget());
    file->tick   = file->tick_f = tickget();
    file->fpos_n = 0;
    file->tick_n = 0;

    if (!*file->path) {
        sprintf(msg, "empty file path not supported on FatFs");
        return 0;
    }

    /* keywork exchange: %Y %m %d %h %M %S */
    reppath(file->path, file->openpath, time, "", "");

    if (!fatfs_fopen(&file->fp, file->openpath, file->mode)) {
        sprintf(msg, "file open error: %s", file->openpath);
        tracet(1, "openfile: %s\n", msg);
        return 0;
    }

    file->fp_valid = 1;
    tracet(4, "openfile_: open file %s\n", file->openpath);

    sprintf(tagpath, "%s.tag", file->openpath);

    if (file->timetag) {
        if (!fatfs_fopen(&file->fp_tag, tagpath, file->mode)) {
            sprintf(msg, "tag open error: %s", tagpath);
            tracet(1, "openfile: %s\n", msg);
            f_close(&file->fp);
            file->fp_valid = 0;
            return 0;
        }
        file->fp_tag_valid = 1;
        tracet(4, "openfile_: open tag file %s\n", tagpath);

        if (file->mode & STR_MODE_R) {
            /* write tag head: HEADER(64) + TIME(4+8) */
            FRESULT fr;
            fr = f_read(&file->fp_tag, tagh, TIMETAGH_LEN, &nr);
            if (fr == FR_OK && nr == TIMETAGH_LEN &&
                f_read(&file->fp_tag, &time_time2, sizeof(time_time2), &nr) == FR_OK &&
                nr == sizeof(time_time) &&
                f_read(&file->fp_tag, &time_sec, sizeof(time_sec), &nr) == FR_OK &&
                nr == sizeof(time_sec)) {

                memcpy(&file->tick_f, tagh + TIMETAGH_LEN - 4,
                    sizeof(file->tick_f));
                file->time.time = (time_t)time_time;
                file->time.sec  = time_sec;
                file->wtime     = file->time;
            } else {
                /* tag head invalid -> no tag replay */
                file->tick_f = 0;
            }

#if CONFIG_DISABLE_STREAM_TAG_TIME
            {
                uint32_t first_tick = 0;
                UINT     nr_pre     = 0;
                long     tag_rec_start = fatfs_tell(&file->fp_tag); /* it should be 76 */

                if (f_read(&file->fp_tag, &first_tick,
                        sizeof(first_tick), &nr_pre) == FR_OK &&
                    nr_pre == sizeof(first_tick)) {

                    if (first_tick > 86400000U) {
                        file->tag_absolute = 1;
                        file->tick_f = first_tick;
                    } else {
                        file->tag_absolute = 0;
                    }

                    fatfs_seek(&file->fp_tag, tag_rec_start);
                }
            }
#endif
            /* calibrate system time by file start time */
            timeset(gpst2utc(file->time));
        } else {
            /* write tag head */
            UINT nw = 0;
            memset(tagh, 0, TIMETAGH_LEN);
            sprintf(tagh, "TIMETAG RTKLIB %s", VER_RTKLIB);
            memcpy(tagh + TIMETAGH_LEN - 4, &file->tick_f, sizeof(file->tick_f));
            time_time = (uint32_t)file->time.time;
            time_time2 = (uint64_t)file->time.time;
            time_sec  = file->time.sec;
            f_write(&file->fp_tag, tagh, TIMETAGH_LEN, &nw);
            // f_write(&file->fp_tag, &time_time, sizeof(time_time), &nw);
            f_write(&file->fp_tag, &time_time2, sizeof(time_time2), &nw);
            f_write(&file->fp_tag, &time_sec,  sizeof(time_sec),  &nw);
            f_sync(&file->fp_tag);
        }
    } else if (file->mode & STR_MODE_W) {
        /* no writing tag -> delete old tag */
        f_unlink(tagpath);
    }
    return 1;
}

static void closefile_(file_t *file)
{
    tracet(3, "closefile_: path=%s\n", file->path);

    if (file->fp_valid)         { f_close(&file->fp);         file->fp_valid         = 0; }
    if (file->fp_tag_valid)     { f_close(&file->fp_tag);     file->fp_tag_valid     = 0; }
    if (file->fp_tmp_valid)     { f_close(&file->fp_tmp);     file->fp_tmp_valid     = 0; }
    if (file->fp_tag_tmp_valid) { f_close(&file->fp_tag_tmp); file->fp_tag_tmp_valid = 0; }

    timereset();
}

static file_t *openfile(const char *path, int mode, char *msg)
{
    file_t *file;
    gtime_t time, time0 = {0};
    double  speed = 1.0, start = 0.0, swapintv = 0.0;
    char   *p;
    int     timetag = 0, size_fpos = 4;

    tracet(3, "openfile: path=%s mode=%d\n", path, mode);

    if (!(mode & (STR_MODE_R | STR_MODE_W))) {
        return NULL;
    }

    for (p = (char *)path; (p = strstr(p, "::")); p += 2) {
        if      (*(p + 2) == 'T') timetag = 1;
        else if (*(p + 2) == '+') sscanf(p + 2, "+%lf", &start);
        else if (*(p + 2) == 'x') sscanf(p + 2, "x%lf", &speed);
        else if (*(p + 2) == 'S') sscanf(p + 2, "S=%lf", &swapintv);
        else if (*(p + 2) == 'P') sscanf(p + 2, "P=%d",  &size_fpos);
    }

    if (start <= 0.0) {
        start = 0.0;
    }

    if (swapintv <= 0.0) {
        swapintv = 0.0;
    }

    if (!(file = (file_t *)rtkport_malloc(sizeof(file_t)))) {
        return NULL;
    }

    memset(file, 0, sizeof(file_t));
    strncpy(file->path, path, MAXSTRPATH - 1);
    file->path[MAXSTRPATH - 1] = '\0';

    if ((p = strstr(file->path, "::"))) {
        *p = '\0';
    }

    file->openpath[0] = '\0';
    file->mode        = mode;
    file->timetag     = timetag;
    file->repmode     = 0;
    file->offset      = 0;
    file->size_fpos   = size_fpos;
    file->time        = file->wtime = time0;
    file->tick        = file->tick_f = file->tick_n = 0;
    file->fpos_n      = 0;
    file->start       = start;
    file->speed       = speed;
    file->swapintv    = swapintv;
    file->tag_absolute = 0;
    initlock(&file->lock);

    time = utc2gpst(timeget());

    if (!openfile_(file, time, msg)) {
        rtkport_free(file);
        return NULL;
    }

    return file;
}

static void closefile(file_t *file)
{
    tracet(3, "closefile:\n");
    if (!file) {
        return;
    }
    closefile_(file);
    rtkport_free(file);
}

static void swapfile(file_t *file, gtime_t time, char *msg)
{
    char openpath[MAXSTRPATH];

    tracet(3, "swapfile: time=%s\n", time_str(time, 0));

    if (file->fp_tmp_valid || file->fp_tag_tmp_valid) {
        return;
    }

    reppath(file->path, openpath, time, "", "");
    if (!strcmp(openpath, file->openpath)) {
        tracet(2, "swapfile: no need to swap %s\n", openpath);
        return;
    }

    memcpy(&file->fp_tmp,     &file->fp,     sizeof(FIL));
    memcpy(&file->fp_tag_tmp, &file->fp_tag, sizeof(FIL));
    file->fp_tmp_valid     = file->fp_valid;
    file->fp_tag_tmp_valid = file->fp_tag_valid;
    file->fp_valid         = 0;
    file->fp_tag_valid     = 0;

    openfile_(file, time, msg);
}

static void swapclose(file_t *file)
{
    tracet(3, "swapclose:\n");

    if (file->fp_tmp_valid) { 
        f_close(&file->fp_tmp);
        file->fp_tmp_valid = 0; 
    }

    if (file->fp_tag_tmp_valid) { 
        f_close(&file->fp_tag_tmp);
        file->fp_tag_tmp_valid = 0;
    }
}

static int statefile(file_t *file) 
{ 
    return file ? 2 : 0; 
}

static int statexfile(file_t *file, char *msg)
{
    char *p = msg, tstr1[32], tstr2[32];
    int   state = file ? 2 : 0;

    p += sprintf(p, "file:\n");
    p += sprintf(p, "  state   = %d\n", state);
    if (!state) {
        return 0;
    }

    time2str(file->time,  tstr1, 3);
    time2str(file->wtime, tstr2, 3);
    p += sprintf(p, "  path    = %s\n",  file->path);
    p += sprintf(p, "  openpath= %s\n",  file->openpath);
    p += sprintf(p, "  mode    = %d\n",  file->mode);
    p += sprintf(p, "  timetag = %d\n",  file->timetag);
    p += sprintf(p, "  repmode = %d\n",  file->repmode);
    p += sprintf(p, "  offset  = %d\n",  file->offset);
    p += sprintf(p, "  time    = %s\n",  tstr1);
    p += sprintf(p, "  wtime   = %s\n",  tstr2);
    p += sprintf(p, "  tick    = %u\n",  (unsigned)file->tick);
    p += sprintf(p, "  tick_f  = %u\n",  (unsigned)file->tick_f);
    p += sprintf(p, "  start   = %.3f\n", file->start);
    p += sprintf(p, "  speed   = %.3f\n", file->speed);
    p += sprintf(p, "  swapintv= %.3f\n", file->swapintv);
    return state;
}

static int readfile(file_t *file, uint8_t *buff, int nmax, char *msg)
{
    uint64_t fpos_8B;
    uint32_t t, fpos_4B, tn = 0;
    UINT     nr_uint = 0;
    long     pos, n;
    int      nr = 0;

    tracet(4, "readfile: nmax=%d\n", nmax);

    if (!file || !file->fp_valid) {
        return 0;
    } 

    if (file->fp_tag_valid) {

        if (file->repmode) {
            t = (uint32_t)(tick_master + file->offset);
        } else {
            t = (uint32_t)((tickget() - file->tick) * file->speed +
                            file->start * 1000.0);
            tick_master = t;
        }

#if CONFIG_DISABLE_STREAM_TAG_TIME
        if (file->tick_n == (uint32_t)(-1)) {
            tn = (uint32_t)(-1);
        } else if (file->tag_absolute) {
            tn = file->tick_n - file->tick_f;
        } else {
            tn = file->tick_n;
        }

        while ((int)(tn - t) <= 0)
#else
        while ((int)(file->tick_n - t) <= 0)
#endif
        {
            UINT nr_tick = 0, nr_fpos = 0;
            FRESULT fr_t = f_read(&file->fp_tag, &file->tick_n,
                                   sizeof(file->tick_n), &nr_tick);
            FRESULT fr_f = f_read(&file->fp_tag,
                                   (file->size_fpos == 4) ? (void *)&fpos_4B
                                                          : (void *)&fpos_8B,
                                   (UINT)file->size_fpos, &nr_fpos);

            if (fr_t != FR_OK || nr_tick < sizeof(file->tick_n) ||
                fr_f != FR_OK || nr_fpos < (UINT)file->size_fpos)
            {
                file->tick_n = (uint32_t)(-1);
                pos = fatfs_tell(&file->fp);
                file->fpos_n = fatfs_size(&file->fp);
                fatfs_seek(&file->fp, pos);
                break;
            }

#if !CONFIG_DISABLE_STREAM_TAG_TIME
            if ((unsigned)file->tick_n > file->tick_f) {
                file->tick_n = 0;
            }
#endif
            file->fpos_n = (long)((file->size_fpos == 4) ? fpos_4B
                                                          : (long)fpos_8B);

#if CONFIG_DISABLE_STREAM_TAG_TIME
            /* read new line, recalculate tn */
            tn = file->tag_absolute ?
                 (file->tick_n - file->tick_f) : file->tick_n;
#endif
        }

        if (file->tick_n == (uint32_t)(-1)) {
            sprintf(msg, "end");
        } else {
            sprintf(msg, "T%+.1fs", (int)t * 0.001);
            file->wtime = timeadd(file->time, (int)t * 0.001);
#if CONFIG_DISABLE_STREAM_TAG_TIME
            timeset(timeadd(gpst2utc(file->time),
                            (int)file->tick_n * 0.001));
#else
            /* stream.c 非 disable 分支使用 tn；这里等价置为 tick_n */
            tn = file->tick_n;
            timeset(timeadd(gpst2utc(file->time),
                            (int)tn * 0.001));
#endif
        }

        if ((n = file->fpos_n - fatfs_tell(&file->fp)) < nmax) {
            nmax = (int)n;
        }
    }

    if (nmax > 0) {
        if (f_read(&file->fp, buff, (UINT)nmax, &nr_uint) == FR_OK) {
            nr = (int)nr_uint;
        }
    }

    if (f_eof(&file->fp)) {
        sprintf(msg, "end");
    }

    tracet(5, "readfile: nr=%d\n", nr);
    return nr;
}

static int writefile(file_t *file, uint8_t *buff, int n, char *msg)
{
    gtime_t  wtime;
    uint64_t fpos_8B;
    uint32_t tick = tickget(), fpos_4B, tick_rel;
    int      week1, week2;
    double   tow1, tow2, intv;
    long     fpos;
    UINT     nw_uint = 0;
    int      ns = 0;

    tracet(4, "writefile: n=%d\n", n);

    if (!file || !file->fp_valid) {
        return 0;
    }

    wtime = utc2gpst(timeget());

    if (file->swapintv > 0.0 && file->wtime.time != 0) {
        intv = file->swapintv * 3600.0;
        tow1 = time2gpst(file->wtime, &week1);
        tow2 = time2gpst(wtime,       &week2);
        tow2 += 604800.0 * (week2 - week1);

        if (floor((tow1 + fswapmargin) / intv) <
            floor((tow2 + fswapmargin) / intv)) {
            swapfile(file, timeadd(wtime, fswapmargin), msg);
        }
        if (floor((tow1 - fswapmargin) / intv) <
            floor((tow2 - fswapmargin) / intv)) {
            swapclose(file);
        }
    }

    if (!file->fp_valid) {
        return 0;
    }

    if (f_write(&file->fp, buff, (UINT)n, &nw_uint) == FR_OK) {
        ns = (int)nw_uint;
    }

    fpos        = fatfs_tell(&file->fp);
    file->wtime = wtime;

    if (file->fp_tag_valid) {
        UINT nw_t = 0, nw_f = 0;
        tick_rel = tick - file->tick;

        f_write(&file->fp_tag, &tick_rel, sizeof(tick_rel), &nw_t);
        if (file->size_fpos == 4) {
            fpos_4B = (uint32_t)fpos;
            f_write(&file->fp_tag, &fpos_4B, sizeof(fpos_4B), &nw_f);
        } else {
            fpos_8B = (uint64_t)fpos;
            f_write(&file->fp_tag, &fpos_8B, sizeof(fpos_8B), &nw_f);
        }
    }

#if STREAM_FATFS_SYNC_ON_WRITE
    f_sync(&file->fp);
    if (file->fp_tag_valid) {
        f_sync(&file->fp_tag);
    }
#endif

    tracet(5, "writefile: ns=%d tick=%5d fpos=%ld\n", ns, (int)tick_rel, fpos);
    return ns;
}

static void syncfile(file_t *file1, file_t *file2)
{
    if (!file1->fp_tag_valid || !file2->fp_tag_valid) {
        return;
    }

    file1->repmode = 0;
    file2->repmode = 1;
    file2->offset  = (int)(file1->tick_f - file2->tick_f);
}

/* ------------------------------------------------------------------------ */
/* Interface - membuf                                                       */
/* ------------------------------------------------------------------------ */

static membuf_t *openmembuf(const char *path, char *msg)
{
    membuf_t *membuf;
    int       bsize = DEFAULT_MEMBUF_SIZE;

    tracet(3, "openmembuf: path=%s\n", path);
    msg[0] = '\0';
    sscanf(path, "%d", &bsize);

    if (!(membuf = (membuf_t *)rtkport_malloc(sizeof(membuf_t)))) {
        return NULL;
    }

    membuf->state = 1;
    membuf->rp    = 0;
    membuf->wp    = 0;

    if (!(membuf->buf = (uint8_t *)rtkport_malloc(bsize))) {
        rtkport_free(membuf);
        return NULL;
    }

    membuf->bufsize = bsize;
    initlock(&membuf->lock);
    sprintf(msg, "membuf sizebuf=%d", bsize);
    return membuf;
}

static void closemembuf(membuf_t *membuf)
{
    tracet(3, "closemembuf\n");

    if (!membuf) {
        return;
    }
    rtkport_free(membuf->buf);
    rtkport_free(membuf);
}

static int readmembuf(membuf_t *membuf, uint8_t *buff, int n, char *msg)
{
    int i, nr = 0;
    (void)msg;

    if (!membuf) {
        return 0;
    }

    lock(&membuf->lock);

    for (i = membuf->rp; i != membuf->wp && nr < n; i++) {
        if (i >= membuf->bufsize) i = 0;
        buff[nr++] = membuf->buf[i];
    }

    membuf->rp = i;
    unlock(&membuf->lock);
    return nr;
}

static int writemembuf(membuf_t *membuf, uint8_t *buff, int n, char *msg)
{
    int i;
    if (!membuf) return 0;

    lock(&membuf->lock);

    for (i = 0; i < n; i++) {
        membuf->buf[membuf->wp++] = buff[i];
        if (membuf->wp >= membuf->bufsize) membuf->wp = 0;
        if (membuf->wp == membuf->rp) {
            strcpy(msg, "mem-buffer overflow");
            membuf->state = -1;
            unlock(&membuf->lock);
            return i + 1;
        }
    }

    unlock(&membuf->lock);
    return i;
}

static int statemembuf(membuf_t *membuf) 
{ 
    return !membuf ? 0 : membuf->state; 
}

static int statexmembuf(membuf_t *membuf, char *msg)
{
    char *p = msg;
    int   state = !membuf ? 0 : membuf->state;

    p += sprintf(p, "membuf:\n");
    p += sprintf(p, "  state   = %d\n", state);
    if (!state) {
        return 0;
    }

    p += sprintf(p, "  buffsize= %d\n", membuf->bufsize);
    p += sprintf(p, "  wp      = %d\n", membuf->wp);
    p += sprintf(p, "  rp      = %d\n", membuf->rp);

    return state;
}

/* ------------------------------------------------------------------------ */
/* Interface - serial                                                       */
/* ------------------------------------------------------------------------ */

static serial_t *openserial(const char *path)
{
    serial_t *com;

    if (!(com = (serial_t *)rtkport_malloc(sizeof(serial_t)))) {
        return 0;
    }

    com->dev = up_bind(path);
    if (com->dev == NULL) {
        return 0;
    }

    return com;
}

static void closeserial(serial_t *com)
{
    if (com) {
        rtkport_free(com);
    }
}

static int readserial(serial_t *com, uint8_t *buff, int n)
{
    int rsz = 0;

    if (!com->dev) {
        return 0;
    }

    return SERIAL_RDBUF(com->dev, buff, n);
}

static int writeserial(serial_t *com, uint8_t *buff, int n)
{
    int rsz = 0;

    if (!com->dev) {
        return 0;
    }

    return SERIAL_SEND(com->dev, (uint8_t *)buff, n);
}

static int stateserial(serial_t *com)
{
    return !com ? 0 : (com->error ? -1 : 2);
}

extern void strinit(stream_t *stream)
{
    tracet(3, "strinit:\n");

    stream->type   = 0;
    stream->mode   = 0;
    stream->state  = 0;
    stream->inb = stream->inr = stream->outb = stream->outr = 0;
    stream->tick_i = stream->tick_o = stream->tact = 0;
    stream->inbt = stream->outbt = 0;
    initlock(&stream->lock);
    stream->port     = NULL;
    stream->path[0]  = '\0';
    stream->msg[0]   = '\0';
}

extern int stropen(stream_t *stream, int type, int mode, const char *path)
{
    tracet(3, "stropen: type=%d mode=%d path=%s\n", type, mode, path);

    stream->type   = type;
    stream->mode   = mode;
#if 0
    strcpy(stream->path,path);
#else
    strncpy(stream->path, path, sizeof(stream->path) - 1);
    stream->path[sizeof(stream->path) - 1] = '\0';
#endif
    stream->inb = stream->inr = stream->outb = stream->outr = 0;
    stream->tick_i = stream->tick_o = tickget();
    stream->inbt = stream->outbt = 0;
    stream->msg[0] = '\0';
    stream->port   = NULL;

    switch (type) {
        case STR_SERIAL: stream->port = openserial(path);                    break;
        case STR_FILE:   stream->port = openfile(path, mode, stream->msg);   break;
        case STR_MEMBUF: stream->port = openmembuf(path, stream->msg);       break;
        default: stream->state = 0; return 1;
    }

    stream->state = !stream->port ? -1 : 1;
    return stream->port != NULL;
}

extern void strclose(stream_t *stream)
{
    tracet(3, "strclose: type=%d mode=%d\n", stream->type, stream->mode);

    strlock(stream);
    if (stream->port) {
        switch (stream->type) {
            case STR_SERIAL: closeserial((serial_t *)stream->port); break;
            case STR_FILE:   closefile((file_t *)stream->port);     break;
            case STR_MEMBUF: closemembuf((membuf_t *)stream->port); break;
            default: break;
        }
    }

    stream->type    = 0;
    stream->mode    = 0;
    stream->state   = 0;
    stream->inr     = stream->outr = 0;
    stream->path[0] = '\0';
    stream->msg[0]  = '\0';
    stream->port    = NULL;
    strunlock(stream);
}

extern void strsync(stream_t *stream1, stream_t *stream2)
{
    file_t *f1, *f2;
    if (stream1->type != STR_FILE || stream2->type != STR_FILE) return;
    f1 = (file_t *)stream1->port;
    f2 = (file_t *)stream2->port;
    if (f1 && f2) {
        syncfile(f1, f2);
    }
}

extern void strlock  (stream_t *stream) { lock  (&stream->lock); }
extern void strunlock(stream_t *stream) { unlock(&stream->lock); }

extern int strread(stream_t *stream, uint8_t *buff, int n)
{
    uint32_t tick = tickget();
    char    *msg  = stream->msg;
    int      nr   = 0, tt;

    tracet(4, "strread: n=%d\n", n);

    if (!(stream->mode & STR_MODE_R) || !stream->port) return 0;

    strlock(stream);
    switch (stream->type) {
        case STR_SERIAL: nr = readserial((serial_t *)stream->port, buff, n);       break;
        case STR_FILE:   nr = readfile((file_t *)stream->port, buff, n, msg);      break;
        case STR_MEMBUF: nr = readmembuf((membuf_t *)stream->port, buff, n, msg);  break;
        default: strunlock(stream); return 0;
    }

    if (nr > 0) {
        stream->inb  += nr;
        stream->tact  = tick;
    }

    tt = (int)(tick - stream->tick_i);
    if (tt >= tirate) {
        stream->inr    = (uint32_t)((double)((stream->inb - stream->inbt) * 8) /
                                    (tt * 0.001));
        stream->tick_i = tick;
        stream->inbt   = stream->inb;
    }

    strunlock(stream);
    return nr;
}

extern int strwrite(stream_t *stream, uint8_t *buff, int n)
{
    uint32_t tick = tickget();
    char    *msg  = stream->msg;
    int      ns   = 0, tt;

    tracet(4, "strwrite: n=%d\n", n);

    if (!(stream->mode & STR_MODE_W) || !stream->port) return 0;

    strlock(stream);
    switch (stream->type) {
        case STR_SERIAL: ns = writeserial((serial_t *)stream->port, buff, n);      break;
        case STR_FILE:   ns = writefile((file_t *)stream->port, buff, n, msg);     break;
        case STR_MEMBUF: ns = writemembuf((membuf_t *)stream->port, buff, n, msg); break;
        default: strunlock(stream); return 0;
    }

    if (ns > 0) {
        stream->outb += ns;
        stream->tact  = tick;
    }

    tt = (int)(tick - stream->tick_o);
    if (tt >= tirate) {
        stream->outr    = (uint32_t)((double)((stream->outb - stream->outbt) * 8) /
                                     (tt * 0.001));
        stream->tick_o  = tick;
        stream->outbt   = stream->outb;
    }

    strunlock(stream);
    return ns;
}

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
        case STR_SERIAL: state = stateserial((serial_t *)stream->port); break;
        case STR_FILE:   state = statefile((file_t *)stream->port);     break;
        case STR_MEMBUF: state = statemembuf((membuf_t *)stream->port); break;
        default: strunlock(stream); return 0;
    }

    if (state == 2 && (int)(tickget() - stream->tact) <= TINTACT) {
        state = 3;
    }

    strunlock(stream);
    return state;
}

extern int strstatx(stream_t *stream, char *msg)
{
    int state = 0;

    tracet(4, "strstatx:\n");

    strlock(stream);

    if (!stream->port) {
        strunlock(stream);
        return stream->state;
    }

    switch (stream->type) {
        case STR_FILE:   state = statexfile((file_t *)stream->port, msg);     break;
        case STR_MEMBUF: state = statexmembuf((membuf_t *)stream->port, msg); break;
        default: msg[0] = '\0'; strunlock(stream); return 0;
    }

    if (state == 2 && (int)(tickget() - stream->tact) <= TINTACT) {
        state = 3;
    }

    strunlock(stream);
    return state;
}

extern void strsum(stream_t *stream, int *inb, int *inr, int *outb, int *outr)
{
    tracet(4, "strsum:\n");

    strlock(stream);
    if (inb)  *inb  = stream->inb;
    if (inr)  *inr  = stream->inr;
    if (outb) *outb = stream->outb;
    if (outr) *outr = stream->outr;
    strunlock(stream);
}

extern void strsetopt(const int *opt)
{
    tracet(3, "strsetopt: opt=%d %d %d %d %d %d %d %d\n", opt[0], opt[1], opt[2],
            opt[3], opt[4], opt[5], opt[6], opt[7]);
#if 0
    tirate     = opt[2] < 100 ? 100 : opt[2]; /* >=0.1s */
#endif
    buffsize    = opt[3] < 4096 ? 4096 : opt[3];
    fswapmargin = opt[4] < 0 ? 0 : opt[4];
}

extern gtime_t strgettime(stream_t *stream)
{
#if 1
    file_t *file;

    if (stream->type == STR_FILE && (stream->mode & STR_MODE_R) &&
        (file = (file_t *)stream->port))
    {
        return timeadd(file->time, file->start);
    }
#endif

#if 0
    gtime_t t = {0};
    return t;
#endif

    return utc2gpst(timeget());
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
#if 0
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
#endif
}
