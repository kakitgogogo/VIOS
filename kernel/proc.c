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
#include "sched.h"
#include "list.h"

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

PRIVATE print_align(int maxlen, const char *fmt, va_list args)
{
	int i, len;
	char* buf;

	vsprintf(buf, fmt, args);
	len = strlen(buf);
	for(i = 0; i < len && i < maxlen; ++i)
	{
		printk("%c", buf[i]);
	}
	for(i; i < maxlen; ++i)
	{
		printk(" ");
	}
}

PUBLIC proc_info()
{
	int i;
	PROCESS* proc;

	for(i = 0, proc = proc_table; i < NR_TASKS + NR_PROCS; ++i, ++proc)
	{
		if(proc->pflags & FREE_SLOT == 0)
		{
			print_align(8, "%s", proc->pname);
			print_align(8, "%x", proc->pflags);
			print_align(8, "%d", proc->pid);
			print_align(8, "%d", proc->prio);
			print_align(8, "%d", proc->tty_id);
		}
	}
}