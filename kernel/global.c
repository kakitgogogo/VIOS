#define GLOBAL_VARIABLE_HERE

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

PUBLIC	PROCESS	proc_table[NR_TASKS + NR_PROCS];

PUBLIC	TASK	task_table[NR_TASKS] = {
	{task_tty, STACK_SIZE_TTY, "tty"},
	{task_sys, STACK_SIZE_SYS, "sys"},
	{task_hd, STACK_SIZE_HD, "hd"},
	{task_fs, STACK_SIZE_FS, "fs"},
	{task_mm, STACK_SIZE_MM, "mm"}
};

PUBLIC	TASK	user_proc_table[NR_NATIVE_PROCS] = {
	{init, STACK_SIZE_INIT, "INIT"},
	{test, STACK_SIZE_TESTA, "test"}
};

PUBLIC	char	task_stack[STACK_SIZE_TOTAL];

PUBLIC	irq_handler	irq_table[NR_IRQ];

PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {
	sys_get_ticks,
	sys_write,
	sys_printx,
	sys_sendrec
};

PUBLIC	TTY		tty_table[NR_CONSOLES];
PUBLIC	CONSOLE	console_table[NR_CONSOLES];

PUBLIC	DRIVER		dd_map[]={
	{INVALID_DRIVER},
	{INVALID_DRIVER},
	{INVALID_DRIVER},
	{TASK_HD},
	{TASK_TTY},
	{INVALID_DRIVER}
};

PUBLIC	u8*			fsbuf		= (u8*)0x600000;	/* 6M */
PUBLIC	const int	FSBUF_SIZE	= 0x100000;		/* 1M */

PUBLIC	u8*			mmbuf		= (u8*)0x700000;	/* 7M */
PUBLIC	const int	MMBUF_SIZE	= 0x100000;		/* 1M */

PUBLIC	bool 		isPreemptDisabled = FALSE;