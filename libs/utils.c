#include <stddef.h>
#include <limits.h>
#include "utils.h"
#include "errno.h"

size_t utils_strlen(const char *p, size_t maxlen)
{
    const char *pt = p;
    size_t len = 0;
    if (p != NULL) {
        while (*pt != '\0' && len < maxlen) {
            len++;
            pt++;
        }
    }
    return len;
}

bool utils_validtimespec(const struct timespec *p)
{
    bool ret = false;
    if (p != NULL) {
        if (p->tv_nsec >= 0 && p->tv_nsec < NANOSECONDS_PER_SECOND) {
            ret = true;
        }
    }
    return ret;
}

int utils_timespec_compare(const struct timespec *x, const struct timespec *y)
{
    int ret = 0;
    if (x == NULL && y == NULL) {
        ret = 0;
    } else if (y == NULL) {
        ret = 1;
    } else if (x == NULL) {
        ret = -1;
    } else if (x->tv_sec > y->tv_sec) {
        ret = 1;
    } else if (x->tv_sec < y->tv_sec) {
        ret = -1;
    } else {
        if (x->tv_nsec > y->tv_nsec) {
            ret = 1;
        } else if (x->tv_nsec < y->tv_nsec) {
            ret = -1;
        } else {
            ret = 0;
        }
    }
    return ret;
}

int utils_timespec_add(const struct timespec *x, const struct timespec *y, struct timespec *z)
{
    int64_t psec = 0;
    int ret = 0;

    if (z == NULL || x == NULL || y == NULL) {
        ret = -1;
    }

    if (ret == 0) {
        z->tv_nsec = x->tv_nsec + y->tv_nsec;
        if (z->tv_nsec < 0) {
            ret = 1;
        } else {
            psec = z->tv_nsec / NANOSECONDS_PER_SECOND;
            z->tv_nsec = z->tv_nsec % NANOSECONDS_PER_SECOND;
            z->tv_sec = x->tv_sec + y->tv_sec + psec;
            if (z->tv_sec < 0) {
                ret = 1;
            }
        }
    }
    return ret;
}

int utils_timespec_addnanoseconds(const struct timespec *x, int64_t nanosec, struct timespec *y)
{
    int64_t total_nsec = 0;
    int ret = 0;

    if( ( y == NULL ) || ( x == NULL ) ) {
        ret = -1;
    }

    if (ret == 0) {
        total_nsec = x->tv_nsec + nanosec;
        if (total_nsec < 0) {
            ret = 1;
        } else {
            y->tv_nsec = total_nsec % NANOSECONDS_PER_SECOND;
            y->tv_sec = x->tv_sec + (total_nsec / NANOSECONDS_PER_SECOND);
            if (y->tv_sec < 0) {
                ret = 1;
            }
        }
    }
    return ret;
}

int utils_timespec_subtract(const struct timespec *x, const struct timespec *y, struct timespec *z)
{
    int compare_ret = 0;
    int ret = 0;

    if (z == NULL || x == NULL || y == NULL) {
        ret = -1;
    }

    if (ret == 0) {
        compare_ret = utils_timespec_compare(x, y);
        if (compare_ret == -1) {
            ret = 1;
        } else if(compare_ret == 0) {
            z->tv_sec = 0;
            z->tv_nsec = 0;
        } else {
            z->tv_sec = x->tv_sec - y->tv_sec;
            z->tv_nsec = x->tv_nsec - y->tv_nsec;
            if (z->tv_nsec < 0) {
                z->tv_sec--;
                z->tv_nsec += (long)NANOSECONDS_PER_SECOND;
            }
            if (z->tv_nsec < 0) {
                ret = -1;
            }
        }
    }
    return ret;
}

int utils_timespec_toticks(const struct timespec *x, TickType_t *res)
{
    int ret = 0;
    int64_t total_tick = 0;
    long nano_sec = 0;

    if ( x == NULL || res == NULL) {
        ret = EINVAL;
    } else if (ret == 0 && utils_validtimespec(x) == false)
    {
        ret = EINVAL;
    }

    if (ret == 0 ) {
        total_tick = (int64_t)configTICK_RATE_HZ * x->tv_sec;
        nano_sec = x->tv_nsec / (long)NANOSECONDS_PER_TICK + (long) (x->tv_nsec % (long)NANOSECONDS_PER_TICK != 0);
        total_tick += (int64_t) nano_sec;
        if (total_tick < 0) {
            ret = EINVAL;
        } else {
            uint32_t ulTickTypeSize = sizeof(TickType_t);
            if (ulTickTypeSize == sizeof(uint32_t)) {
                if (total_tick > UINT_MAX) {
                    ret = EINVAL;
                }
            }
        }
        *res = (TickType_t)total_tick;
    }
    return ret;
}

void utils_nanoseconds_totimespec(int64_t source, struct timespec *dst)
{
    long carry_sec = 0;

    dst->tv_sec = (time_t)(source / NANOSECONDS_PER_SECOND);
    dst->tv_nsec = (long)(source % NANOSECONDS_PER_SECOND);
    if (dst->tv_nsec < 0L) {
        carry_sec = (dst->tv_nsec / (long)NANOSECONDS_PER_SECOND) + 1L;
        dst->tv_sec -= (time_t)(carry_sec);
        dst->tv_nsec += carry_sec * (long)NANOSECONDS_PER_SECOND;
    }
}

int utils_timespec_todeltaticks(const struct timespec *ab, const struct timespec *cur, TickType_t *res)
{
    int ret = 0;
    struct timespec diff = { 0 };

    if (ab == NULL || cur == NULL || res == NULL ) {
        ret = EINVAL;
    }

    if (ret == 0) {
        ret = utils_timespec_subtract(ab, cur, &diff);
        if (ret == 1) {
            ret = ETIMEDOUT;
        } else if (ret == -1) {
            ret = EINVAL;
        }
    }

    if (ret == 0) {
        ret = utils_timespec_toticks(&diff, res);
    }
    return ret;
}

