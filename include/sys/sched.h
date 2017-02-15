#ifndef	VIOS_SCHED_H
#define	VIOS_SCHED_H

#include "bitmap.h"
#include "atomic.h"
#include "proc.h"

#define ON_RUNQUEUE_WEIGHT	30
#define CHILD_PENALTY		95
#define PARENT_PENALTY		100
#define EXIT_WEIGHT		3

#define NS_MAX_SLEEP_AVG	(JIFFIES_TO_NS(MAX_SLEEP_AVG))
#define NODE_THRESHOLD		125

/*  Time Slice Interval */
#define	MIN_TIMESLICE		( 10 * HZ / 1000)    // HZ--define in const.h 
#define	MAX_TIMESLICE		(200 * HZ / 1000)	// Now HZ = 1000, so MIN_TIMESLICE = 10ms, MAX_TIMESLICE = 200ms

/* Number of Real Time Priority and User Priority */
#define	MAX_RT_PRIO			100
#define	MAX_PRIO				(MAX_RT_PRIO + 40)	//140
#define	USER_PRIO(p)			((p) - MAX_RT_PRIO)
#define	MAX_USER_PRIO		(USER_PRIO(MAX_PRIO))	//40

/* About Nice and Prio */
#define	NICE_TO_PRIO(nice)	(MAX_RT_PRIO + (nice) + 20)
#define	PRIO_TO_NICE(prio)	((prio) - MAX_RT_PRIO - 20)
#define	TASK_NICE(p)			PRIO_TO_NICE((p)->static_prio)

/* About Bonus and Time Slice*/
#define	PRIO_BONUS_RATIO		25
#define	MAX_BONUS			(MAX_USER_PRIO * PRIO_BONUS_RATIO / 100)	//10
#define	MAX_SLEEP_AVG		(AVG_TIMESLICE * MAX_BONUS)				//about 1000ms
#define	AVG_TIMESLICE		\
	(MIN_TIMESLICE + ((MAX_TIMESLICE - MIN_TIMESLICE) * (MAX_PRIO - 1 - NICE_TO_PRIO(0)) / (MAX_USER_PRIO - 1)))	// about 100ms, default process's time slice

#define	CURRENT_BONUS(p)		(p->sleep_avg * MAX_BONUS / MAX_SLEEP_AVG)
#define	BASE_TIMESLICE(p)	((MAX_TIMESLICE - MIN_TIMESLICE) * (MAX_PRIO - 1 - (p)->static_prio) / (MAX_USER_PRIO - 1))

/* About Interactive Proccess */
#define	INTERACTIVE_DELTA		2
#define	SCALE(v1,v1_max,v2_max) 	(v1) * (v2_max) / (v1_max)
#define	DELTA(p) 					(SCALE(TASK_NICE(p), 40, MAX_USER_PRIO * PRIO_BONUS_RATIO / 100) + INTERACTIVE_DELTA)
#define	TASK_INTERACTIVE(p) 		((p)->prio <= (p)->static_prio - DELTA(p))
#define	INTERACTIVE_SLEEP(p) 		((MAX_SLEEP_AVG * (MAX_BONUS / 2 + DELTA((p)) + 1) / MAX_BONUS - 1))

#define	CREDIT_LIMIT		100
#define	HIGH_CREDIT(p) 	((p)->interactive_credit > CREDIT_LIMIT)
#define	LOW_CREDIT(p) 	((p)->interactive_credit < -CREDIT_LIMIT)

//About Prevention of Starvation
#define STARVATION_LIMIT	(MAX_SLEEP_AVG)
#define EXPIRED_STARVING(rq) ((sched_clock() - (rq)->expired_timestamp >= STARVATION_LIMIT * ((rq)->nr_running) + 1))

// whether a process is a real time process
#define	IS_RT(p)				((p)->prio < MAX_RT_PRIO)

#define this_rq()				(&runqueue)

typedef struct s_proc PROCESS;

typedef struct prio_array
{
	int nr_active;
	bitmap bm;
	list_head queue[MAX_PRIO];
}prio_array_t;

typedef struct runqueue
{
	PROCESS* curr;
	PROCESS* idle;
	prio_array_t* active;
	prio_array_t* expired;
	prio_array_t arrays[2];

	u32 nr_running;
	u32 nr_switches;

	u32 expired_timestamp;
}runqueue_t;

#endif