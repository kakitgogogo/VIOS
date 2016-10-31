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

extern	PROCESS		proc_table[NR_TASKS];
extern	char		task_stack[STACK_SIZE_TOTAL];

extern	TASK		task_table[NR_TASKS];
extern	irq_handler	irq_table[NR_IRQ];

extern	system_call	sys_call_table[NR_SYS_CALL];

#endif
