#ifndef	VIOS_TTY_H
#define	VIOS_TTY_H

#define TTY_IN_BYTES 256

struct s_console;

typedef struct s_tty
{
	u32 buf[TTY_IN_BYTES];
	u32* head;
	u32* rear;
	int count;

	struct s_console* console;
}TTY;

#endif