#ifndef	VIOS_GLOBAL_H
#define	VIOS_GLOBAL_H

#ifdef	GLOBAL_VARIABLE_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int 			disp_pos;
EXTERN	u8 			gdt_ptr[6];
EXTERN	DESCRIPTOR 	gdt[GDT_SIZE];
EXTERN	u8 			idt_ptr[6];
EXTERN	GATE 		idt[IDT_SIZE];

EXTERN	TSS			tss;
EXTERN	PROCESS*	proc_ready;

EXTERN	PROCESS		proc_table[NR_TASKS];
EXTERN	char		task_stack[STACK_SIZE_TOTAL];

#endif
