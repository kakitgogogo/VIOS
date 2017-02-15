#ifndef	VIOS_SYNC_H
#define	VIOS_SYNC_H

typedef struct semaphore
{
	u8 value;
	list_head waiters;
}semaphore;

typedef struct lock
{
	PROCESS* holder;
	semaphore sem;
	u32 nr_holder_repeat;
}lock;

void sem_init(semaphore* sem, u8 value);
void sem_down(semaphore* sem);
void sem_up(semaphore* sem);
void lock_init(lock* lck);
void lock_acquire(lock* lck);
void lock_release(lock* lck);

#endif