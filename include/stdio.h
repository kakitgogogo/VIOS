#ifndef	VIOS_STDIO_H
#define	VIOS_STDIO_H

#define	O_CREAT		1
#define	O_RDWR		2

#define	SEEK_SET		1
#define	SEEK_CUR		2
#define	SEEK_END		3

#define	MAX_PATH		128

#ifdef	ENABLE_DISK_LOG
#define	SYSLOG	syslog
#endif

PUBLIC int	open(const char* pathname, int flags);

PUBLIC int	close(int fd);

PUBLIC int	read(int fd, const void* buf, int count);

PUBLIC int	write(int fd, const void* buf, int count);

PUBLIC int	unlink(const char *pathname);

PUBLIC int	getpid();

PUBLIC int	syslog(const char* fmt, ...);

#endif