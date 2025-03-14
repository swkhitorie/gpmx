#include <dev/dnode.h>
#include <string.h>

static struct dnode d_list[CONFIG_DRIVER_DEVICE_NODE_NUM];

bool dregister(char *name, void *dev)
{
    int i = 0;
    int j = 0;
    int sz = strlen(name);
    int msz = sz < 16 ? sz : 16;

    for (; i < CONFIG_DRIVER_DEVICE_NODE_NUM; i++) {
        if (d_list[i].dev == NULL) {
            d_list[i].dev = dev;
            for (j = 0; j < msz; j++) {
                d_list[i].name[j] = name[j];
            }
            if (j < 16) {
                d_list[i].name[j] = '\0';
            }
            break;
        }
    }

    return (i != CONFIG_DRIVER_DEVICE_NODE_NUM);
}

void *dbind(char *name)
{
    int i = 0;
    for (; i < CONFIG_DRIVER_DEVICE_NODE_NUM; i++) {
        if (d_list[i].dev != NULL) {
            if (!strcmp(d_list[i].name, name)) {
                return d_list[i].dev;
            }
        }
    }
    return NULL;
}

