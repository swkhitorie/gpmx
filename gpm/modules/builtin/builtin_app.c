
/****************************************************************************
 * Included Files
 ****************************************************************************/
#define _GNU_SOURCE

#include <gpmx/config.h>

#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>

#include <mlog.h>

#include "builtin_app.h"

/************************************
 * BUILD APP Stask
 ************************************/
#ifndef CONFIG_BUILDAPP_MAX_TASKS
#define CONFIG_BUILDAPP_MAX_TASKS 20
#endif

#if defined(CONFIG_POSIXRUN_ENABLE)
#define	 BUILT_STACK_OVERHEAD	(1024 * 24)
#else
#define	 BUILT_STACK_OVERHEAD	(0)
#endif
#define BUILT_STACK_ADJUSTED(_s) (_s * (__SIZEOF_POINTER__ >> 2) + BUILT_STACK_OVERHEAD)

typedef int btask_t;
struct task_entry {
	pthread_t pid;
	char name[16];
	bool isused;
    bool joinable; // false : background task, true: front task
};
static pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct task_entry taskmap[CONFIG_BUILDAPP_MAX_TASKS] = {0};

typedef struct {
    int (*entry)(int argc, char *argv[]);
	char name[16];
	int argc;
	char *argv[];
	// strings are allocated after the struct data
} build_pthdata_t;

static void *built_task_entry(void *ptr)
{
    int ret = 0;
	build_pthdata_t *data = (build_pthdata_t *) ptr;

	pthread_setname_np(pthread_self(), data->name);
	ret = data->entry(data->argc, data->argv);

	free(ptr);
	built_task_exit(0);

	return NULL;
}

btask_t built_task_spawn(const char *name, int scheduler, int priority, 
    int stack_size, int (*entry)(int argc, char *argv[]), char *const argv[], bool joinable)
{
	int argc = 0;
	unsigned int len = strlen(name) + 1;
	pthread_attr_t attr;
	struct sched_param param;
	char *p = (char *)argv;

	while (p != (char *)NULL) {
		p = argv[argc];
		if (p == (char *)NULL) {
			break;
		}

		++argc;
		len += strlen(p) + 1;
	}

	unsigned long structsize = sizeof(build_pthdata_t) + (argc + 2) * sizeof(char *);
	build_pthdata_t *taskdata = (build_pthdata_t *)malloc(structsize + len);
	if (taskdata == NULL) {
		return -ENOMEM;
	}

	memset(taskdata, 0, structsize + len);
	strncpy(taskdata->name, name, 16);
	taskdata->name[15] = '\0';
	taskdata->entry = entry;
	taskdata->argc = argc + 1;

	char *offset = (char *)taskdata + structsize;
	taskdata->argv[0] = offset;
	strcpy(offset, name);
	offset += strlen(name) + 1;

	for (int i = 0; i < argc; ++i) {
		taskdata->argv[i + 1] = offset;
		strcpy(offset, argv[i]);
		offset += strlen(argv[i]) + 1;
	}

	taskdata->argv[argc+1] = (char *)NULL;

	pthread_attr_init(&attr);
	if (stack_size < 512) {
		stack_size = 512;
	}

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	int rv = pthread_attr_setstacksize(&attr, BUILT_STACK_ADJUSTED(stack_size));
	if (rv != 0) {
		goto error_task_init;
	}

	rv = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (rv != 0) {
		goto error_task_init;
	}

	rv = pthread_attr_setschedpolicy(&attr, scheduler);
	if (rv != 0) {
		goto error_task_init;
	}

	param.sched_priority = priority;
	rv = pthread_attr_setschedparam(&attr, &param);
	if (rv != 0) {
        goto error_task_init;
	}

	pthread_mutex_lock(&task_mutex);

	btask_t taskid = 0;
	int i;
	for (i = 0; i < CONFIG_BUILDAPP_MAX_TASKS; ++i) {
		if (!taskmap[i].isused) {
            strncpy(taskmap[i].name, name, 16 - 1);
            taskmap[i].name[16 - 1] = '\0';
			taskmap[i].isused = true;
			taskid = i;
			break;
		}
	}
	if (i >= CONFIG_BUILDAPP_MAX_TASKS) {
		pthread_attr_destroy(&attr);
		pthread_mutex_unlock(&task_mutex);
		free(taskdata);
		return -ENOSPC;
	}

    taskmap[taskid].joinable = joinable;
	rv = pthread_create(&taskmap[taskid].pid, &attr, &built_task_entry, (void *)taskdata);
	if (rv != 0) {
		if (rv == EPERM) {
			// WARNING: NOT RUNING AS ROOT, UNABLE TO RUN REALTIME THREADS
			rv = pthread_create(&taskmap[taskid].pid, NULL, &built_task_entry, (void *) taskdata);
			if (rv != 0) {
				taskmap[taskid].isused = false;
				pthread_attr_destroy(&attr);
				pthread_mutex_unlock(&task_mutex);
				free(taskdata);
				return (rv < 0) ? rv : -rv;
			}
		} else {
			pthread_attr_destroy(&attr);
			pthread_mutex_unlock(&task_mutex);
			free(taskdata);
			return (rv < 0) ? rv : -rv;
		}
	}

	pthread_attr_destroy(&attr);
	pthread_mutex_unlock(&task_mutex);
	return taskid;

error_task_init:
	pthread_attr_destroy(&attr);
	free(taskdata);
	return (rv < 0) ? rv : -rv;
}

void built_task_exit(int ret)
{
#if 0
	pthread_t pid = pthread_self();

    pthread_mutex_lock(&task_mutex);
	int i;
	for (i = 0; i < CONFIG_BUILDAPP_MAX_TASKS; ++i) {
		if (taskmap[i].pid == pid) {
			taskmap[i].isused = false;
			break;
		}
	}

	pthread_mutex_unlock(&task_mutex);
	pthread_exit((void *)(unsigned long)ret);
#else
	pthread_t pid = pthread_self();

    pthread_mutex_lock(&task_mutex);
	int i;
	for (i = 0; i < CONFIG_BUILDAPP_MAX_TASKS; ++i) {
		if (taskmap[i].pid == pid) {
            if (!taskmap[i].joinable) {
                taskmap[i].isused = false;
                taskmap[i].name[0] = '\0';
            }
			break;
		}
	}

	// if (i >= CONFIG_BUILDAPP_MAX_TASKS)  {
	// 	// self task not found
	// }

	pthread_mutex_unlock(&task_mutex);
	pthread_exit((void *)(unsigned long)ret);
#endif
}

int built_task_delete(btask_t id)
{
	int rv = 0;
	pthread_t pid;

	if (id < CONFIG_BUILDAPP_MAX_TASKS && taskmap[id].isused) {
		pid = taskmap[id].pid;
	} else {
		return -EINVAL;
	}

	pthread_mutex_lock(&task_mutex);

	if (pthread_self() == pid) {
		pthread_join(pid, NULL);
		taskmap[id].isused = false;
        taskmap[id].name[0] = '\0';
		pthread_mutex_unlock(&task_mutex);
		pthread_exit(NULL);
	} else {
		rv = pthread_cancel(pid);
	}

    taskmap[id].name[0] = '\0';
	taskmap[id].isused = false;
	pthread_mutex_unlock(&task_mutex);
	return rv;
}

int built_task_join(btask_t id)
{
    pthread_t pid;
    bool is_joinable;

    pthread_mutex_lock(&task_mutex);
    if (id >= CONFIG_BUILDAPP_MAX_TASKS || !taskmap[id].isused) {
        pthread_mutex_unlock(&task_mutex);
        return -EINVAL;
    }

    if (!taskmap[id].joinable) {
        pthread_mutex_unlock(&task_mutex);
        return -EPERM;
    }

    pid = taskmap[id].pid;
    pthread_mutex_unlock(&task_mutex);

    int ret = pthread_join(pid, NULL);
    if (ret != 0) {
        return ret;
    }

    pthread_mutex_lock(&task_mutex);
    if (taskmap[id].isused && taskmap[id].pid == pid) {
        taskmap[id].isused = false;
        taskmap[id].name[0] = '\0';
        taskmap[id].joinable = false;
    }
    pthread_mutex_unlock(&task_mutex);

    return 0;
}

int built_task_kill(btask_t id, int sig)
{
	int rv = 0;
	pthread_t pid;

	if (id < CONFIG_BUILDAPP_MAX_TASKS && taskmap[id].isused && taskmap[id].pid != 0) {
		pthread_mutex_lock(&task_mutex);
		pid = taskmap[id].pid;
		pthread_mutex_unlock(&task_mutex);
	} else {
		return -EINVAL;
	}

	// If current thread then exit, otherwise cancel
	rv = pthread_kill(pid, sig);
	return rv;
}

void built_show_tasks()
{
	int idx;
	int count = 0;

	KMINFO("Active Tasks:");

	for (idx = 0; idx < CONFIG_BUILDAPP_MAX_TASKS; idx++) {
		if (taskmap[idx].isused) {
			KMINFO("   %-10s %lu\r\n", taskmap[idx].name, (unsigned long)taskmap[idx].pid);
			count++;
		}
	}

	if (count == 0) {
		KMINFO("   No running tasks");
	}
}

bool built_task_is_running(const char *taskname)
{
	int idx;
	for (idx = 0; idx < CONFIG_BUILDAPP_MAX_TASKS; idx++) {
		if (taskmap[idx].isused && (strcmp(taskmap[idx].name, taskname) == 0)) {
			return true;
		}
	}
	return false;
}

btask_t built_getpid()
{
	pthread_t pid = pthread_self();
	btask_t ret = -1;

	pthread_mutex_lock(&task_mutex);
	for (int i = 0; i < CONFIG_BUILDAPP_MAX_TASKS; i++) {
		if (taskmap[i].isused && taskmap[i].pid == pid) {
			ret = i;
		}
	}
	pthread_mutex_unlock(&task_mutex);
	return ret;
}

const char *built_get_taskname()
{
	pthread_t pid = pthread_self();
	const char *prog_name = "UnknownApp";

	pthread_mutex_lock(&task_mutex);
	for (int i = 0; i < CONFIG_BUILDAPP_MAX_TASKS; i++) {
		if (taskmap[i].isused && taskmap[i].pid == pid) {
			prog_name = taskmap[i].name;
		}
	}
	pthread_mutex_unlock(&task_mutex);

	return prog_name;
}

