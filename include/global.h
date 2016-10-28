#ifndef	VIOS_GLOBAL_H
#define	VIOS_GLOBAL_H

#ifndef	GLOBAL_VARIABLE_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int 			disp_pos;
EXTERN	u8 			gdt_ptr[6];
EXTERN	DESCRIPTOR 	gdt[GDT_SIZE];
EXTERN	u8 			idt_ptr[6];
EXTERN	GATE 		idt[IDT_SIZE];

#endif
