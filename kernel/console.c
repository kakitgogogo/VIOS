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

PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}

PRIVATE void flush(CONSOLE* console)
{
	if(is_current_console(console))
	{
		set_cursor(console->cursor);
		set_video_start_addr(console->current_start_addr);
	}
}

PUBLIC void scroll_screen(CONSOLE* console, int direction)
{
	if(direction == SCR_UP)
	{
		if(console->current_start_addr > console->original_addr)
		{
			console->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if(direction == SCR_DN)
	{
		if(console->current_start_addr + SCREEN_SIZE < console->original_addr + console->v_mem_limit)
		{
			console->current_start_addr += SCREEN_WIDTH;
		}
	}
	else
	{
		return;
	}

	flush(console);
}

PUBLIC void out_char(CONSOLE *console, char ch)
{
	u8 *vmem = (u8*)(V_MEM_BASE + console->cursor * 2);

	switch(ch)
	{
	case '\n':
		if(console->cursor < console->original_addr + console->v_mem_limit - SCREEN_WIDTH);
		{
			console->cursor = console->original_addr + SCREEN_WIDTH * ((console->cursor - console->original_addr) / SCREEN_WIDTH + 1);
		}
		break;
	case '\b':
		if(console->cursor > console->original_addr)
		{
			--console->cursor;
			*(vmem - 2) = ' ';
			*(vmem - 1) = DEFAULT_CHAR_COLOR;
		}
		break;
	default:
		if(console->cursor < console->original_addr + console->v_mem_limit - 1)
		{
			*vmem++ = ch;
			*vmem++ = DEFAULT_CHAR_COLOR;
			++console->cursor;
		}
		break;
	}
	while(console->cursor >= console->current_start_addr + SCREEN_SIZE)
	{
		scroll_screen(console, SCR_DN);
	}
	flush(console);
}

PUBLIC void screen_init(TTY* tty)
{
	int tty_id = tty - tty_table;
	tty->console = console_table + tty_id;

	int v_mem_size = V_MEM_SIZE >> 1;

	int con_v_mem_size = v_mem_size / NR_CONSOLES;

	tty->console->original_addr = tty_id * con_v_mem_size;
	tty->console->v_mem_limit = con_v_mem_size;
	tty->console->current_start_addr = tty->console->original_addr;

	tty->console->cursor = tty->console->original_addr;

	if(tty_id == 0)
	{
		tty->console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else
	{
		out_char(tty->console, tty_id + '1');
		out_char(tty->console, '#');
	}

	set_cursor(tty->console->cursor);
}

PUBLIC int is_current_console(CONSOLE *console)
{
	return (console == &console_table[current_console_id]);
}

PUBLIC void select_console(int console_id)
{
	if((console_id < 0) || (console_id >= NR_CONSOLES))
	{
		return;
	}

	current_console_id = console_id;

	set_cursor(console_table[console_id].cursor);
	set_video_start_addr(console_table[console_id].current_start_addr);
}