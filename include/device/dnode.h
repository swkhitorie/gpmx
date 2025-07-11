#ifndef DEV_OPS_NODE_H_
#define DEV_OPS_NODE_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CONFIG_DRIVER_DEVICE_NODE_NUM    (20)

#define DEV_OK     (0)
#define DEV_NOINIT (1)
#define DEV_ERROR  (2)

#ifndef DEVICE_DEBUG(...)
#include <stdio.h>
#define DEVICE_DEBUG(...)     do{ printf(__VA_ARGS__);}while(0);
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
