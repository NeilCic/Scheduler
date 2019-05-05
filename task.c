#include <assert.h>
#include <stdlib.h>

#include "task.h"

/*
*	return value - pointer to task
*	arguments - task function pointer, pointer to task function arguments, timing value
*	this function creates a new task, with the provided arguments
*/
struct task_t
{
	nuid_t uid;
	int (*task_func)(void *args);
	void *args;
	struct timeval timing;
	struct timeval next_iteration;
};

task_t *TaskCreate(task_func task, void *args, struct timeval timing)
{
	task_t *new_task = NULL;	

	new_task = malloc(sizeof(task_t));
	if (new_task)
	{
		new_task->task_func = task;
		new_task->args = args;
		new_task->timing.tv_sec = timing.tv_sec;
		new_task->timing.tv_usec = timing.tv_usec;
		new_task->next_iteration.tv_sec = timing.tv_sec;
		new_task->next_iteration.tv_usec = timing.tv_usec;
		new_task->uid = UidCreate();
		if (!UidIsValid(TaskGetUID(new_task)))
		{
			new_task->uid = g_bad_uid;
		}
	}

	return new_task;
}

/*
*	return value - unique id struct
*	arguments - pointer to a specific task
*	this function destroys the given task, returning its id
*/
nuid_t TaskDestroy(task_t *task)
{
	nuid_t uid = {0};
	
	assert(task);
	
	uid = task->uid;
	
	free(task);
	task = NULL;

	return uid;
}

/*
*	return value - unique id struct
*	arguments - const pointer to task
*	this function receives a pointer to a task, and returns
*	a pointer to the unique id of that task
*/
nuid_t TaskGetUID(const task_t *task)
{
	assert(task);

	return task->uid;
}

/*
*	return value - timeval struct
*	arguments - const pointer to task
*	this function receives a pointer to a task, and returns
*	the time of its next iteration
*/
int TaskGetNextRunTime(const task_t *task)
{
	assert(task);

	return task->next_iteration.tv_sec;
}

/*
*	return value - pointer to task
*	arguments - pointer to task
*	this function receives a pointer to a task to re add
*	to the schedule, and updates its next run time
*/
void TaskUpdateNextRunTime(task_t *task)
{
	assert(task);

	task->next_iteration.tv_sec += task->timing.tv_sec;
	task->next_iteration.tv_usec += task->timing.tv_usec;
}

/*
*	return value - return value of the task
*	arguments - pointer to task
*	this function receives a task pointer, and executes it
*	returning the return value of that task
*/
int TaskRun(task_t *task)
{
	assert(task);

	return ((task->task_func)(task->args));
}
