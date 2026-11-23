#include "WorkItemSingleShot.hpp"

namespace px4
{

WorkItemSingleShot::WorkItemSingleShot(const wq_config_t &config, worker_method method, void *argument)
	: WorkItem("<single_shot>", config), _argument(argument), _method(method)
{
	sem_init(&_sem, 0, 0);
}

WorkItemSingleShot::WorkItemSingleShot(const WorkItem &work_item, worker_method method, void *argument)
	: WorkItem("<single_shot>", work_item), _argument(argument), _method(method)
{
	sem_init(&_sem, 0, 0);
}

WorkItemSingleShot::~WorkItemSingleShot()
{
	sem_destroy(&_sem);
}

void WorkItemSingleShot::wait()
{
	do {} while (sem_wait(&_sem) != 0);
}

void WorkItemSingleShot::Run()
{
	_method(_argument);

	sem_post(&_sem);
}

} // namespace px4
