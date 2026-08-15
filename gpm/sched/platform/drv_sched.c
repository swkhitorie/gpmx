#include <gpmx/config.h>

#include <driver/drv_sched.h>
#include <string.h>

void *kmm_realloc(void *oldmem, size_t newsize)
{
    void *tmp = kmm_malloc(newsize); 
    if (!tmp) {
        return NULL;
    }
    memcpy(tmp, oldmem, newsize);
    kmm_free(oldmem); 

    return tmp; 
}

void *kmm_zalloc(size_t size)
{
    void *tmp = kmm_malloc(size); 
    if (tmp) {
        memset(tmp,0,size); 
    }

    return tmp;
}

void *up_memcpy(void *dst, const void *src, size_t n)
{
    char *tmp = (char *)dst;
    const char *s = (char *)src;
    uint32_t len;

    if (tmp <= s || tmp > (s + n)) {
        while (n--) {
            *tmp ++ = *s ++;
        }
    } else {
        for (len = n; len > 0; len --) {
            tmp[len - 1] = s[len - 1];
        }
    }

    return dst;
}

#ifndef CONFIG_DRIVER_DEVICE_NODE_NUM
#define CONFIG_DRIVER_DEVICE_NODE_NUM    (20)
#endif

struct device_node {
    void *ops;
    char  path[16];
} _gdev_list[CONFIG_DRIVER_DEVICE_NODE_NUM];

bool up_register(const char *path, void *dev)
{
    int i = 0;
    int j = 0;
    int sz = strlen(path);
    int msz = sz < 16 ? sz : 16;

    for (; i < CONFIG_DRIVER_DEVICE_NODE_NUM; i++) {
        if (_gdev_list[i].ops == NULL) {
            _gdev_list[i].ops = dev;
            for (j = 0; j < msz; j++) {
                _gdev_list[i].path[j] = path[j];
            }
            if (j < 16) {
                _gdev_list[i].path[j] = '\0';
            }
            break;
        }
    }

    return (i != CONFIG_DRIVER_DEVICE_NODE_NUM);
}

void *up_bind(const char *path)
{
    int i = 0;
    for (; i < CONFIG_DRIVER_DEVICE_NODE_NUM; i++) {
        if (_gdev_list[i].ops != NULL) {
            if (!strcmp(_gdev_list[i].path, path)) {
                return _gdev_list[i].ops;
            }
        }
    }
    return NULL;
}

