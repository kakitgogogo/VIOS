#include "sync.h"
#include "const.h"
#include "type.h"
#include "list.h"
#include "proc.h"
#include "global.h"

void sem_init(semaphore* sem, u8 value)
{
	sem->value = value;
	list_init(&sem->waiters);
}

void lock_init(lock* lck)
{
	lck->holder = NULL;
	lck->nr_holder_repeat = 0;
	sem_init(&plk->sem, 1);
}

void sem_down(semaphore* sem)
{
	disable_int();
	while(sem->value == 0)
	{
		assert(list_find(&sem->waiters, &running_proc()->self) == LIST_POISON);

		list_add(&sem->waiters, &running_proc()->self);
		proc_block(BLOCKED);
	}

	--sem->value;

	enable_int();
}