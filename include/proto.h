#ifndef	VIOS_PROTO_H
#define	VIOS_PROTO_H

/*	 klib.asm 	*/
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char *info);
PUBLIC void	disp_color_str(char *info, int color);

/*	 protect.c 	*/
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 selector);

/*	 i8259.c 		*/
PUBLIC void	init_8259A();

/*	 klib.c 		*/
PUBLIC void delay();

/*	 kernel.asm 	*/
PUBLIC void restart();

/*	 main.c 		*/
PUBLIC void testA();
PUBLIC void testB();

#endif