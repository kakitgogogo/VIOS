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

PUBLIC int stat(const char* pathname, stat_t* buf)
{
	MESSAGE msg;

	msg.type = STAT;

	msg.PATHNAME = (void*)pathname;
	msg.BUF = (void*)buf;
	msg.NAME_LEN = strlen(pathname);

	send_recv(BOTH, TASK_FS, &msg);
	assert(msg.type == SYSCALL_RET);

	return msg.RETVAL;
}