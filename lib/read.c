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

PUBLIC int read(int fd, const void* buf, int count)
{
	MESSAGE msg;

	msg.type		= READ;
	msg.FD		= fd;
	msg.BUF		= (void*)buf;
	msg.CNT		= count;

	send_recv(BOTH, TASK_FS, &msg);

	return msg.CNT;
}