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

#define	TTY_BEGIN	(tty_table)
#define	TTY_END		(tty_table + NR_CONSOLES)	

PUBLIC void tty_init(TTY* tty)
{
	tty->count = 0;
	tty->head = tty->tail = tty->buf;
	screen_init(tty);
}

PRIVATE void put_key(TTY* tty, u32 key)
{
	if(tty->count < TTY_IN_BYTES)
	{
		*(tty->head) = key;
		++tty->head;
		if(tty->head == tty->buf + TTY_IN_BYTES)
		{
			tty->head = tty->buf;
		}
		++tty->count;
	}
}

PRIVATE void tty_dev_read(TTY* tty)
{
	if(is_current_console(tty->console))
	{
		keyboard_read(tty);
	}
}

PRIVATE void tty_dev_write(TTY* tty)
{
	while(tty->count)
	{
		char ch = *(tty->tail);
		++tty->tail;
		if(tty->tail == tty->buf + TTY_IN_BYTES)
		{
			tty->tail = tty->buf;
		}
		--tty->count;

		if(tty->tty_left_cnt)
		{
			if(ch >= ' ' && ch <= '~')
			{
				out_char(tty->console, ch);
				void* ptr = tty->tty_req_buf + tty->tty_trans_cnt;
				memcpy(ptr, (void*)va2la(TASK_TTY, &ch), 1);
				++tty->tty_trans_cnt;
				--tty->tty_left_cnt;
			}
			else if(ch == '\b' && tty->tty_trans_cnt)
			{
				out_char(tty->console, ch);
				--tty->tty_trans_cnt;
				++tty->tty_left_cnt;
			}

			if(ch == '\n' || tty->tty_left_cnt == 0)
			{
				out_char(tty->console, '\n');
				MESSAGE msg;
				msg.type = RESUME_PROC;
				msg.PROC_ID = tty->tty_proc;
				msg.CNT = tty->tty_trans_cnt;
				send_recv(SEND, tty->tty_caller, &msg);
				tty->tty_left_cnt = 0;
			}
		}
	}
}

PRIVATE void tty_do_read(TTY* tty, MESSAGE* msg)
{
	tty->tty_caller = msg->source;
	tty->tty_proc = msg->PROC_ID;
	tty->tty_req_buf = va2la(tty->tty_proc, msg->BUF);
	tty->tty_left_cnt = msg->CNT;
	tty->tty_trans_cnt = 0;

	msg->type = SUSPEND_PROC;
	msg->CNT = tty->tty_left_cnt;
	send_recv(SEND, tty->tty_caller, msg);
}

PRIVATE void tty_do_write(TTY* tty, MESSAGE* msg)
{
	char buf[TTY_OUT_BUF_LEN];
	char* p = (char*)va2la(msg->PROC_ID, msg->BUF);
	int i = msg->CNT, j;

	while(i)
	{
		int bytes = min(TTY_OUT_BUF_LEN, i);
		memcpy(va2la(TASK_TTY, buf), (void*)p, bytes);
		for(j = 0; j < bytes; ++j)
		{
			out_char(tty->console, buf[j]);
		}
		i -= bytes;
		p += bytes;
	}

	msg->type = SYSCALL_RET;
	send_recv(SEND, msg->source, msg);
}

PUBLIC void task_tty()
{
	TTY *tty;
	MESSAGE msg;

	keyboard_init();

	for(tty = TTY_BEGIN; tty < TTY_END; ++tty)
	{
		tty_init(tty);
	}

	select_console(0);

	while(1)
	{
		for(tty = TTY_BEGIN; tty < TTY_END; ++tty)
		{
			do
			{
				tty_dev_read(tty);
				tty_dev_write(tty);
			}
			while(tty->count);
		}

		send_recv(RECEIVE, ANY, &msg);

		int src = msg.source;
		assert(src != TASK_TTY);

		tty = &tty_table[msg.DEVICE];

		switch(msg.type)
		{
		case DEV_OPEN:
			reset_msg(&msg);
			msg.type = SYSCALL_RET;
			send_recv(SEND, src, &msg);
			break;
		case DEV_READ:
			tty_do_read(tty, &msg);
			break;
		case DEV_WRITE:
			tty_do_write(tty, &msg);
			break;
		case DEV_CLEAR:
			clear_console();
			reset_msg(&msg);
			msg.type = SYSCALL_RET;
			send_recv(SEND, src, &msg);
			break;
		case HARD_INT:
			key_pressed = FALSE;
			continue;
		default:
			dump_msg("TTY : unknown msg", &msg);
			break;
		}
	}
}

PUBLIC void in_process(TTY* tty, u32 key)
{
	if(!(key & FLAG_EXT))
	{
		put_key(tty, key);
	}
	else
	{
		int raw_code = key & MASK_RAW;
		switch(raw_code)
		{
		case ENTER:
			put_key(tty, '\n');
			break;
		case BACKSPACE:
			put_key(tty, '\b');
			break;
		case TAB:
			put_key(tty, '\t');
			break;
		case UP:
			if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R))
			{
				scroll_screen(tty->console, SCR_UP);
			}
			break;
		case DOWN:
			if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R))
			{
				scroll_screen(tty->console, SCR_DN);
			}
			break;
		case F1:
		case F2:
		case F3:
		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			if((key & FLAG_ALT_L) || (key & FLAG_ALT_R))
			{
				select_console(raw_code - F1);
			}
			else
			{
				if(raw_code == F12)
				{
					disable_int();
					dump_proc(proc_table + 4);
					for(;;);
				}
			}
			break;
		default:
			break;
		}
	}
}


PUBLIC int sys_printx(int _unused1, int _unused2, char* s, PROCESS* proc)
{
	const char *p;
	char ch;

	char reenter_err[] = "? k_reenter is incorrect for unknown reason";
	reenter_err[0] = MAG_CH_PANIC;

	if(k_reenter == 0)
		p = va2la(proc2pid(proc), s);
	else if(k_reenter > 0)
		p = s;
	else
		p = reenter_err;

	if((*p == MAG_CH_PANIC) || (*p == MAG_CH_ASSERT && proc_ready < &proc_table[NR_TASKS]))
	{
		disable_int();
		char *v = (char*)V_MEM_BASE;
		const char *q = p + 1;

		while(v < (char*)(V_MEM_BASE + V_MEM_SIZE))
		{
			*v++ = *q++;
			*v++ = RED_CHAR;
			if(!*q)
			{
				while(((int)v - V_MEM_BASE) % (SCREEN_WIDTH * 8))
				{
					*v++ = ' ';
					*v++ = RED_CHAR;
				}
				q = p + 1;
			}
		}
		__asm__ __volatile__("hlt");
	}

	while((ch = *p++) != 0)
	{
		if(ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT)
			continue;

		out_char(tty_table[proc->tty_id].console, ch);
	}

	return 0;
}

PUBLIC void dump_tty_buf(int idx)
{
	TTY * tty = &tty_table[idx];

	printk("------------------------------------------\n");

	printk("head: %d\n", tty->head - tty->buf);
	printk("tail: %d\n", tty->tail - tty->buf);
	printk("count: %d\n", tty->count);

	int pid = tty->tty_caller;
	printk("tty_caller: %s (%d)\n", proc_table[pid].pname, pid);
	pid = tty->tty_proc;
	printk("tty_caller: %s (%d)\n", proc_table[pid].pname, pid);

	printk("tty_req_buf: %d\n", (int)tty->tty_req_buf);
	printk("tty_left_cnt: %d\n", tty->tty_left_cnt);
	printk("tty_trans_cnt: %d\n", tty->tty_trans_cnt);

	printk("------------------------------------------\n");
}