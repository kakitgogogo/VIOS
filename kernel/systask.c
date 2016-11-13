#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
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
		default:
			panic("unknown msg type");
			break;
		}
	}
}