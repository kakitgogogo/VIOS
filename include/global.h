#ifndef	VIOS_GLOBAL_H
#define	VIOS_GLOBAL_H

#ifdef	GLOBAL_VARIABLE_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int			ticks;

EXTERN	int 			disp_pos;
EXTERN	u8 			gdt_ptr[6];
EXTERN	DESCRIPTOR 	gdt[GDT_SIZE];
EXTERN	u8 			idt_ptr[6];
EXTERN	GATE 		idt[IDT_SIZE];

EXTERN	u32			k_reenter;

EXTERN	TSS			tss;
EXTERN	PROCESS*	proc_ready;

EXTERN	int			current_console_id;

extern	PROCESS		proc_table[NR_TASKS + NR_PROCS];
extern	char		task_stack[STACK_SIZE_TOTAL];

extern	TASK		task_table[NR_TASKS];
extern	TASK		user_proc_table[NR_PROCS];

extern	irq_handler	irq_table[NR_IRQ];

extern	system_call	sys_call_table[NR_SYS_CALL];

extern	TTY			tty_table[NR_CONSOLES];
extern	CONSOLE		console_table[NR_CONSOLES];

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

#endif
