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
#include "stdio.h"

PRIVATE int read_reg_from_cmos(char reg_addr)
{
	out_byte(CLK_ELE, reg_addr);
	return in_byte(CLK_IO);
}

PRIVATE u32 get_rtc_time(time_t *t)
{
	t->year	= read_reg_from_cmos(YEAR);
	t->month	= read_reg_from_cmos(MONTH);
	t->day	= read_reg_from_cmos(DAY);
	t->hour	= read_reg_from_cmos(HOUR);
	t->minute = read_reg_from_cmos(MINUTE);
	t->second = read_reg_from_cmos(SECOND);

	if ((read_reg_from_cmos(CLK_STATUS) & 0x04) == 0) 
	{
		t->year	= BCD_TO_DEC(t->year);
		t->month	= BCD_TO_DEC(t->month);
		t->day	= BCD_TO_DEC(t->day);
		t->hour	= BCD_TO_DEC(t->hour);
		t->minute = BCD_TO_DEC(t->minute);
		t->second = BCD_TO_DEC(t->second);
	}

	t->year += 2000;

	return 0;
}

PUBLIC void task_sys()
{
	MESSAGE msg;
	time_t t;

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
		case GET_RTC_TIME:
			msg.type = SYSCALL_RET;
			get_rtc_time(&t);
			memcpy(va2la(src, msg.BUF), va2la(TASK_SYS, &t), sizeof(t));
			send_recv(SEND, src, &msg);
		default:
			panic("Unknown msg type");
			break;
		}
	}
}