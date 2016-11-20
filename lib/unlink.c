#include "config.h"
#include "const.h"
#include "type.h"
#include "protect.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
#include "hd.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include "stdio.h"

PUBLIC int unlink(const char* pathname)
{
	MESSAGE msg;
	msg.type = UNLINK;

	msg.PATHNAME = (void*)(pathname);
	msg.NAME_LEN = strlen(pathname);

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}