#include "compiler.h"
#include "kernel.h"
#include "const.h"
#include "type.h"
#include "protect.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
#include "bitmap.h"
#include "proc.h"
#include "global.h"
#include "preempt.h"
#include "atomic.h"
#include "sched.h"

PRIVATE inline u32 task_timeslice(PROCESS* proc)
{
	return BASE_TIMESLICE(proc);
}

PRIVATE void dequeue(PROCESS* proc, prio_array_t* array)
{
	--array->nr_active;
	list_del(&proc->run_list);
	if(list_empty(array->queue + proc->prio))
	{
		bitmap_set(&array->bm, proc->prio, 0);
	}
}

PRIVATE void enqueue(PROCESS* proc, prio_array_t* array)
{
	list_add_tail(array->queue + proc->prio, &proc->run_list);
	bitmap_set(&array->bm, proc->prio, 1);
	++array->nr_active;
	proc->array = array;
}

PRIVATE void enqueue_head(PROCESS* proc, prio_array_t* array)
{
	list_add(array->queue + proc->prio, &proc->run_list);
	bitmap_set(&array->bm, proc->prio, 1);
	++array->nr_active;
	proc->array = array;
}

PRIVATE void requeue(PROCESS* proc, prio_array_t* array)
{
	list_move_tail(&proc->run_list, array->queue + proc->prio);
}

PRIVATE int effective_prio(PROCESS* proc)
{
	int bonus, prio;

	if (IS_RT(proc))
		return proc->prio;

	bonus = CURRENT_BONUS(proc) - MAX_BONUS / 2;

	prio = proc->static_prio - bonus;
	if (prio < MAX_RT_PRIO)
		prio = MAX_RT_PRIO;
	if (prio > MAX_PRIO - 1)
		prio = MAX_PRIO - 1;
	return prio;
}

PRIVATE inline void __activate_task(PROCESS* proc, runqueue_t* rq)
{
	enqueue(proc, rq->active);
	rq->nr_running++;
}

/* Priority Recalculation */
PRIVATE void update_task_prio(PROCESS* proc, u32 now)
{
	u32 sleep_time;
	if(now < proc->timestamp) sleep_time = now + (MAX_MSEC -  proc->timestamp);
	else sleep_time = now - proc->timestamp;
	if(sleep_time > MAX_SLEEP_AVG) sleep_time = MAX_SLEEP_AVG;

	if(likely(sleep_time > 0))
	{
		if(IS_USER_PROC(proc) && proc->activated != -1 && sleep_time > INTERACTIVE_SLEEP(proc))
		{
			proc->sleep_avg = (MAX_SLEEP_AVG - AVG_TIMESLICE);
			if(!HIGH_CREDIT(proc)) ++proc->interactive_credit;
		}
		else
		{
			sleep_time *= (MAX_BONUS - CURRENT_BONUS(proc)) ? : 1;
			if (LOW_CREDIT(proc) && sleep_time > task_timeslice(proc))
			{
				sleep_time = task_timeslice(proc);
			}

			if (IS_USER_PROC(proc) && proc->activated == -1 && !HIGH_CREDIT(proc))
			{
				if (proc->sleep_avg >= INTERACTIVE_SLEEP(proc))
				{
					sleep_time = 0;
				}
				else if (proc->sleep_avg + sleep_time >= INTERACTIVE_SLEEP(proc))
				{
					proc->sleep_avg = INTERACTIVE_SLEEP(proc);
					sleep_time = 0;
				}
			}

			proc->sleep_avg += sleep_time;

			if (proc->sleep_avg > MAX_SLEEP_AVG)
			{
				proc->sleep_avg = MAX_SLEEP_AVG;
				if (!HIGH_CREDIT(proc))
				{
					++proc->interactive_credit;
				}
			}
		}
	}

	proc->prio = effective_prio(proc);
}

PUBLIC inline void activate_task(PROCESS* proc, runqueue_t* rq)
{
	u32 now = sched_clock();

	update_task_prio(proc, now);

	proc->timestamp = now;

	__activate_task(proc, rq);
}

PUBLIC inline deactivate_task(PROCESS* proc, runqueue_t* rq)
{
	--rq->nr_running;

	dequeue(proc, proc->array);

	proc->array = NULL;
}

void sched_tick()
{
	PROCESS* proc = current();
	runqueue_t *rq = this_rq();

	if(proc->array != rq->active)
	{
		printk("warning!\tcunrrent->array != rq->active\n");
		BREAK_POINT;
	}

	if(unlikely(IS_RT(proc)))
	{
		if(--proc->time_slice < 0)
		{
			proc->time_slice = task_timeslice(proc);
			dequeue(proc, rq->active);
			enqueue(proc, rq->active);
		}
	}
	else if(--proc->time_slice < 0)
	{
		dequeue(proc, rq->active);
		proc->prio = effective_prio(proc);
		proc->time_slice = task_timeslice(proc);

		if (!rq->expired_timestamp)
		{
			rq->expired_timestamp = sched_clock();
		}
		if (!TASK_INTERACTIVE(proc) || EXPIRED_STARVING(rq)) 
		{
			enqueue(proc, rq->expired);
		} 
		else
		{
			enqueue(proc, rq->active);
		}
	}
}

PUBLIC void schedule()
{
	PROCESS* prev, * next;
	runqueue_t* rq;
	prio_array_t* array;
	struct list_head* queue;
	u32 now;
	u32 run_time;
	int idx;

	preempt_disable();

	prev = current();
	rq = this_rq();
	now = sched_clock();

	if (likely(now - prev->timestamp < MAX_SLEEP_AVG))
	{
		run_time = now - prev->timestamp;
	}
	else
	{
		run_time = MAX_SLEEP_AVG;
	}

	if (HIGH_CREDIT(prev))
	{
		run_time /= (CURRENT_BONUS(prev) ? : 1);
	}

	array = rq->active;
	if(unlikely(!array->nr_active))
	{
		rq->active = rq->expired;
		rq->expired = array;

		array = rq->active;
		rq->expired_timestamp = 0;
	}

	idx = bitmap_find_first_one(&array->bm);
	queue = array->queue + idx;
	next = list_entry(queue->next, PROCESS, run_list);
	next->activated = 0;

	prefetch(next);
	prev->sleep_avg = prev->sleep_avg > run_time ? prev->sleep_avg - run_time : 0;
	if(prev->sleep_avg == 0)
	{
		if(!(HIGH_CREDIT(prev) || LOW_CREDIT(prev)))
		{
			prev->interactive_credit--;
		}
	}
	prev->timestamp = now;

	if (likely(prev != next)) 
	{
		next->timestamp = now;
		rq->nr_switches++;
		rq->curr = next;

		barrier();

		proc_ready = next;
	}

	preempt_enable();
}

PUBLIC void schedule_old_version()
{
	preempt_disable();
	PROCESS *p;
	int greatest_ticks = 0;

	while(!greatest_ticks)
	{
		for(p = &FIRST_PROC; p <= &LAST_PROC; ++p)
		{
			if(p->pflags == 0)
			{
				if(p->time_slice > greatest_ticks)
				{
					greatest_ticks = p->time_slice;
					proc_ready = p;
				}
			}
		}
		if(!greatest_ticks)
		{
			for(p = &FIRST_PROC; p <= &LAST_PROC; ++p)
			{
				if(p->pflags == 0)
				{
					p->time_slice = p->prio;
				}
			}
		}
	}
	preempt_enable();
}

PUBLIC void block(PROCESS *proc)
{
	assert(proc->pflags);

	deactivate_task(proc, this_rq());

	schedule();
}

PUBLIC void unblock(PROCESS *proc)
{
	assert(proc->pflags == 0);

	activate_task(proc, this_rq());

	schedule();
}

PUBLIC int task_prio(PROCESS* proc)
{
	return proc->prio - MAX_RT_PRIO;
}

PUBLIC int task_nice(PROCESS* proc)
{
	return TASK_NICE(proc);
}

PUBLIC void sched_init()
{
	int i, j;
	prio_array_t* array;

	runqueue.curr = NULL;
	runqueue.idle = NULL;

	runqueue.active = &runqueue.arrays[0];
	runqueue.expired = &runqueue.arrays[1];

	runqueue.nr_running = 0;
	runqueue.nr_switches = 0;

	runqueue.expired_timestamp = 0;

	for (i = 0; i < 2; ++i) 
	{
		array = runqueue.arrays + i;
		bitmap_init(&array->bm, prio_array_1, MAX_PRIO);
		for (j = 0; j < MAX_PRIO; ++j) 
		{
			list_init(array->queue + j);
		}
	}

	for(i = 0; i < NR_TASKS + NR_NATIVE_PROCS; ++i)
	{
		activate_task(&proc_table[i], &runqueue);
	}
}
