#ifndef PUBLICATION_H_
#define PUBLICATION_H_

#include "uorb_common.h"
#include "uorb_device_node.h"
#include <uorb/topics/uorb_topics.h>

struct __publication_base {
    orb_advert_t _handle;
    enum ORB_ID _orb_id;
};

struct __publication {
    struct __publication_base _pubase;
    uint8_t _queue_size;
};

#ifdef __cplusplus
extern "C" {
#endif

void publication_base_init(struct __publication_base *pubase, enum ORB_ID id);
void publication_base_deinit(struct __publication_base *pubase);

bool publication_base_advertised(struct __publication_base *pubase);
bool publication_base_unadvertise(struct __publication_base *pubase);
orb_id_t publication_base_get_topic(struct __publication_base *pubase);

void publication_init(struct __publication *pub, uint8_t queue_sz, enum ORB_ID id);
void publication_init2(struct __publication *pub, uint8_t queue_sz, const struct orb_metadata *meta);
void publication_deinit(struct __publication *pub);
bool publication_advertised(struct __publication *pub);
bool publication_unadvertise(struct __publication *pub);
orb_id_t publication_get_topic(struct __publication *pub);
bool publication_advertise(struct __publication *pub);
bool publication_publish(struct __publication *pub, const void *data);

#ifdef __cplusplus
}
#endif

#endif

