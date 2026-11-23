#pragma once

#include <gpmx/config.h>

#include <stdint.h>
#include <string.h>

#include "workqueue_manager.h"
#include "workqueue.h"
#include "workitem.h"

#include <drivers/drv_hrt.h>
#include <mlog/mlog.h>

namespace px4
{

typedef ::wq_config_t wq_config_t;

namespace wq_configurations
{
extern const wq_config_t &rate_ctrl;

extern const wq_config_t &spi0;
extern const wq_config_t &spi1;
extern const wq_config_t &spi2;
extern const wq_config_t &spi3;
extern const wq_config_t &spi4;
extern const wq_config_t &spi5;
extern const wq_config_t &spi6;

extern const wq_config_t &i2c0;
extern const wq_config_t &i2c1;
extern const wq_config_t &i2c2;
extern const wq_config_t &i2c3;
extern const wq_config_t &i2c4;

extern const wq_config_t &nav_and_controllers;

extern const wq_config_t &ins0;
extern const wq_config_t &ins1;
extern const wq_config_t &ins2;
extern const wq_config_t &ins3;

extern const wq_config_t &hp_default;
extern const wq_config_t &uavcan;

extern const wq_config_t &uart0;
extern const wq_config_t &uart1;
extern const wq_config_t &uart2;
extern const wq_config_t &uart3;
extern const wq_config_t &uart4;
extern const wq_config_t &uart5;
extern const wq_config_t &uart6;
extern const wq_config_t &uart7;
extern const wq_config_t &uart8;
extern const wq_config_t &ttyacm0;
extern const wq_config_t &uart_unknown;

extern const wq_config_t &lp_default;

extern const wq_config_t &test1;
extern const wq_config_t &test2;
} // namespace wq_configurations

const px4::wq_config_t &ins_instance_to_wq(uint8_t instance);
const px4::wq_config_t &serial_port_to_wq(const char *serial);
const px4::wq_config_t &device_bus_to_wq(const char *bus);

class WorkItem
{
public:

	WorkItem() = delete;

	// no copy, assignment, move, move assignment
	WorkItem(const WorkItem &) = delete;
	WorkItem &operator=(const WorkItem &) = delete;
	WorkItem(WorkItem &&) = delete;
	WorkItem &operator=(WorkItem &&) = delete;

	// WorkItems sorted by name
	bool operator<=(const WorkItem &rhs) const { return (strcmp(ItemName(), rhs.ItemName()) <= 0); }

	inline void ScheduleNow()
	{
		workitem_schedule_now(&_item);
	}

	virtual void print_run_status();

	/**
	 * Switch to a different WorkQueue.
	 * NOTE: Caller is responsible for synchronization.
	 */
	bool ChangeWorkQueue(const wq_config_t &config) { return Init(config); }

	const char *ItemName() const { return _item._name; }

protected:

	explicit WorkItem(const char *name, const wq_config_t &config);
	explicit WorkItem(const char *name, const WorkItem &work_item);

	virtual ~WorkItem();

	/**
	 * Remove work item from the runnable queue, if it's there
	 */
	void ScheduleClear();

	virtual void Run() = 0;

	/**
	 * Initialize WorkItem given a WorkQueue config.
	 * NOTE: Caller is responsible for synchronization.
	 */
	bool Init(const wq_config_t &config);
	void Deinit();

	float elapsed_time() const;
	float average_rate() const;
	float average_interval() const;

private:

	static void RunTrampoline(void *arg);

	struct __workitem *raw() const { return const_cast<struct __workitem *>(&_item); }

	// if malloc this class, should clear to zero
	struct __workitem _item {};
};

} // namespace px4
