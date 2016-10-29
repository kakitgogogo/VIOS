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

	PROCESS* proc = &proc_table[0];

	proc->ldt_selector = SELECTOR_LDT_FIRST;

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

	proc->regs.eip = (u32)testA;
	proc->regs.esp = (u32)(task_stack + STACK_SIZE_TOTAL);
	proc->regs.eflags = 0x1202;

	proc_ready = &proc_table[0];

	restart();

	while(1){}
}

void testA()
{
	int i = 0;
	while(1)
	{
		disp_str("A");
		disp_int(i++);
		disp_str(" ");
		delay(1);
	}
}