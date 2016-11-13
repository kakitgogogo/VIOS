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
#include "keyboard.h"

PUBLIC int kernel_main()
{
	disp_str("Welcome to VIOS\n");

	PROCESS* proc = proc_table;
	TASK* task = task_table;
	char* stack_top = task_stack + STACK_SIZE_TOTAL;
	u16 selector_ldt = SELECTOR_LDT_FIRST;	

	int i;
	int tty_ids[NR_TASKS + NR_PROCS] = {0, 0, 0, 0, 1, 2, 2};
	u8 privilege, rpl;
	u32 eflags, prio;
	for(i = 0; i < NR_TASKS + NR_PROCS; ++i)
	{
		if(i < NR_TASKS)
		{
			task = task_table + i;
			privilege = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202;
			prio = 15;
		}
		else
		{
			task = user_proc_table + (i - NR_TASKS);
			privilege = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202;
			prio = 5;
		}
		strcpy(proc->pname, task->name);

		proc->pid = i;

		proc->ldt_selector = selector_ldt;

		memcpy(&proc->ldts[0], &gdt[SELECTOR_KERNEL_CS>>3], sizeof(DESCRIPTOR));
		proc->ldts[0].attr1 = DA_C | privilege << 5;

		memcpy(&proc->ldts[1], &gdt[SELECTOR_KERNEL_DS>>3], sizeof(DESCRIPTOR));
		proc->ldts[1].attr1 = DA_DRW | privilege << 5;

		proc->regs.cs = (SELECTOR_IN_LDT(0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.ds = (SELECTOR_IN_LDT(1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.es = (SELECTOR_IN_LDT(1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.fs = (SELECTOR_IN_LDT(1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.ss = (SELECTOR_IN_LDT(1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK & SA_TI_MASK) | SA_TIG | rpl;

		proc->regs.eip = (u32)task->initial_eip;
		proc->regs.esp = (u32)(stack_top);
		proc->regs.eflags = eflags;

		proc->ticks = proc->priority = prio;

		proc->pflags = 0;
		proc->pmsg = 0;
		proc->recvfrom = NO_TASK;
		proc->sendto = NO_TASK;
		proc->has_int_msg = FALSE;
		proc->sending = 0;
		proc->next_sending = 0;

		proc->tty_id = tty_ids[i];

		stack_top -= task->stacksize;	
		++proc;
		++task;
		selector_ldt += 1<<3;
	}
	
	ticks = 0;
	k_reenter = 0;

	proc_ready = proc_table;

	clock_init();
	keyboard_init();

	restart();

	while(1);
}

#if 1
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}
#endif

void testA()
{
	while(1)
	{
		printf("<Ticks: %d>", get_ticks());
		milli_delay(1000);
	}
}

void testB()
{
	while(1)
	{
		printf("B");
		milli_delay(1000);
	}
}

void testC()
{
	while(1)
	{
		printf("C");
		milli_delay(200);
	}
}