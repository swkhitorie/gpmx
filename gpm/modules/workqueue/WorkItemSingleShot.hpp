#pragma once

#include "WorkItem.hpp"

#include <semaphore.h>

namespace px4
{

/**
 * @class WorkItemSingleShot
 * Run a method on a specific work queue and wait for it to exit.
 */
class WorkItemSingleShot : public WorkItem
{
public:
	using worker_method = void (*)(void *arg);

	WorkItemSingleShot(const wq_config_t &config, worker_method method, void *argument);
	WorkItemSingleShot(const WorkItem &work_item, worker_method method, void *argument);

	~WorkItemSingleShot() override;

	WorkItemSingleShot() = delete;

	// no copy, assignment, move, move assignment
	WorkItemSingleShot(const WorkItemSingleShot &) = delete;
	WorkItemSingleShot &operator=(const WorkItemSingleShot &) = delete;
	WorkItemSingleShot(WorkItemSingleShot &&) = delete;
	WorkItemSingleShot &operator=(WorkItemSingleShot &&) = delete;

	void wait();

protected:
	void Run() override;

private:
	void *_argument;
	worker_method _method;
	sem_t _sem;
};

} // namespace px4
