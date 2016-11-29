#ifndef	VIOS_STDIO_H
#define	VIOS_STDIO_H

#include "type.h"

#define	O_CREAT		1
#define	O_RDWR		2

#define	SEEK_SET		1
#define	SEEK_CUR		2
#define	SEEK_END		3

#define	MAX_PATH		128

/* File status, returned by syscall stat() */
typedef struct stat
{
	int st_dev;
	int st_ino;
	int st_mode;
	int st_rdev;
	int st_size;
}stat_t;

/* RTC time from CMOS */
typedef struct time
{
	u32 year;
	u32 month;
	u32 day;
	u32 hour;
	u32 minute;
	u32 second;
}time_t;

#define	BCD_TO_DEC(x) ((x >> 4) * 10 + (x & 0x0F))

#ifdef	ENABLE_DISK_LOG
#define	SYSLOG	syslog
#endif

/* printf.c */
PUBLIC int	printf(const char* fmt, ...);
PUBLIC int	printk(const char* fmt, ...);

/* vsprintf.c */
PUBLIC int	vsprintf(char* buf, const char* fmt, va_list args);
PUBLIC int	sprintf(char* buf, const char *fmt, ...);

PUBLIC int	open(const char* pathname, int flags);

PUBLIC int	close(int fd);

PUBLIC int	read(int fd, const void* buf, int count);

PUBLIC int	write(int fd, const void* buf, int count);

PUBLIC int	unlink(const char *pathname);

PUBLIC int	getpid();

PUBLIC int	fork();

PUBLIC void	exit(int status);

PUBLIC int	wait(int* status);

PUBLIC int	stat(const char* pathname, stat_t* buf);

PUBLIC int	exec(const char* pathname);
PUBLIC int	execl(const char* pathname, const char *arg, ...);
PUBLIC int	execv(const char* pathname, char* argv[]);

PUBLIC int	syslog(const char* fmt, ...);

#endif