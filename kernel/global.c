#define GLOBAL_VARIABLE_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

PUBLIC	PROCESS		proc_table[NR_TASKS];

PUBLIC	char		task_stack[STACK_SIZE_TOTAL];

PUBLIC	TASK		task_table[NR_TASKS] = {
	{testA, STACK_SIZE_TESTA, "testA"},
	{testB, STACK_SIZE_TESTB, "testB"},
	{testC, STACK_SIZE_TESTC, "testC"}
};

PUBLIC	irq_handler	irq_table[NR_IRQ];

PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {
	sys_get_ticks
};