#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

PUBLIC int sys_get_ticks()
{
	return ticks;
}

PUBLIC void schedule()
{
	PROCESS *p;
	int greatest_ticks = 0;

	while(!greatest_ticks)
	{
		for(p = proc_table; p < proc_table + NR_TASKS; ++p)
		{
			if(p->ticks > greatest_ticks)
			{
				greatest_ticks = p->ticks;
				proc_ready = p;
			}
		}
		if(!greatest_ticks)
		{
			for(p = proc_table; p < proc_table + NR_TASKS; ++p)
			{
				p->ticks = p->priority;
			}
		}
	}
}