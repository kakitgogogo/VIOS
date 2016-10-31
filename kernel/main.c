#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "string.h"
#include "global.h"

PUBLIC int kernel_main()
{
	disp_str("Test Process...\n");

	PROCESS* proc = proc_table;
	TASK* task = task_table;
	char* stack_top = task_stack + STACK_SIZE_TOTAL;
	u16 selector_ldt = SELECTOR_LDT_FIRST;	

	int i;
	int prio[NR_TASKS] = {15, 5, 3};
	for(i = 0; i < NR_TASKS; ++i)
	{
		//strcpy(proc->pname, task->name);

		proc->pid = i;

		proc->ldt_selector = selector_ldt;

		memcpy(&proc->ldts[0], &gdt[SELECTOR_KERNEL_CS>>3],sizeof(DESCRIPTOR));
		proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;

		memcpy(&proc->ldts[1], &gdt[SELECTOR_KERNEL_DS>>3],sizeof(DESCRIPTOR));
		proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;

		proc->regs.cs = (SELECTOR_IN_LDT(0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		proc->regs.ds = (SELECTOR_IN_LDT(1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		proc->regs.es = (SELECTOR_IN_LDT(1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		proc->regs.fs = (SELECTOR_IN_LDT(1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		proc->regs.ss = (SELECTOR_IN_LDT(1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK & SA_TI_MASK) | SA_TIG | RPL_TASK;

		proc->regs.eip = (u32)task->initial_eip;
		proc->regs.esp = (u32)(stack_top);
		proc->regs.eflags = 0x1202;

		proc->ticks = proc->priority = prio[i];

		stack_top -= task->stacksize;
		++proc;
		++task;
		selector_ldt += 1<<3;
	}
	
	ticks = 0;
	k_reenter = 0;

	proc_ready = proc_table;

	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (u8)(TIMER_FREQ / HZ));
	out_byte(TIMER0, (u8)((TIMER_FREQ / HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);

	restart();

	while(1){}
}

void testA()
{
	int i = 0;
	while(1)
	{
		disp_str("A ");
		//disp_int(get_ticks());
		milli_delay(10);
	}
}

void testB()
{
	int i = 0;
	while(1)
	{
		disp_str("B ");
		//disp_int(i++);
		milli_delay(10);
	}
}

void testC()
{
	int i = 0;
	while(1)
	{
		disp_str("C ");
		//disp_int(i++);
		milli_delay(10);
	}
}