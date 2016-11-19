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

PUBLIC char* itoa(char *str, int num)
{
	char *p = str;
	char ch;
	int i, flag = 0;

	*p++ = '0';
	*p++ = 'x';

	if(num == 0)
	{
		*p++ = '0';
	}
	else
	{
		for(i = 28; i >= 0; i -= 4)
		{
			ch = (num >> i) & 0xF;
			if(flag || (ch > 0))
			{
				flag = 1;
				ch += '0';
				if(ch > '9')
				{
					ch +=7;
				}
				*p++ = ch;
			}
		}
	}
	*p = 0;
	return str;
}

PUBLIC void disp_int(int input)
{
	char output[16];
	itoa(output, input);
	disp_str(output);
}

PUBLIC void disp_color_int(int input, int color)
{
	char output[16];
	itoa(output, input);
	disp_color_str(output, color);
}

PUBLIC void delay(int time)
{
	int i, j, k;
	for(k = 0; k < time; ++k)
	{
		for(i = 0; i < 10; ++i)
		{
			for(j = 0; j < 10000; ++j)
			{
				
			}
		}
	}
}

void dump(u8* address, int n)
{
	int i;
	printf("Dump 0x%x:\n", address);
	for(i = 0; i< n;)
	{
		printf("%02x  ", *(address++));
		++i;
		if(i % 16 == 0) printf("\n");
		else if(i % 8 == 0) printf("  ");
	}
	printf("\n");
}