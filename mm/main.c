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
#include "memory.h"

PRIVATE void mm_init()
{
	boot_params bp;
	get_boot_params(&bp);

	memory_size = bp.mem_size;

	printk("[MM] mem_size: %dMB\n", memory_size >> 20);

	buddy_init();

	test_mm();
}

void test_mm()
{
	int i;
	int* a = (int*)malloc(sizeof(int)*4);
	for(i = 0; i < 4; ++i)
	{
		a[i] = i;
	}
	for(i = 0; i < 4; ++i)
	{
		printk("%d ", a[i]);
	}
	printk("\n");
	printk("addr: 0x%x\n", a);
	printk("order: %d\n", *((u8*)a - 1));
	free(a);
	a = (int*)malloc(sizeof(int)*4);
	for(i = 0; i < 4; ++i)
	{
		printk("%d ", a[i]);
	}
	printk("\n");
	printk("addr: 0x%x\n", a);
	printk("order: %d\n", *((u8*)a - 1));
}

PUBLIC int alloc_mem(int pid, int mem_size)
{
	assert(pid >= (NR_TASKS + NR_NATIVE_PROCS));
	if(mem_size > PROC_SIZE_DEFAULT)
	{
		panic("Unsupported memory request: %d (should be less than %d).",
		      mem_size,
		      PROC_SIZE_DEFAULT);
	}

	int base = PROCS_BASE + 
		(pid - (NR_TASKS + NR_NATIVE_PROCS)) * PROC_SIZE_DEFAULT;

	if(base + mem_size >= memory_size)
	{
		panic("memory allocation failed (pid: %d).", pid);
	}

	return base;
}

PUBLIC int free_mem(int pid)
{
	return 0;
}

PUBLIC void task_mm()
{
	mm_init();

	while(1)
	{
		send_recv(RECEIVE, ANY, &mm_msg);
		int src = mm_msg.source;
		bool reply = TRUE;

		int msgtype = mm_msg.type;

		switch(msgtype)
		{
		case FORK:
			mm_msg.RETVAL = do_fork();
			break;
		case EXIT:
			do_exit(mm_msg.STATUS);
			reply = FALSE;
			break;
		case EXEC:
			mm_msg.RETVAL = do_exec();
			break;
		case WAIT:
			do_wait();
			reply= FALSE;
			break;
		default:
			dump_msg("MM :: unknown msg", &mm_msg);
			assert(0);
			break;
		}
		if(reply)
		{
			mm_msg.type = SYSCALL_RET;
			send_recv(SEND, src, &mm_msg);
		}
	}
}