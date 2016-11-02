#define GLOBAL_VARIABLE_HERE

#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

PUBLIC	PROCESS		proc_table[NR_TASKS];

PUBLIC	char		task_stack[STACK_SIZE_TOTAL];

PUBLIC	TASK		task_table[NR_TASKS] = {
	{task_tty, STACK_SIZE_TTY, "tty"},
	{testA, STACK_SIZE_TESTA, "testA"},
	{testB, STACK_SIZE_TESTB, "testB"},
	{testC, STACK_SIZE_TESTC, "testC"}
};

PUBLIC	irq_handler	irq_table[NR_IRQ];

PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {
	sys_get_ticks
};

PUBLIC	TTY			tty_table[NR_CONSOLES];
PUBLIC	CONSOLE		console_table[NR_CONSOLES];