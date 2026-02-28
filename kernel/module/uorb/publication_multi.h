#ifndef PUBLICATION_MULTI_H_
#define PUBLICATION_MULTI_H_

#include "uorb_common.h"
#include "publication.h"

struct __publication_multi {
    struct __publication_base _pubase;

    uint8_t _queue_size;
    enum ORB_PRIO _priority;
};

#ifdef __cplusplus
extern "C" {
#endif

void publication_multi_init(struct __publication_multi *pub_multi, uint8_t queue_sz, enum ORB_ID id, enum ORB_PRIO priority);
void publication_multi_init2(struct __publication_multi *pub_multi, uint8_t queue_sz, const struct orb_metadata *meta, enum ORB_PRIO priority);
void publication_multi_deinit(struct __publication_multi *pub_multi);

bool publication_multi_advertise(struct __publication_multi *pub_multi);
bool publication_multi_publish(struct __publication_multi *pub_multi, const void *data);

#ifdef __cplusplus
}
#endif

#endif

