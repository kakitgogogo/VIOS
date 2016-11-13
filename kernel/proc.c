#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
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

PUBLIC int sys_printx(int _unused1, int _unused2, char* s, PROCESS* proc)
{
	const char *p;
	char ch;

	char reenter_err[] = "? k_reenter is incorrect for unknown reason";
	reenter_err[0] = MAG_CH_PANIC;

	if(k_reenter == 0)
		p = va2la(proc2pid(proc), s);
	else if(k_reenter > 0)
		p = s;
	else
		p = reenter_err;

	if((*p == MAG_CH_PANIC) || (*p == MAG_CH_ASSERT && proc_ready < &proc_table[NR_TASKS]))
	{
		disable_int();
		char *v = (char*)V_MEM_BASE;
		const char *q = p + 1;

		while(v < (char*)(V_MEM_BASE + V_MEM_SIZE))
		{
			*v++ = *q++;
			*v++ = RED_CHAR;
			if(!*q)
			{
				while(((int)v - V_MEM_BASE) % (SCREEN_WIDTH * 8))
				{
					*v++ = ' ';
					*v++ = RED_CHAR;
				}
				break;
			}
		}
		__asm__ __volatile__("hlt");
	}

	while((ch = *p++) != 0)
	{
		if(ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT)
			continue;

		out_char(tty_table[proc->tty_id].console, ch);
	}

	return 0;
}