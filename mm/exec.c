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

#include "elf.h"

PUBLIC int do_exec()
{
	int name_len = mm_msg.NAME_LEN;
	int src = mm_msg.source;
	assert(name_len < MAX_PATH);

	char pathname[MAX_PATH];
	memcpy((void*)va2la(TASK_MM, pathname), (void*)va2la(src, mm_msg.PATHNAME), name_len);
	pathname[name_len] = 0;

	stat_t s;
	int ret = stat(pathname, &s);
	if(ret != 0)
	{
		printk("[MM] do_exec() : stat() returns error(pathname: %s).", pathname);
		return -1;
	}

	int fd = open(pathname, O_RDWR);
	if(fd == -1)
	{
		return -1;
	}
	assert(s.st_size < MMBUF_SIZE);
	read(fd, mmbuf, s.st_size);
	close(fd);

	Elf32_Ehdr* elf_hdr = (Elf32_Ehdr*)(mmbuf);
	int i;
	for(i = 0; i < elf_hdr->e_phnum; ++i)
	{
		Elf32_Phdr* proc_hdr = (Elf32_Phdr*)(mmbuf + elf_hdr->e_phoff + (i * elf_hdr->e_phentsize));
		if(proc_hdr->p_type == PT_LOAD)
		{
			assert(proc_hdr->p_vaddr + proc_hdr->p_memsz < PROC_SIZE_DEFAULT);
			memcpy(
				(void*)va2la(src, (void*)proc_hdr->p_vaddr),
				(void*)va2la(TASK_MM, mmbuf + proc_hdr->p_offset),
				proc_hdr->p_filesz);
		}
	}

	int origin_stack_len = mm_msg.BUF_LEN;
	char stackcopy[PROC_ORIGIN_STACK];
	memcpy((void*)va2la(TASK_MM, stackcopy), (void*)va2la(src, mm_msg.BUF), origin_stack_len);

	u8* origin_stack = (u8*)(PROC_SIZE_DEFAULT - PROC_ORIGIN_STACK);

	int delta = (int)origin_stack - (int)mm_msg.BUF;

	int argc = 0;
	if(origin_stack_len)
	{
		char** q = (char**)stackcopy;
		for(; *q != 0; ++q, ++argc)
		{
			*q += delta;
		}
	}

	memcpy((void*)va2la(src, origin_stack), (void*)va2la(TASK_MM, stackcopy), origin_stack_len);

	proc_table[src].regs.ecx = argc;
	proc_table[src].regs.eax = (u32)origin_stack;

	proc_table[src].regs.eip = elf_hdr->e_entry;
	proc_table[src].regs.esp = PROC_SIZE_DEFAULT - PROC_ORIGIN_STACK;

	strcpy(proc_table[src].pname, pathname);

	return 0;
}