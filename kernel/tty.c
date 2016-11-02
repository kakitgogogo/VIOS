#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
#include "keyboard.h"

#define	TTY_BEGIN	(tty_table)
#define	TTY_END		(tty_table + NR_CONSOLES)	

PUBLIC void tty_init(TTY* tty)
{
	tty->count = 0;
	tty->head = tty->rear = tty->buf;
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

PRIVATE void tty_do_read(TTY* tty)
{
	if(is_current_console(tty->console))
	{
		keyboard_read(tty);
	}
}

PRIVATE void tty_do_write(TTY* tty)
{
	if(tty->count)
	{
		char ch = *(tty->rear);
		++tty->rear;
		if(tty->rear == tty->buf + TTY_IN_BYTES)
		{
			tty->rear = tty->buf;
		}
		--tty->count;

		out_char(tty->console, ch);
	}
}

PUBLIC void task_tty()
{
	TTY *tty;

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
			tty_do_read(tty);
			tty_do_write(tty);
		}
	}
}

PUBLIC void in_process(TTY* tty, u32 key)
{
	char output[2] = {'\0', '\0'};

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
			break;
		default:
			break;
		}
	}
}