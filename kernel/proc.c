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

PUBLIC PROCESS* current()
{
	return proc_ready;
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

	if(pid < NR_TASKS + NR_NATIVE_PROCS)
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

PUBLIC void proc_info(PROCESS* proc)
{
	printk("\nPROCESS %s(%d) INFO:\n", proc->pname, proc->pid);
	printk("\tldt_selector: 0x%x\n", proc->ldt_selector);
	printk("\tprio: 0x%x\n", proc->prio);
	printk("\ttime_slice: %dms\n", proc->time_slice);
	printk("\ttimestamp: 0x%x\n", proc->timestamp);
	printk("\tinteractive_credit: 0x%x\n", proc->interactive_credit);
}
