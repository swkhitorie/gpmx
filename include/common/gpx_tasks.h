#pragma once

#include <stdbool.h>

#include <pthread.h>
#include <sched.h>

#define SCHED_DEFAULT	SCHED_FIFO
#define SCHED_PRIORITY_MAX sched_get_priority_max(SCHED_FIFO)
#define SCHED_PRIORITY_MIN sched_get_priority_min(SCHED_FIFO)
#define SCHED_PRIORITY_DEFAULT (((sched_get_priority_max(SCHED_FIFO) - sched_get_priority_min(SCHED_FIFO)) / 2) + sched_get_priority_min(SCHED_FIFO))

#define PR_SET_NAME	1

typedef int gpx_task_t;

typedef struct {
	int argc;
	char **argv;
} gpx_task_args_t;


// work queue starting high priority
#define GPX_WQ_HP_BASE (SCHED_PRIORITY_MAX - 15)

// Fast drivers - they need to run as quickly as possible to minimize control
// latency.
#define SCHED_PRIORITY_FAST_DRIVER		(SCHED_PRIORITY_MAX - 0)

// Actuator outputs should run as soon as the rate controller publishes
// the actuator controls topic
#define SCHED_PRIORITY_ACTUATOR_OUTPUTS		(GPX_WQ_HP_BASE - 3)

// Attitude controllers typically are in a blocking wait on driver data
// they should be the first to run on an update, using the current sensor
// data and the *previous* attitude reference from the position controller
// which typically runs at a slower rate
#define SCHED_PRIORITY_ATTITUDE_CONTROL		(GPX_WQ_HP_BASE - 4)

// Estimators should run after the attitude controller but before anything
// else in the system. They wait on sensor data which is either coming
// from the sensor hub or from a driver. Keeping this class at a higher
// priority ensures that the estimator runs first if it can, but will
// wait for the sensor hub if its data is coming from it.
#define SCHED_PRIORITY_ESTIMATOR		(GPX_WQ_HP_BASE - 5)

// Position controllers typically are in a blocking wait on estimator data
// so when new sensor data is available they will run last. Keeping them
// on a high priority ensures that they are the first process to be run
// when the estimator updates.
#define SCHED_PRIORITY_POSITION_CONTROL		(GPX_WQ_HP_BASE - 7)

// The log capture (which stores log data into RAM) should run faster
// than other components, but should not run before the control pipeline
#define SCHED_PRIORITY_LOG_CAPTURE		(GPX_WQ_HP_BASE - 10)

// Slow drivers should run at a rate where they do not impact the overall
// system execution
#define SCHED_PRIORITY_SLOW_DRIVER		(GPX_WQ_HP_BASE - 35)

// The navigation system needs to execute regularly but has no realtime needs
#define SCHED_PRIORITY_NAVIGATION		(SCHED_PRIORITY_DEFAULT + 5)
#define SCHED_PRIORITY_LOG_WRITER		(SCHED_PRIORITY_DEFAULT - 10)
#define SCHED_PRIORITY_PARAMS			(SCHED_PRIORITY_DEFAULT - 15)

typedef int (*gpx_main_t)(int argc, char *argv[]);

__BEGIN_DECLS

/** Starts a task and performs any specific accounting, scheduler setup, etc. */
__EXPORT gpx_task_t gpx_task_spawn_cmd(const char *name,
                        int scheduler,
                        int priority,
                        int stack_size,
                        gpx_main_t entry,
                        char *const argv[]);

/** Deletes a task - does not do resource cleanup **/
__EXPORT int gpx_task_delete(gpx_task_t pid);

/** Send a signal to a task **/
__EXPORT int gpx_task_kill(gpx_task_t pid, int sig);

/** Exit current task with return value **/
__EXPORT void gpx_task_exit(int ret);

/** Show a list of running tasks **/
__EXPORT void gpx_show_tasks(void);

/** See if a task is running **/
__EXPORT bool gpx_task_is_running(const char *taskname);

/** return the name of the current task */
__EXPORT const char *gpx_get_taskname(void);

__END_DECLS
