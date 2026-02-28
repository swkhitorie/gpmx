#include "subscription_interval.h"
#include "drv_hrt.h"
#include "math.h"

static uint64_t tmp_constrain(uint64_t x, uint64_t a, uint64_t b)
{
    if (x < a) {
        return a;
    } else if (x > b) {
        return b;
    } else {
        return x;
    }
}

void subscription_interval_init(struct __subscription_interval *subitv, enum ORB_ID id, uint32_t interval_us, uint8_t instance)
{
    subitv->_interval_us = interval_us;
    subscription_init(&subitv->_sub, id, instance);
}

void subscription_interval_init2(struct __subscription_interval *subitv, const struct orb_metadata *meta, uint32_t interval_us, uint8_t instance)
{
    subitv->_interval_us = interval_us;
    subscription_init2(&subitv->_sub, meta, instance);
}

bool subscription_interval_updated(struct __subscription_interval *subitv)
{
    if (subscription_advertised(&subitv->_sub) && (hrt_elapsed_time(&subitv->_last_update) >= subitv->_interval_us)) {
        return subscription_updated(&subitv->_sub);
    }

    return false;
}

bool subscription_interval_update(struct __subscription_interval *subitv, void *dst)
{
    if (subscription_updated(&subitv->_sub)) {
        return subscription_interval_copy(subitv, dst);
    }

    return false;
}

bool subscription_interval_copy(struct __subscription_interval *subitv, void *dst)
{
    if (subscription_copy(&subitv->_sub, dst)) {
        const hrt_abstime now = hrt_absolute_time();
        // shift last update time forward, but don't let it get further behind than the interval
        subitv->_last_update = tmp_constrain(subitv->_last_update + subitv->_interval_us, now - subitv->_interval_us, now);
        return true;
    }

    return false;
}

uint32_t subscription_interval_get_us(struct __subscription_interval *subitv)
{
    return subitv->_interval_us;
}

void subscription_interval_set_us(struct __subscription_interval *subitv, uint32_t interval)
{
    subitv->_interval_us = interval;
}

void subscription_interval_set_ms(struct __subscription_interval *subitv, uint32_t interval)
{
    subitv->_interval_us = interval * 1000;
}
