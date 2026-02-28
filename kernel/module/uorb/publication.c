#include "publication.h"

void publication_base_init(struct __publication_base *pubase, enum ORB_ID id)
{
    pubase->_orb_id = id;
}

void publication_base_deinit(struct __publication_base *pubase)
{
    if (pubase->_handle != NULL) {

        // don't automatically unadvertise queued publications (eg vehicle_command)
        if (uorb_device_node_get_queue_size((struct __uorb_device_node *)pubase->_handle) == 1) {
            publication_base_unadvertise(pubase);
        }
    }
}

bool publication_base_advertised(struct __publication_base *pubase)
{
    return pubase->_handle != NULL;
}

bool publication_base_unadvertise(struct __publication_base *pubase)
{
    return (uorb_device_node_unadvertise(pubase->_handle) == 0);
}

orb_id_t publication_base_get_topic(struct __publication_base *pubase)
{
    return get_orb_meta(pubase->_orb_id);
}

void publication_init(struct __publication *pub, uint8_t queue_sz, enum ORB_ID id)
{
    publication_base_init(&pub->_pubase, id);
    pub->_queue_size = queue_sz;
}

void publication_init2(struct __publication *pub, uint8_t queue_sz, const struct orb_metadata *meta)
{
    publication_base_init(&pub->_pubase, (enum ORB_ID)(meta->o_id));
    pub->_queue_size = queue_sz;
}

void publication_deinit(struct __publication *pub)
{
    publication_base_deinit(&pub->_pubase);
}

bool publication_advertised(struct __publication *pub)
{
    return publication_base_advertised(&pub->_pubase);
}

bool publication_unadvertise(struct __publication *pub)
{
    return publication_base_unadvertise(&pub->_pubase);
}

orb_id_t publication_get_topic(struct __publication *pub)
{
    return publication_base_get_topic(&pub->_pubase);
}

bool publication_advertise(struct __publication *pub)
{
    if (!publication_advertised(pub)) {
        pub->_pubase._handle = orb_advertise_queue(publication_get_topic(pub), NULL, pub->_queue_size);
    }

    return publication_advertised(pub);
}

bool publication_publish(struct __publication *pub, const void *data)
{
    if (!publication_advertised(pub)) {
        publication_advertise(pub);
    }

    return (uorb_device_node_publish(publication_get_topic(pub), pub->_pubase._handle, data) == 0);
}
