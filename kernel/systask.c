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
#include "keyboard.h"

PUBLIC void task_sys()
{
	MESSAGE msg;
	while(1)
	{
		send_recv(RECEIVE, ANY, &msg);
		int src = msg.source;

		switch(msg.type)
		{
		case GET_TICKS:
			msg.RETVAL = ticks;
			send_recv(SEND, src, &msg);
			break;
		case GET_PID:
			msg.type = SYSCALL_RET;
			msg.PID = src;
			send_recv(SEND, src, &msg);
			break;
		default:
			panic("Unknown msg type");
			break;
		}
	}
}