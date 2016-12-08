#include "const.h"
#include "config.h"
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

/* GNU */
#include "elf.h"

PUBLIC void get_boot_params(boot_params* bp)
{
	int* p = (int*)BOOT_PARAM_ADDR;
	assert(p[BI_MAGIC] == BOOT_PARAM_MAGIC);

	bp->mem_size = p[BI_MEM_SIZE];
	bp->kernel_bin = (unsigned*)(p[BI_KERNEL_BIN]);

	assert(memcmp(bp->kernel_bin, ELFMAG, SELFMAG) == 0);
}

PUBLIC int get_kernel_map(unsigned int* base, unsigned int* limit)
{
	boot_params bp;
	get_boot_params(&bp);

	Elf32_Ehdr* elf_header = (Elf32_Ehdr*)(bp.kernel_bin);

	if(memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0)
	{
		return -1;
	}

	*base = ~0;
	unsigned int top = 0;
	int i;
	for (i = 0; i < elf_header->e_shnum; ++i) 
	{
		Elf32_Shdr* section_header = (Elf32_Shdr*)
			(bp.kernel_bin + 
			elf_header->e_shoff + 
			i * elf_header->e_shentsize);

		if (section_header->sh_flags & SHF_ALLOC) 
		{
			int bottom = section_header->sh_addr;
			int tmp = section_header->sh_addr +
				section_header->sh_size;

			if (*base > bottom)
				*base = bottom;
			if (top < tmp)
				top = tmp;
		}
	}
	assert(*base < top);
	*limit = top - *base - 1;

	return 0;
}

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
			for(j = 0; j < 10000; ++j);
		}
	}
}

void dump(u8* address, int n)
{
	int i;
	printk("Dump 0x%x:\n", address);
	for(i = 0; i< n;)
	{
		printk("%02x  ", *(address++));
		++i;
		if(i % 16 == 0) printk("\n");
		else if(i % 8 == 0) printk("  ");
	}
	printk("\n");
}