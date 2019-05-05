#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "priority_queue_heap.h"
#include "task.h"
#include "scheduler.h"

#define EMPTY 0 
#define STOPPED 1 
#define ENQFAIL 2 
#define WTFNUMBER 100

struct sch_t
{
	p_queue_t *pqueue;
	task_t *curr_task;
	int is_running;
};

int SchedIsBefore(const void *data1, const void *data2, void *param)
{
	(void)param;
	return (TaskGetNextRunTime(data1) < TaskGetNextRunTime(data2));
}

unsigned int SchedIsMatch(void * const data1, void * const data2)
{
	return (UidIsSame(TaskGetUID(data1), TaskGetUID(data2)));
}

sch_t *SchCreate(void)
{
	sch_t *new = malloc(sizeof(sch_t));
	if (new)
	{
		new->is_running = 0;
		new->curr_task = NULL;

		new->pqueue = PQCreate(SchedIsBefore, WTFNUMBER, NULL);
		if (!new->pqueue)
		{
			free(new);
			new = NULL;
			new->pqueue = NULL;
		}
	}

	return new;
}

/*
*	return value - none
*	arguments - scheduler management struct
*	this function receives a schedular management struct
*	and frees its allocated memory
*/
void SchDestroy(sch_t *sch)
{
	assert(sch);

	SchClear(sch);

	PQDestroy(sch->pqueue);

	free(sch);
	sch = NULL;
}

/*
*	return value - unique id struct
*	arguments - scheduler management struct, task function pointer, 
*	pointer to task function arguments, timing value
*	this function adds a new task into the schedule, with the given
*	arguments, where timing determines the place in the schedule
*/
nuid_t SchAdd(sch_t *sch, task_func task, void *args, struct timeval timing)
{
	task_t *new_task = NULL;

	assert(sch);
	assert(task);

	new_task = TaskCreate(task, args, timing);
	if (new_task)
	{
		 if (PQEnqueue(sch->pqueue, new_task))
		 {
		 	TaskDestroy(new_task);
			return g_bad_uid;
		 }
	}

	return (new_task? TaskGetUID(new_task) : g_bad_uid);
}

/*
*	return value - unique id struct
*	arguments - scheduler management struct, unique id struct
*	this function finds and removes a task with the received unique id 
*	from the schedule, returning the id of the removed task
*/

nuid_t SchRemove(sch_t *sch, nuid_t id)
{
	task_t *removed_task = NULL;

	assert(sch);

	removed_task = (task_t *)PQErase(sch->pqueue, SchedIsMatch, &id);

	if (sch->curr_task && UidIsSame(id, TaskGetUID(sch->curr_task)))
	{
		sch->curr_task = NULL;
		return id;
	}

	return (removed_task? TaskDestroy(removed_task) : g_bad_uid);
}

/*
*	return value - return status
*	arguments - scheduler management struct
*	this function starts the execution phase of the schedule, allowing
*	it to start executing scheduled tasks
*/
int SchRun(sch_t *sch)
{
	int task_timer = 0, next_time = 0;

	task_t *temp = NULL;

	assert(sch);

	sch->is_running = 1;

	while (sch->is_running && !PQIsEmpty(sch->pqueue))
	{
		sch->curr_task = (task_t *)PQDequeue(sch->pqueue);
		temp = sch->curr_task;

		task_timer = TaskGetNextRunTime((task_t *)sch->curr_task);

		sleep(task_timer - next_time);

		next_time = task_timer;

		/*if task status is 1 and curr_task not null, re-ENQ*/
		if (sch->curr_task && TaskRun(sch->curr_task))
		{
			TaskUpdateNextRunTime(sch->curr_task);
			if (PQEnqueue(sch->pqueue, sch->curr_task)) /*if ENQ fails*/
			{
				sch->is_running = 0;
				return ENQFAIL;
			}
		}
		else
		{
			TaskDestroy(temp);
		}

		sch->curr_task = NULL;
	}

	if (!sch->is_running)
	{
		return STOPPED;
	}

	sch->is_running = 0;

	return EMPTY;
}

/*
*	return value - return status
*	arguments - scheduler management struct
*	this function stops the execution phase of the schedule
*/
int SchStop(sch_t *sch)
{
	assert(sch);

	sch->is_running = 0;

	return 0;
}

/*
*	return value - number of tasks in the schedule
*	arguments - scheduler management struct
*	this function returns the number of tasks in the schedule
*/
size_t SchSize(const sch_t *sch)
{
	assert(sch);

	return (PQSize(sch->pqueue) + sch->is_running); /*new*/
}

/*
*	return value - boolean value
*	arguments - scheduler management struct
*	this function returns 1 if the provided schedule is empty
*	and 0 otherwise
*/
int SchIsEmpty(const sch_t *sch)
{
	assert(sch);

	return (PQIsEmpty(sch->pqueue) && !sch->is_running); /*fix this somehow*/
}

/*
*	return value - void
*	arguments - scheduler management struct
*	this function clears all remaining tasks from the schedule
*/
void SchClear(sch_t *sch)
{
	assert(sch);

	while (!PQIsEmpty(sch->pqueue))
	{
		TaskDestroy(PQDequeue(sch->pqueue));
	}
}