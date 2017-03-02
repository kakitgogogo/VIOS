#ifndef	VIOS_GLOBAL_H
#define	VIOS_GLOBAL_H

#ifdef	GLOBAL_VARIABLE_HERE
#undef	EXTERN
#define	EXTERN
#endif

#include "bitmap.h"
#include "const.h"
#include "type.h"
#include "protect.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
#include "proc.h"
#include "sched.h"

EXTERN	int			ticks;

EXTERN	int 		disp_pos;

EXTERN	u8 			gdt_ptr[6];
EXTERN	DESCRIPTOR 	gdt[GDT_SIZE];
EXTERN	u8 			idt_ptr[6];
EXTERN	GATE 		idt[IDT_SIZE];

EXTERN	u32			k_reenter;

EXTERN	int			current_console_id;

EXTERN	bool		key_pressed;

EXTERN	TSS			tss;
EXTERN	PROCESS*	proc_ready;

extern	PROCESS		proc_table[];
extern	char		task_stack[];

extern	TASK		task_table[];
extern	TASK		user_proc_table[];

extern	irq_handler	irq_table[];

extern	system_call	sys_call_table[];

extern	TTY			tty_table[];
extern	CONSOLE		console_table[];

/* File System */
EXTERN	file_desc	fd_table[NR_FILE_DESC];
EXTERN	inode		inode_table[NR_INODE];
EXTERN	super_block	sb_table[NR_SUPER_BLOCK];
extern	u8*			fsbuf;
extern	const int	FSBUF_SIZE;
EXTERN	MESSAGE		fs_msg;
EXTERN	PROCESS*	fs_caller;
EXTERN	inode*		root_inode;
extern	DRIVER		dd_map[];

/* Memory Management */
EXTERN	MESSAGE		mm_msg;
extern	u8 *		mmbuf;
extern	const int	MMBUF_SIZE;
EXTERN	int			memory_size;

EXTERN	u8			prio_array_1[MAX_PRIO / 8 + 1];
EXTERN	u8			prio_array_2[MAX_PRIO / 8 + 1];
EXTERN	runqueue_t	runqueue;

extern	bool 		isPreemptDisabled;

#endif
