#ifndef DEV_OPS_NODE_H_
#define DEV_OPS_NODE_H_

#include <stdint.h>
#include <stdbool.h>

#define CONFIG_DRIVER_DEVICE_NODE_NUM    (20)

struct dnode {
    void *dev;
    char  name[16];
};

#ifdef cplusplus
extern "C" {
#endif

bool dregister(char *name, void *dev);

void *dbind(char *name);

#ifdef cplusplus
}
#endif

#endif
