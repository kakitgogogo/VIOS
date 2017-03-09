#include "const.h"
#include "type.h"
#include "protect.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

PUBLIC void clock_init()
{
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (u8)(TIMER_FREQ / HZ));
	out_byte(TIMER0, (u8)((TIMER_FREQ / HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);
}

PUBLIC void clock_handler(int irq)
{
	if(++ticks >= MAX_TICKS)
	{
		ticks = 0;
	}

	sched_tick();

	if(key_pressed) inform_int(TASK_TTY);

	if(isPreemptDisabled) return;

	if(k_reenter != 0) return;

	if(proc_ready->time_slice > 0) return;

	schedule();
}

PUBLIC u32 sched_clock()
{
	return ticks * 1000 / HZ;
}

PUBLIC void milli_delay(int milli_sec)
{
	int t = get_ticks();

	while(((get_ticks() - t) * 1000 / HZ) < milli_sec);
}
