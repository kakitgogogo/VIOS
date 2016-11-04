#ifndef	VIOS_PROTO_H
#define	VIOS_PROTO_H

/*	 klib.asm 	*/
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char *info);
PUBLIC void	disp_color_str(char *info, int color);

/*	 protect.c 	*/
PUBLIC void	protect_init();
PUBLIC u32	seg2phys(u16 selector);

/*	 i8259.c 		*/
PUBLIC void	init_8259A();
PUBLIC void	put_irq_handler(int irq, irq_handler handler);
PUBLIC void	spurious_irq(int irq);

/*	 klib.c 		*/
PUBLIC void	delay(int time);

/*	 kernel.asm 	*/
PUBLIC void	restart();
PUBLIC void	sys_call();

/*	 main.c 		*/
PUBLIC void	testA();
PUBLIC void	testB();
PUBLIC void	testC();

/*	 clock.c 		*/
PUBLIC void	clock_init();
PUBLIC void	clock_handler(int irq);
PUBLIC void	milli_delay(int milli_sec);

/*	 keyboard.c 	*/
PUBLIC void	keyboard_init();
PUBLIC void	keyboard_handler();

/*	tty.c		*/
PUBLIC void	task_tty();
PUBLIC void	in_process(TTY* tty, u32 key);
PUBLIC void	tty_write(TTY* tty, char* buf, int len);

/*	 console.c 	*/
PUBLIC void	out_char(CONSOLE* console, char ch);
PUBLIC void	scroll_screen(CONSOLE* console, int direction);

/*	 printf.c 	*/
PUBLIC int	printf(const char *fmt, ...);

/*	 vsprintf.c 	*/
PUBLIC int	vsprintf(char *buf, const char *fmt, va_list args);

/*	 proc.c 		*/
PUBLIC int	sys_get_ticks();
PUBLIC int	sys_write(char* buf, int len, PROCESS* proc);
PUBLIC void	schedule();

/*	 syscall.asm 	*/
PUBLIC int	get_ticks();
PUBLIC void	write(char* buf, int len);

#endif