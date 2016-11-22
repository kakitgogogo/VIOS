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

PUBLIC void schedule()
{
	PROCESS *p;
	int greatest_ticks = 0;

	while(!greatest_ticks)
	{
		for(p = &FIRST_PROC; p <= &LAST_PROC; ++p)
		{
			if(p->pflags == 0)
			{
				if(p->ticks > greatest_ticks)
				{
					greatest_ticks = p->ticks;
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
					p->ticks = p->priority;
				}
			}
		}
	}
}

PUBLIC int sys_get_ticks()
{
	return ticks;
}

PUBLIC int sys_write(int _unused1, char* buf, int len, PROCESS* proc)
{
	tty_write(&tty_table[proc->tty_id], buf, len);
	return 0;
}


PUBLIC int ldt_seg_linear(PROCESS *proc, int idx)
{
	DESCRIPTOR *desc = &proc->ldts[idx];

	return desc->base_high << 24 | desc->base_mid << 16 | desc->base_low;
}

PUBLIC void* va2la(int pid, void* va)
{
	PROCESS* proc = &proc_table[pid];

	u32 seg_base = ldt_seg_linear(proc, INDEX_LDT_RW);
	u32 la = seg_base + (u32)va;

	if(pid < NR_TASKS + NR_PROCS)
	{
		assert(la == (u32)va);
	}

	return (void*)la;
}

PUBLIC void tty_write(TTY* tty, char* buf, int len)
{
	char *p = buf;
	int i = len;

	while(i)
	{
		out_char(tty->console, *p++);
		--i;
	}
}