#ifndef	VIOS_CONSOLE_H
#define	VIOS_CONSOLE_H

typedef struct s_console
{
	unsigned int current_start_addr;
	unsigned int original_addr;
	unsigned int console_size;
	unsigned int cursor;
	bool is_full;
}CONSOLE;

#define	SCR_UP			1
#define	SCR_DN			-1

#define	SCREEN_SIZE		(80 *25)
#define SCREEN_WIDTH	80

#define	DEFAULT_CHAR_COLOR 	0x0F
#define	GRAY_CHAR			(COLOR(BLACK, BLACK) | BRIGHT)
#define	RED_CHAR			0xF4

#endif