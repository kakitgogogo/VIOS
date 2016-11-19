#include "const.h"
#include "type.h"
#include "protect.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include "stdio.h"

PUBLIC int close(int fd)
{
	MESSAGE msg;

	msg.type		= CLOSE;
	msg.FD		= fd;

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}