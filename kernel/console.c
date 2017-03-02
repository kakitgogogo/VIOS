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

//#define __TTY_DEBUF__

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
#ifdef __TTY_DEBUF__
	int lineno = 0;
	for(; lineno < console->console_size / SCREEN_WIDTH; ++lineno)
	{
		u8 * ch = (u8*)(V_MEM_BASE + (console->original_addr + (lineno + 1) * SCREEN_WIDTH) * 2 - 4);
		*ch++ = lineno / 10 + '0';
		*ch++ = RED_CHAR;
		*ch++ = lineno % 10 + '0';
		*ch++ = RED_CHAR;
	}
#endif
}

PUBLIC void scroll_screen(CONSOLE* console, int direction)
{
	int oldest;
	int newest;
	int screen_top;

	newest = (console->cursor - console->original_addr) / SCREEN_WIDTH * SCREEN_WIDTH;
	oldest = console->is_full ? (newest + SCREEN_WIDTH) % console->console_size : 0;
	screen_top = console->current_start_addr - console->original_addr;

	if(direction == SCR_UP)
	{
		if(!console->is_full && screen_top > 0)
		{
			console->current_start_addr -= SCREEN_WIDTH;
		}
		else if(console->is_full && screen_top != oldest)
		{
			if(console->cursor - console->original_addr >= console->console_size - SCREEN_SIZE)
			{
				if(console->current_start_addr != console->original_addr)
				{
					console->current_start_addr -= SCREEN_WIDTH;
				}
			}
			else if(console->current_start_addr == console->original_addr)
			{
				screen_top = console->console_size - SCREEN_SIZE;
				console->current_start_addr = console->original_addr + screen_top;
			}
			else
			{
				console->current_start_addr -= SCREEN_WIDTH;
			}
		}
	}
	else if(direction == SCR_DN)
	{
		if(!console->is_full && newest >= screen_top + SCREEN_SIZE)
		{
			console->current_start_addr += SCREEN_WIDTH;
		}
		else if(console->is_full && screen_top + SCREEN_SIZE - SCREEN_WIDTH != newest)
		{
			if(screen_top + SCREEN_SIZE == console->console_size)
			{
				console->current_start_addr = console->original_addr;
			}
			else
			{
				console->current_start_addr += SCREEN_WIDTH;
			}
		}
	}
	else
	{
		assert(direction == SCR_UP || direction == SCR_DN);
	}

	flush(console);
}

PRIVATE void clear_screen(int pos, int len)
{
	u8* ch = (u8*)(V_MEM_BASE + pos * 2);
	while(--len >= 0)
	{
		*ch++ = ' ';
		*ch++ = DEFAULT_CHAR_COLOR;
	}
}

PRIVATE void w_copy(unsigned int dst, const unsigned int src, int size)
{
	memcpy((void*)(V_MEM_BASE + (dst << 1)), (void*)(V_MEM_BASE + (src << 1)), size << 1);
}

PUBLIC void out_char(CONSOLE *console, char ch)
{
	u8 *vmem = (u8*)(V_MEM_BASE + console->cursor * 2);

	assert(console->cursor - console->original_addr < console->console_size);

	int cursor_x = (console->cursor - console->original_addr) % SCREEN_WIDTH;
	int cursor_y = (console->cursor - console->original_addr) / SCREEN_WIDTH;

	switch(ch)
	{
	case '\n':
		if(console->cursor < console->original_addr + console->console_size - SCREEN_WIDTH);
		{
			console->cursor = console->original_addr + SCREEN_WIDTH * (cursor_y + 1);
		}
		break;
	case '\t':
		if(console->cursor < console->original_addr + console->console_size - 4)
		{
			console->cursor += 4;
			int i;
			for(i = 0; i < 4; ++i)
			{
				*(vmem + i * 2) = ' ';
				*(vmem + i * 2 + 1) = DEFAULT_CHAR_COLOR;
			}
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
		*vmem++ = ch;
		*vmem++ = DEFAULT_CHAR_COLOR;
		++console->cursor;
		break;
	}

	if(console->cursor - console->original_addr >= console->console_size)
	{
		cursor_x = (console->cursor - console->original_addr) % SCREEN_WIDTH;
		cursor_y = (console->cursor - console->original_addr) / SCREEN_WIDTH;
		int new_origin = console->original_addr + (cursor_y + 1) * SCREEN_WIDTH - SCREEN_SIZE;
		w_copy(console->original_addr, new_origin, SCREEN_SIZE - SCREEN_WIDTH);

		console->current_start_addr = console->original_addr;
		console->cursor = console->original_addr + (SCREEN_SIZE - SCREEN_WIDTH) + cursor_x;
		clear_screen(console->cursor, SCREEN_WIDTH);
		
		console->is_full = TRUE;
	}
	assert(console->cursor - console->original_addr < console->console_size);

	while(console->cursor >= console->current_start_addr + SCREEN_SIZE || console->cursor < console->current_start_addr)
	{
		scroll_screen(console, SCR_DN);

		clear_screen(console->cursor, SCREEN_WIDTH);
	}
	flush(console);
}

PUBLIC void screen_init(TTY* tty)
{
	int tty_id = tty - tty_table;
	tty->console = console_table + tty_id;

	int v_mem_size = V_MEM_SIZE >> 1;

	int size_per_console = v_mem_size / NR_CONSOLES;

	tty->console->original_addr = tty_id * size_per_console;
	tty->console->console_size = size_per_console / SCREEN_WIDTH * SCREEN_WIDTH;
	tty->console->cursor = tty->console->current_start_addr = tty->console->original_addr;
	tty->console->is_full = FALSE;

	if(tty_id == 0)
	{
		tty->console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else
	{
		out_char(tty->console, 'T');
		out_char(tty->console, 'T');
		out_char(tty->console, 'Y');
		out_char(tty->console, tty_id + '1');
		out_char(tty->console, '\n');
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

PUBLIC void clear_console()
{
	CONSOLE *console = &console_table[current_console_id];
	console->cursor = console->original_addr;
	console->current_start_addr = console->original_addr;

	flush(console);

	clear_screen(console->cursor, console->console_size);

	out_char(console, 'T');
	out_char(console, 'T');
	out_char(console, 'Y');
	out_char(console, current_console_id + '1');
	out_char(console, '\n');
}