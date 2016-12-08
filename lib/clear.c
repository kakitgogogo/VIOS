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

PUBLIC int clear()
{
	MESSAGE msg;

	msg.type	= DEV_CLEAR;

	send_recv(BOTH, TASK_TTY, &msg);

	return msg.RETVAL;
}