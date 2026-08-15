#ifndef MODULE_LOG_H_
#define MODULE_LOG_H_

#include <gpmx/config.h>
#include <driver/drv_sched.h>

#define mlog_out(...)    do { if (mlog_printf) mlog_printf(__VA_ARGS__); }while(0);

#ifndef KMRAW
#define KMRAW(...)    mlog_out(__VA_ARGS__)
#endif

#ifndef KMINFO
#define KMINFO(...)    mlog_out("[INFO] " __VA_ARGS__)
#endif

#ifndef KMERROR
#define KMERROR(...)    mlog_out("[ERROR] " __VA_ARGS__)
#endif

#ifndef KMWARN
#define KMWARN(...)    mlog_out("[WARN] " __VA_ARGS__)
#endif

#ifndef KMDEBUG
#define KMDEBUG(...)    mlog_out("[DEBUG] " __VA_ARGS__)
#endif

#if defined(__cplusplus)
extern "C"{
#endif

extern int (*mlog_printf)(const char *format, ...);
void mlog_redirect(int (*interface_log)(const char *format, ...));

#if defined(__cplusplus)
}
#endif

#endif
