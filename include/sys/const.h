#ifndef	VIOS_CONST_H
#define	VIOS_CONST_H

/* Macro assert */
#define	ASSERT
#ifdef	ASSERT
void assertion_failure(char *exp, char *file, char *base_file, int line);
#define	assert(exp)\
	do\
	{\
		if(exp);\
		else\
		{\
			assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__);\
		}\
	}\
	while(0)
#else
#define	assert(exp)
#endif

/* Macro about Break Point */
#define	BREAK_POINT	\
	do\
	{\
		printk("\n===Break Point %s:%d===\n", __FILE__, __LINE__);\
		spin("break point");\
		__asm__ __volatile__("hlt");\
	}\
	while(0)

/* Macro max and min */
#define	max(a, b)	((a) > (b) ? (a) : (b))
#define	min(a, b)	((a) < (b) ? (a) : (b))

/* Color */
#define	BLACK	0x0
#define	WHITE	0x7
#define	RED		0x4
#define	GREEN	0x2
#define	BLUE	0x1
#define	FLASH	0x80
#define	BRIGHT	0x08
#define	COLOR(x, y) ((x << 4) | y)

/* GDT Size and LDT Size */
#define	GDT_SIZE		128
#define	IDT_SIZE		256

/* PRIVILEGE */
#define	PRIVILEGE_KRNL 	0
#define	PRIVILEGE_TASK 	1
#define	PRIVILEGE_USER 	3

/* Request Privilege Level */
#define	RPL_KRNL			SA_RPL0
#define	RPL_TASK			SA_RPL1
#define	RPL_USER			SA_RPL3

/* 8259A */
#define	INT_M_CTL		0x20
#define	INT_M_CTLMASK	0x21
#define	INT_S_CTL		0xA0
#define	INT_S_CTLMASK	0xA1

/* 8253/8254 */
#define	TIMER0			0x40
#define	TIMER_MODE		0x43
#define	RATE_GENERATOR	0x34
#define	TIMER_FREQ		1193182L
#define	HZ				1000

#define	MAX_TICKS		0x7FFFABCD
#define	MAX_MSEC		(MAX_TICKS * (1000 / HZ))

/* 8042 */
#define	KB_DATA			0x60
#define	KB_CMD			0x64
#define	LED_CODE			0xED
#define	KB_ACK			0xFA

/* Hardware Interrupt Request */
#define	NR_IRQ			16
#define	CLOCK_IRQ		0
#define	KEYBOARD_IRQ		1
#define	CASCADE_IRQ		2
#define	ETHER_IRQ		3
#define	SECONDARY_IRQ	3
#define	RS232_IRQ		4
#define	XI_WINI_IRQ		5
#define	FLOPPY_IRQ		6
#define	PRINTER_IRQ		7
#define	AT_WINI_IRQ		14

/* VGA */
#define	CRTC_ADDR_REG	0x3D4
#define	CRTC_DATA_REG	0x3D5
#define	START_ADDR_H		0xC
#define	START_ADDR_L		0xD
#define	CURSOR_H			0xE
#define	CURSOR_L			0xF
#define	V_MEM_BASE		0xB8000
#define	V_MEM_SIZE		0x8000

/* CMOS */
#define CLK_ELE			0x70
#define CLK_IO			0x71

#define  YEAR			9
#define  MONTH			8
#define  DAY				7
#define  HOUR			4
#define  MINUTE			2
#define  SECOND			0
#define  CLK_STATUS		0x0B
#define  CLK_HEALTH		0x0E

/* TTY */
#define	NR_CONSOLES		3

/* System Call */
#define	NR_SYS_CALL		5

/* String */
#define	STR_DEFAULT_LEN	1024

/* Process */
#define	SENDING			0x02
#define	RECEIVING		0x04
#define	WAITING			0x08
#define	HANGING			0x10
#define	FREE_SLOT		0x20

/* Task */
#define	INVALID_DRIVER	-20
#define	INTERRUPT		-10
#define	TASK_TTY		0
#define	TASK_SYS		1
#define	TASK_HD			2
#define	TASK_FS			3
#define	TASK_MM			4
#define	INIT			5
#define	ANY				(NR_TASKS + NR_PROCS + 10)
#define	NO_TASK			(NR_TASKS + NR_PROCS + 20)

/* IPC */
#define	SEND			1
#define	RECEIVE			2
#define	BOTH			3	//BOTH = SEND | RECEIVE

/* Magic Chars */
#define	MAG_CH_PANIC		'\002'
#define	MAG_CH_ASSERT	'\003'

/* Message Type */
enum msgtype
{
	HARD_INT = 1,

	GET_TICKS,
	GET_PID,
	GET_RTC_TIME,

	OPEN,
	CLOSE,
	READ,
	WRITE,
	LSEEK,
	STAT,
	UNLINK,

	SUSPEND_PROC,
	RESUME_PROC,

	EXEC,
	WAIT,

	FORK,
	EXIT,

	SYSCALL_RET,

	DEV_OPEN = 1001,
	DEV_CLOSE,
	DEV_READ,
	DEV_WRITE,
	DEV_IOCTL,
	DEV_CLEAR,

	DISK_LOG
};

/* Macro for Message */
#define	FD			u.m3.m3i1
#define	PATHNAME	u.m3.m3p1 
#define	FLAGS		u.m3.m3i1 
#define	NAME_LEN	u.m3.m3i2 
#define	BUF_LEN		u.m3.m3i3
#define	CNT			u.m3.m3i2
#define	REQUEST		u.m3.m3i2
#define	PROC_ID		u.m3.m3i3
#define	DEVICE		u.m3.m3i4
#define	POSITION	u.m3.m3l1
#define	BUF			u.m3.m3p2
#define	OFFSET		u.m3.m3i2 
#define	WHENCE		u.m3.m3i3 

#define	PID			u.m3.m3i2 
#define	STATUS		u.m3.m3i1 
#define	RETVAL		u.m3.m3i1
#define	STATUS		u.m3.m3i1


#define	DIOCTL_GET_GEO		1

/* Hard Driver */
#define	SECTOR_SIZE			512
#define	SECTOR_BITS			(SECTOR_SIZE * 8)
#define	SECTOR_SIZE_SHIFT	9

/* Major Device numbers */
#define	NO_DEV				0
#define	DEV_FLOPPY			1
#define	DEV_CDROM			2
#define	DEV_HD				3
#define	DEV_CHAR_TTY			4
#define	DEV_SCSI				5

/* About Device Major and Minor Number  */
#define	MAJOR_SHIFT			8
#define	MAKE_DEV(a, b)			((a << MAJOR_SHIFT) | b)
#define	MAJOR(x)				((x >> MAJOR_SHIFT) & 0xFF)
#define	MINOR(x)				(x & 0xFF)

/* Device Number of Hard disk */
#define	MINOR_HD1A			0x10
#define	MINOR_HD2A			0x20
#define	MINOR_HD2B			0x21
#define	MINOR_HD3A			0x30
#define	MINOR_HD4A			0x40

#define	ROOT_DEV				MAKE_DEV(DEV_HD, MINOR_BOOT)

#define	INVALID_INODE		0
#define	ROOT_INODE			1

#define	MAX_DRIVERS			2
#define	NR_PART_PER_DRIVER	4
#define	NR_SUB_PER_PART		16
#define	NR_SUB_PER_DRIVER	(NR_SUB_PER_PART * NR_PART_PER_DRIVER)
#define	NR_PRIM_PER_DRIVER	(NR_PART_PER_DRIVER + 1)

/*  */
#define	MAX_PRIM				(MAX_DRIVERS * NR_PRIM_PER_DRIVER - 1)

#define	MAX_SUBPARTITONS		(NR_SUB_PER_DRIVER * MAX_DRIVERS)

#define	P_PRIMARY			0
#define	P_EXTENDED			1

#define	VIOS_PART			0x99
#define	NO_PART				0x00
#define	EXT_PART				0x05

#define	NR_FILES				64
#define	NR_FILE_DESC			64
#define	NR_INODE				64
#define	NR_SUPER_BLOCK		8

/* INODE */
#define	I_TYPE_MASK			0170000
#define	I_REGULAR       		0100000
#define	I_BLOCK_SPECIAL 		0060000
#define	I_DIRECTORY    		0040000
#define	I_CHAR_SPECIAL 		0020000
#define	I_NAMED_PIPE			0010000

#define	is_special(m) \
	((((m) & I_TYPE_MASK) == I_BLOCK_SPECIAL) || (((m) & I_TYPE_MASK) == I_CHAR_SPECIAL))

#define	NR_DEFAULT_FILE_SECTS	2048

#endif