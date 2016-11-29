#ifndef	VIOS_PROTO_H
#define	VIOS_PROTO_H

/* lib/kliba.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char *info);
PUBLIC void	disp_color_str(char *info, int color);
PUBLIC void	enable_irq(int irq);
PUBLIC void	disable_irq(int irq);
PUBLIC void	enable_int();
PUBLIC void	disable_int();
PUBLIC void	port_read(u16 port, void* buf, int n);
PUBLIC void	port_write(u16 port, void* buf, int n);
PUBLIC void	glitter(int row, int col);

/* lib/klib.c */
PUBLIC void	delay(int time);
PUBLIC void	disp_int(int input);
PUBLIC char*	itoa(char* str, int num);

/* lib/string.asm */
PUBLIC void*	memcpy(void* dst, void* src, int size);
PUBLIC void*	memset(void* des, char ch, int size);
PUBLIC char*	strcpy(char* dst, char* src);
PUBLIC int	strlen(char* str);

/* lib/misc.c */
PUBLIC void	spin(char *func_name);
PUBLIC void	assertion_failure(char *exp, char *file, char *base_file, int line);
PUBLIC int	memcmp(const void* s1, const void *s2, int n);;
PUBLIC int	strcmp(const char* s1, const char* s2);
PUBLIC char*	strcat(char *s1, const char *s2);

/* lib/open.c */
PUBLIC int	open(const char* pathname, int flags);

/* lib/close.c */
PUBLIC int	close(int fd);

/* kernel/protect.c */
PUBLIC void	protect_init();
PUBLIC u32	seg2phys(u16 selector);

/* kernel/i8259.c */
PUBLIC void	init_8259A();
PUBLIC void	put_irq_handler(int irq, irq_handler handler);
PUBLIC void	spurious_irq(int irq);

/* kernel/kernel.asm */
PUBLIC void	restart();
PUBLIC void	sys_call();

/* kernel/main.c */
PUBLIC void	init();
PUBLIC void	testA();
PUBLIC void	testB();
PUBLIC void	testC();

/* kernel/clock.c */
PUBLIC void	clock_init();
PUBLIC void	clock_handler(int irq);
PUBLIC void	milli_delay(int milli_sec);

/* kernel/keyboard.c */
PUBLIC void	keyboard_init();
PUBLIC void	keyboard_handler();
PUBLIC void	keyboard_read(TTY* tty);

/* kernel/tty.c */
PUBLIC void	task_tty();
PUBLIC void	in_process(TTY* tty, u32 key);
PUBLIC void	tty_write(TTY* tty, char* buf, int len);

/* kernel/console.c */
PUBLIC void	out_char(CONSOLE* console, char ch);
PUBLIC void	scroll_screen(CONSOLE* console, int direction);

/* kernel/printf.c */
PUBLIC int	printf(const char *fmt, ...);

/* kernel/vsprintf.c */
PUBLIC int	vsprintf(char *buf, const char *fmt, va_list args);

/* kernel/proc.c */
PUBLIC int	sys_get_ticks();
PUBLIC int	sys_write(int _unused1, char* buf, int len, PROCESS* proc);
PUBLIC int	sys_printx(int _unused1, int _unused2, char* s, PROCESS* proc);
PUBLIC int	ldt_seg_linear(PROCESS *proc, int idx);
PUBLIC void*	va2la(int pid, void* va);
PUBLIC void	schedule();

/* kernel/message.c */
PUBLIC int	sys_sendrec(int function, int src_des, MESSAGE* msg, PROCESS* proc);
PUBLIC int	send_recv(int function, int src_des, MESSAGE* msg);
PUBLIC void	inform_int(int task_id);
PUBLIC void	dump_proc(PROCESS *proc);
PUBLIC void	dump_msg(const char * title, MESSAGE* msg);

/* kernel/syscall.asm */
PUBLIC int	get_ticks();
PUBLIC void	write0(char* buf, int len);

/* kernel/systask.c */
PUBLIC void	task_sys();

/* kernel/panic.c */
PUBLIC void	panic(const char *fmt, ...);

/* kernel/hd.c */
PUBLIC void	task_hd();
PUBLIC void	hd_handler(int irq);

/* fs/main.c */
PUBLIC int	rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_id, void* buf);
PUBLIC void	task_fs();

/* fs/open.c */
PUBLIC int	do_open();
PUBLIC int	do_close();

/* fs/misc.c */
PUBLIC int	strip_path(char* filename, const char* pathname, inode** inode_ptr_ptr);
PUBLIC int	search_file(char* path);

/* mm/main.c */
PUBLIC void	task_mm();

#endif