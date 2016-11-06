#ifndef	VIOS_PROC_H
#define	VIOS_PROC_H

typedef struct s_stackframe
{
	u32	gs;
	u32	fs;
	u32	es;
	u32	ds;

	u32	edi;
	u32	esi;
	u32	ebp;
	u32	kernel_esp;

	u32	ebx;
	u32	edx;
	u32	ecx;
	u32	eax;

	u32	retaddr;

	u32	eip;
	u32	cs;
	u32	eflags;
	
	u32	esp;
	u32	ss;
}STACK_FRAME;

struct msg1 
{
	int m1i1;
	int m1i2;
	int m1i3;
	int m1i4;
};
struct msg2 
{
	void* m2p1;
	void* m2p2;
	void* m2p3;
	void* m2p4;
};
struct msg3 
{
	int	m3i1;
	int	m3i2;
	int	m3i3;
	int	m3i4;
	u64	m3l1;
	u64	m3l2;
	void* m3p1;
	void* m3p2;
};
typedef struct 
{
	int source;
	int type;
	union 
	{
		struct msg1 m1;
		struct msg2 m2;
		struct msg3 m3;
	} u;
} MESSAGE;

typedef struct s_proc
{
	STACK_FRAME	regs;
	u16			ldt_selector;
	DESCRIPTOR	ldts[LDT_SIZE];

	int			ticks;
	int			priority;

	u32			pid;
	char		pname[16];

	int			pflags;

	MESSAGE*	pmsg;
	int			recvfrom;
	int			sendto;

	bool		has_int_msg;

	struct s_proc*	sending;
	struct s_proc*	next_sending;

	int			tty_id;
} PROCESS;

typedef struct s_task
{
	task_f		initial_eip;
	int			stacksize;
	char		name[32];
} TASK;

#define	proc2pid(x)	(x - proc_table)

#define	NR_TASKS		2
#define	NR_PROCS		3
#define	FIRST_PROC	proc_table[0]
#define	LAST_PROC	proc_table[NR_TASKS + NR_PROCS - 1]

#define	STACK_SIZE_TTY		0x8000
#define	STACK_SIZE_SYS		0x8000
#define	STACK_SIZE_TESTA		0x8000
#define	STACK_SIZE_TESTB		0x8000
#define	STACK_SIZE_TESTC		0x8000

#define	STACK_SIZE_TOTAL		(STACK_SIZE_TTY + \
							STACK_SIZE_SYS + \
							STACK_SIZE_TESTA + \
							STACK_SIZE_TESTB + \
							STACK_SIZE_TESTC)

#endif