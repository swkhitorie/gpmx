
#include <px4_platform_common/log.h>
#include <px4_platform_common/defines.h>

#include <FreeRTOS.h>
#include <tasks.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <px4_platform_common/tasks.h>
#include <px4_platform_common/posix.h>
#include <systemlib/err.h>

//#define PX4_TASK_DEBUG

#define PX4_MAX_TASKS 50
#define SHELL_TASK_ID (PX4_MAX_TASKS+1)

pthread_t _shell_task_id = 0;
pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;

struct task_entry {
	pthread_t pid;
	char name[16];
	bool isused;
	task_entry() : isused(false) {}
};

static task_entry taskmap[PX4_MAX_TASKS] = {};

typedef struct {
	px4_main_t entry;
	char name[16]; //pthread_setname_np is restricted to 16 chars
	int argc;
	char *argv[];
	// strings are allocated after the struct data
} pthdata_t;

static void *entry_adapter(void *ptr)
{
	pthdata_t *data = (pthdata_t *) ptr;

	int rv;
	rv = pthread_setname_np(pthread_self(), data->name);

	if (rv) {
		PX4_ERR("px4_task_spawn_cmd: failed to set name of thread %d %d\n", rv, errno);
	}

	data->entry(data->argc, data->argv);

	vPortFree(ptr);
	#ifdef PX4_TASK_DEBUG
	PX4_DEBUG("Before px4_task_exit");
	#endif
	px4_task_exit(0);
	#ifdef PX4_TASK_DEBUG
	PX4_DEBUG("After px4_task_exit");
	#endif

	return nullptr;
}

int px4_task_spawn_cmd(const char *name, int scheduler, int priority, int stack_size, main_t entry, char *const argv[])
{
	int i;
	int argc = 0;
	unsigned int len = 0;
	struct sched_param param = {};
	char *p = (char *)argv;

	// Calculate argc
	while (p != NULL) {
		p = argv[argc];

		if (p == NULL) {
			break;
		}

		++argc;
		len += strlen(p) + 1;
	}

	unsigned long structsize = sizeof(pthdata_t) + (argc + 1) * sizeof(char *);

	// not safe to pass stack data to the thread creation
	pthdata_t *taskdata = (pthdata_t *)pvPortMalloc(structsize + len);

	if (taskdata == NULL) {
		return -ENOMEM;
	}

	memset(taskdata, 0, structsize + len);
	unsigned long offset = ((unsigned long)taskdata) + structsize;

	strncpy(taskdata->name, name, 16);
	taskdata->name[15] = 0;
	taskdata->entry = entry;
	taskdata->argc = argc;

	for (i = 0; i < argc; i++) {
		#ifdef PX4_TASK_DEBUG
		PX4_DEBUG("arg %d %s\n", i, argv[i]);
		#endif
		taskdata->argv[i] = (char *)offset;
		strcpy((char *)offset, argv[i]);
		offset += strlen(argv[i]) + 1;
	}

	// Must add NULL at end of argv
	taskdata->argv[argc] = (char *)NULL;
    #ifdef PX4_TASK_DEBUG
	PX4_DEBUG("starting task %s", name);
    #endif
	pthread_attr_t attr;
	int rv = pthread_attr_init(&attr);

	if (rv != 0) {
		#ifdef PX4_TASK_DEBUG
		PX4_ERR("px4_task_spawn_cmd: failed to init thread attrs");
		#endif
		vPortFree(taskdata);
		return (rv < 0) ? rv : -rv;
	}

	if (stack_size < PTHREAD_STACK_MIN) {
		stack_size = PTHREAD_STACK_MIN;
	}

	rv = pthread_attr_setstacksize(&attr, PX4_STACK_ADJUSTED(stack_size));

	if (rv != 0) {
		#ifdef PX4_TASK_DEBUG
		PX4_ERR("pthread_attr_setstacksize to %d returned error (%d)", stack_size, rv);
		#endif
		pthread_attr_destroy(&attr);
		vPortFree(taskdata);
		return (rv < 0) ? rv : -rv;
	}

	rv = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	if (rv != 0) {
		#ifdef PX4_TASK_DEBUG
		PX4_ERR("px4_task_spawn_cmd: failed to set inherit sched");
		#endif
		pthread_attr_destroy(&attr);
		vPortFree(taskdata);
		return (rv < 0) ? rv : -rv;
	}

	rv = pthread_attr_setschedpolicy(&attr, scheduler);

	if (rv != 0) {
		#ifdef PX4_TASK_DEBUG
		PX4_ERR("px4_task_spawn_cmd: failed to set sched policy");
		#endif
		pthread_attr_destroy(&attr);
		vPortFree(taskdata);
		return (rv < 0) ? rv : -rv;
	}

	param.sched_priority = priority;

	rv = pthread_attr_setschedparam(&attr, &param);

	if (rv != 0) {
		#ifdef PX4_TASK_DEBUG
		PX4_ERR("px4_task_spawn_cmd: failed to set sched param");
		#endif
		pthread_attr_destroy(&attr);
		vPortFree(taskdata);
		return (rv < 0) ? rv : -rv;
	}

	pthread_mutex_lock(&task_mutex);

	px4_task_t taskid = 0;

	for (i = 0; i < PX4_MAX_TASKS; ++i) {
		if (!taskmap[i].isused) {
			strncpy(taskmap[i].name, name, 16);
            taskmap[i].name[15] = 0;
			taskmap[i].isused = true;
			taskid = i;
			break;
		}
	}

	if (i >= PX4_MAX_TASKS) {
		pthread_attr_destroy(&attr);
		pthread_mutex_unlock(&task_mutex);
		vPortFree(taskdata);
		return -ENOSPC;
	}

	rv = pthread_create(&taskmap[taskid].pid, &attr, &entry_adapter, (void *) taskdata);

	if (rv != 0) {

		if (rv == EPERM) {
			//printf("WARNING: NOT RUNING AS ROOT, UNABLE TO RUN REALTIME THREADS\n");
			rv = pthread_create(&taskmap[taskid].pid, NULL, &entry_adapter, (void *) taskdata);

			if (rv != 0) {
				#ifdef PX4_TASK_DEBUG
				PX4_ERR("px4_task_spawn_cmd: failed to create thread %d %d\n", rv, errno);
				#endif
				taskmap[taskid].isused = false;
				pthread_attr_destroy(&attr);
				pthread_mutex_unlock(&task_mutex);
				vPortFree(taskdata);
				return (rv < 0) ? rv : -rv;
			}

		} else {
			pthread_attr_destroy(&attr);
			pthread_mutex_unlock(&task_mutex);
			vPortFree(taskdata);
			return (rv < 0) ? rv : -rv;
		}
	}

	pthread_attr_destroy(&attr);
	pthread_mutex_unlock(&task_mutex);

	return taskid;
}

/**
 * only support delete itself
 */
int px4_task_delete(px4_task_t id)
{
	int rv = 0;
	pthread_t pid;
	#ifdef PX4_TASK_DEBUG
	PX4_DEBUG("Called px4_task_delete");
    #endif

	if (id < PX4_MAX_TASKS && taskmap[id].isused) {
		pid = taskmap[id].pid;

	} else {
		return -EINVAL;
	}

	pthread_mutex_lock(&task_mutex);

	// If current thread then exit, otherwise cancel
	if (pthread_self() == pid) {
		pthread_join(pid, nullptr);
		taskmap[id].isused = false;
		pthread_mutex_unlock(&task_mutex);
		pthread_exit(nullptr);

	} else {
		rv = pthread_cancel(pid);
	}

	taskmap[id].isused = false;
	pthread_mutex_unlock(&task_mutex);
	return rv;
}

void px4_task_exit(int ret)
{
	int i;
	pthread_t pid = pthread_self();

	// Get pthread ID from the opaque ID
	for (i = 0; i < PX4_MAX_TASKS; ++i) {
		if (taskmap[i].pid == pid) {
			pthread_mutex_lock(&task_mutex);
			taskmap[i].isused = false;
			break;
		}
	}

	if (i >= PX4_MAX_TASKS)  {
		PX4_ERR("px4_task_exit: self task not found!");

	} else {
		#ifdef PX4_TASK_DEBUG
		PX4_DEBUG("px4_task_exit: %s", taskmap[i].name);
		#endif
	}

	pthread_mutex_unlock(&task_mutex);
	pthread_exit((void *)(unsigned long)ret);
}

void px4_show_tasks()
{
	int idx;
	int count = 0;

	PX4_INFO("Active Tasks:");

	for (idx = 0; idx < PX4_MAX_TASKS; idx++) {
		if (taskmap[idx].isused) {
			PX4_INFO("   %-10s %lu", taskmap[idx].name, (unsigned long)taskmap[idx].pid);
			count++;
		}
	}

	if (count == 0) {
		PX4_INFO("   No running tasks");
	}

}

bool px4_task_is_running(const char *taskname)
{
	int idx;

	for (idx = 0; idx < PX4_MAX_TASKS; idx++) {
		if (taskmap[idx].isused && (strcmp(taskmap[idx].name, taskname) == 0)) {
			return true;
		}
	}

	return false;
}

const char *px4_get_taskname()
{
	pthread_t pid = pthread_self();
	const char *prog_name = "UnknownApp";

	pthread_mutex_lock(&task_mutex);

	for (int i = 0; i < PX4_MAX_TASKS; i++) {
		if (taskmap[i].isused && taskmap[i].pid == pid) {
			prog_name = taskmap[i].name;
		}
	}

	pthread_mutex_unlock(&task_mutex);

	return prog_name;
}

int px4_prctl(int option, const char *arg2, px4_task_t pid)
{
	int rv;

	switch (option) {
	case PR_SET_NAME:
		rv = pthread_setname_np(pthread_self(), arg2);
		break;
	default:
		rv = -1;
		PX4_WARN("FAILED SETTING TASK NAME");
		break;
	}

	return rv;
}

