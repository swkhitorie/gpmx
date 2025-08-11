#ifndef DEV_OPS_NODE_H_
#define DEV_OPS_NODE_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define DEV_OK     (0)
#define DEV_NOINIT (1)
#define DEV_ERROR  (2)

#define DLOG_D(...)        // do{ printf(__VA_ARGS__); printf("\r\n"); }while(0);
#define DLOG_E(...)     

#define SMEMCPY(dst,src,len)            memcpy(dst,src,len)

#ifndef CONFIG_DRIVER_DEVICE_NODE_NUM
#define CONFIG_DRIVER_DEVICE_NODE_NUM    (20)
#endif

#ifndef _IOC(type,nr)
#define _IOC(type,nr)   ((type)|(nr))
#endif

struct dnode {
    void    *_devops;
    char  _devname[16];
};

#if defined(__cplusplus)
extern "C"{
#endif

bool dregister(const char *name, void *dev);

void *dbind(const char *name);

#if defined(__cplusplus)
}
#endif

#endif
