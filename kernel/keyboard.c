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
#include "keymap.h"

PRIVATE	KB_INPUT kb_in;

PRIVATE	bool code_with_E0;
PRIVATE	bool shift_l;
PRIVATE	bool shift_r;	
PRIVATE	bool alt_l;	
PRIVATE	bool alt_r;	
PRIVATE	bool ctrl_l;	
PRIVATE	bool ctrl_r;	
PRIVATE	bool caps_lock;	
PRIVATE	bool num_lock;	
PRIVATE	bool scroll_lock;	
PRIVATE	int column;

PRIVATE void kb_wait()
{
	u8 kb_stat;

	do
	{
		kb_stat = in_byte(KB_CMD);
	}
	while(kb_stat & 0x02);
}

PRIVATE void kb_ack()
{
	u8 kb_read;

	do
	{
		kb_read = in_byte(KB_DATA);
	}
	while(kb_read != KB_ACK);
}

PRIVATE void set_leds()
{
	u8 leds = (caps_lock << 2) | (num_lock << 1) | scroll_lock;

	kb_wait();
	out_byte(KB_DATA, LED_CODE);
	kb_ack();

	kb_wait();
	out_byte(KB_DATA, leds);
	kb_ack();
}

PUBLIC void keyboard_init()
{
	kb_in.head = kb_in.rear = kb_in.buf;
	kb_in.count = 0;

	shift_l = shift_r = FALSE;
	alt_l = alt_r = FALSE;
	ctrl_l = ctrl_r = FALSE;

	caps_lock = FALSE;
	num_lock = TRUE;
	scroll_lock = FALSE;

	set_leds();

	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
	enable_irq(KEYBOARD_IRQ);
}

PUBLIC void keyboard_handler()
{
	u8 scan_code = in_byte(KB_DATA);
	
	if(kb_in.count < KB_IN_BYTES)
	{
		*(kb_in.head) = scan_code;
		++kb_in.head;
		if(kb_in.head == kb_in.buf + KB_IN_BYTES)
		{
			kb_in.head = kb_in.buf;
		}
		++kb_in.count;
	}
}

PRIVATE u8 get_byte_from_kbuf()
{
	u8 scan_code;

	while(kb_in.count <= 0);

	disable_int();
	scan_code = *(kb_in.rear);
	++kb_in.rear;
	if(kb_in.rear == kb_in.buf + KB_IN_BYTES)
	{
		kb_in.rear = kb_in.buf;
	}
	--kb_in.count;
	enable_int();

	return scan_code;
}

PUBLIC void keyboard_read(TTY* tty)
{

	u8 scan_code;
	char output[2];
	bool make = FALSE;

	u32 key = 0;
	u32* keyrow;

	if(kb_in.count > 0)
	{
		code_with_E0 = FALSE;

		scan_code = get_byte_from_kbuf();

		if(scan_code == 0xE1)
		{
			int i;
			u8 pausebrk_scode[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
			bool is_pausebreak = TRUE;
			for(i = 1; i < 6; ++i)
			{
				if(get_byte_from_kbuf() != pausebrk_scode[i])
				{
					is_pausebreak = FALSE;
					break;
				}
			}
			if(is_pausebreak)
			{
				key = PAUSEBREAK;
			}
		}
		else if(scan_code == 0xE0)
		{
			scan_code = get_byte_from_kbuf();

			if(scan_code == 0x2A)
			{
				if(get_byte_from_kbuf() == 0xE0)
				{
					if(get_byte_from_kbuf() == 0x37)
					{
						key = PRINTSCREEN;
						make = TRUE;
					}
				}
			}

			if(scan_code == 0xB7)
			{
				if(get_byte_from_kbuf() == 0xE0)
				{
					if(get_byte_from_kbuf() == 0xAA)
					{
						key = PRINTSCREEN;
						make = FALSE;
					}
				}
			}
			if(key == 0)
			{
				code_with_E0 = TRUE;
			}
		}
		if((key != PAUSEBREAK) && (key != PRINTSCREEN))
		{
			make = (scan_code & FLAG_BREAK ? FALSE : TRUE);

			keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];

			column = 0;

			int caps = shift_l || shift_r;
			if(caps_lock)
			{
				if((keyrow[0] >= 'a') && (keyrow[0] <= 'z'))
				{
					caps = !caps;
				}
			}
			if(caps)
			{
				column = 1;
			}
			if(code_with_E0)
			{
				column = 2;
			}

			key = keyrow[column];

			switch(key)
			{
			case SHIFT_L:
				shift_l = make;
				break;
			case SHIFT_R:
				shift_r = make;
				break;
			case CTRL_L:
				ctrl_l = make;
				break;
			case CTRL_R:
				ctrl_r = make;
				break;
			case ALT_L:
				alt_l = make;
				break;
			case ALT_R:
				alt_r = make;
				break;
			case CAPS_LOCK:
				if(make)
				{
					caps_lock = !caps_lock;
					set_leds();
				}
				break;
			case NUM_LOCK:
				if(make)
				{
					num_lock = !num_lock;
					set_leds();
				}
				break;
			case SCROLL_LOCK:
				if(make)
				{
					scroll_lock = !scroll_lock;
					set_leds();
				}
				break;
			default:
				break;
			}

			if(make)
			{
				bool pad = FALSE;
				if((key >= PAD_SLASH) && (key <= PAD_9))
				{
					pad = TRUE;
					switch(key)
					{
					case PAD_SLASH:
						key = '/';
						break;
					case PAD_STAR:
						key = '*';
						break;
					case PAD_MINUS:
						key = '-';
						break;
					case PAD_PLUS:
						key = '+';
						break;
					case PAD_ENTER:
						key = ENTER;
						break;
					default:
						if(num_lock && (key >= PAD_0) && (key <= PAD_9))
						{
							key = key - PAD_0 + '0';
						}
						else if(num_lock && (key == PAD_DOT))
						{
							key = '.';
						}
						else
						{
							switch(key) 
							{
							case PAD_HOME:
								key = HOME;
								break;
							case PAD_END:
								key = END;
								break;
							case PAD_PAGEUP:
								key = PAGEUP;
								break;
							case PAD_PAGEDOWN:
								key = PAGEDOWN;
								break;
							case PAD_INS:
								key = INSERT;
								break;
							case PAD_UP:
								key = UP;
								break;
							case PAD_DOWN:
								key = DOWN;
								break;
							case PAD_LEFT:
								key = LEFT;
								break;
							case PAD_RIGHT:
								key = RIGHT;
								break;
							case PAD_DOT:
								key = DELETE;
								break;
							default:
								break;
							}
						}
						break;
					}
				}
				key |= shift_l ? FLAG_SHIFT_L : 0;
				key |= shift_r ? FLAG_SHIFT_R : 0;
				key |= ctrl_l ? FLAG_CTRL_L : 0;
				key |= ctrl_r ? FLAG_CTRL_R : 0;
				key |= alt_l ? FLAG_ALT_L : 0;
				key |= alt_r ? FLAG_ALT_R : 0;

				in_process(tty, key);
			}
		}
	}
}