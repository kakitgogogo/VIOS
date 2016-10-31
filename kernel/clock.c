#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "string.h"
#include "global.h"

PUBLIC void clock_handler(int irq)
{
	//disp_str(".");
	++ticks;
	--proc_ready->ticks;

	if(k_reenter != 0)
	{
		//disp_str("!");
		return;
	}

	if(proc_ready->ticks > 0)
	{
		return;
	}

	schedule();
	
	/*
	proc_ready++;
	if(proc_ready >= proc_table + NR_TASKS)
	{
		proc_ready = proc_table;
	}
	*/
}

PUBLIC void milli_delay(int milli_sec)
{
	int t = get_ticks();

	while(((get_ticks() - t) * 1000 / HZ) < milli_sec);
}