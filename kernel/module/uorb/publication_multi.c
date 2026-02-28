#include "publication_multi.h"

void publication_multi_init(struct __publication_multi *pub_multi, uint8_t queue_sz, enum ORB_ID id, enum ORB_PRIO priority)
{
    publication_base_init(&pub_multi->_pubase, id);
    pub_multi->_priority = priority;
    pub_multi->_queue_size = queue_sz;
}

void publication_multi_init2(struct __publication_multi *pub_multi, uint8_t queue_sz, const struct orb_metadata *meta, enum ORB_PRIO priority)
{
    publication_base_init(&pub_multi->_pubase, (enum ORB_ID)(meta->o_id));
    pub_multi->_priority = priority;
    pub_multi->_queue_size = queue_sz;
}

void publication_multi_deinit(struct __publication_multi *pub_multi)
{
    publication_base_deinit(&pub_multi->_pubase);
}

bool publication_multi_advertise(struct __publication_multi *pub_multi)
{
    if (!publication_base_advertised(&pub_multi->_pubase)) {
        int instance = 0;
        pub_multi->_pubase._handle = orb_advertise_multi_queue(publication_base_get_topic(&pub_multi->_pubase), NULL, &instance, pub_multi->_priority, pub_multi->_queue_size);
    }

    return publication_base_advertised(&pub_multi->_pubase);
}

bool publication_multi_publish(struct __publication_multi *pub_multi, const void *data)
{
    if (!publication_base_advertised(&pub_multi->_pubase)) {
        publication_multi_advertise(pub_multi);
    }

    return (orb_publish(publication_base_get_topic(&pub_multi->_pubase), pub_multi->_pubase._handle, &data) == 0);
}

