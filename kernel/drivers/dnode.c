#include <device/dnode.h>
#include <string.h>

#ifndef CONFIG_DRIVER_DEVICE_NODE_NUM
#define CONFIG_DRIVER_DEVICE_NODE_NUM    (20)
#endif

static uint16_t     _dev_len= CONFIG_DRIVER_DEVICE_NODE_NUM;
static struct dnode _dev_list[CONFIG_DRIVER_DEVICE_NODE_NUM];

void *gmemcpy(void *dst, const void *src, size_t n)
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

bool dn_register(const char *name, void *dev)
{
    int i = 0;
    int j = 0;
    int sz = strlen(name);
    int msz = sz < 16 ? sz : 16;

    for (; i < _dev_len; i++) {
        if (_dev_list[i]._devops == NULL) {
            _dev_list[i]._devops = dev;
            for (j = 0; j < msz; j++) {
                _dev_list[i]._devname[j] = name[j];
            }
            if (j < 16) {
                _dev_list[i]._devname[j] = '\0';
            }
            break;
        }
    }

    return (i != _dev_len);
}

void *dn_bind(const char *name)
{
    int i = 0;
    for (; i < _dev_len; i++) {
        if (_dev_list[i]._devops != NULL) {
            if (!strcmp(_dev_list[i]._devname, name)) {
                return _dev_list[i]._devops;
            }
        }
    }
    return NULL;
}

