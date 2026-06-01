#ifndef PUBLICATION_CPP_H_
#define PUBLICATION_CPP_H_

#include "uorb_common.h"
#include "uorb_device_node.h"
#include <uorb/topics/uorb_topics.h>

namespace uORB
{

class PublicationBase
{
public:

	bool advertised() const { return _handle != nullptr; }

	bool unadvertise() { return (uorb_device_node_unadvertise(_handle) == 0); }

	orb_id_t get_topic() const { return get_orb_meta(_orb_id); }

protected:

	PublicationBase(enum ORB_ID id) : _orb_id(id) {}

	~PublicationBase()
	{
		if (_handle != nullptr) {
			// don't automatically unadvertise queued publications (eg vehicle_command)
			if (uorb_device_node_get_queue_size(static_cast<struct __uorb_device_node *>(_handle)) == 1) {
				unadvertise();
			}
		}
	}

	orb_advert_t _handle{nullptr};
	const enum ORB_ID _orb_id;
};

/**
 * uORB publication wrapper class
 */
template<typename T, uint8_t ORB_QSIZE = 1>
class Publication : public PublicationBase
{
public:

	/**
	 * Constructor
	 *
	 * @param meta The uORB metadata (usually from the ORB_ID() macro) for the topic.
	 */
	Publication(enum ORB_ID id) : PublicationBase(id) {}
	Publication(const orb_metadata *meta) : PublicationBase(static_cast<enum ORB_ID>(meta->o_id)) {}

	bool advertise()
	{
		if (!advertised()) {
			_handle = orb_advertise_queue(get_topic(), nullptr, ORB_QSIZE);
		}

		return advertised();
	}

	/**
	 * Publish the struct
	 * @param data The uORB message struct we are updating.
	 */
	bool publish(const T &data)
	{
		if (!advertised()) {
			advertise();
		}

        return (uorb_device_node_publish(get_topic(), _handle, &data) == 0);
	}
};

/**
 * The publication class with data embedded.
 */
template<typename T>
class PublicationData : public Publication<T>
{
public:
	/**
	 * Constructor
	 *
	 * @param meta The uORB metadata (usually from the ORB_ID() macro) for the topic.
	 */
	PublicationData(enum ORB_ID id) : Publication<T>(id) {}
	PublicationData(const orb_metadata *meta) : Publication<T>(meta) {}

	T	&get() { return _data; }
	void	set(const T &data) { _data = data; }

	// Publishes the embedded struct.
	bool	update() { return Publication<T>::publish(_data); }
	bool	update(const T &data)
	{
		_data = data;
		return Publication<T>::publish(_data);
	}

private:
	T _data{};
};

template<class T>
using PublicationQueued = Publication<T, T::ORB_QUEUE_LENGTH>;

} // namespace uORB

#endif
