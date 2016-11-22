#ifndef	VIOS_TTY_H
#define	VIOS_TTY_H

#define TTY_IN_BYTES		256
#define	TTY_OUT_BUF_LEN	2

struct s_console;

typedef struct s_tty
{
	u32 buf[TTY_IN_BYTES];
	u32* head;
	u32* tail;

	int count;

	int tty_caller;
	int tty_proc;
	void* tty_req_buf;
	int tty_left_cnt;
	int tty_trans_cnt;

	struct s_console* console;
}TTY;

#endif