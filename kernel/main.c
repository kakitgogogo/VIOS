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

PUBLIC int kernel_main()
{
	disp_str("Welcome to VIOS\n");

	PROCESS* proc = proc_table;
	TASK* task = task_table;
	char* stack_top = task_stack + STACK_SIZE_TOTAL;

	int i, j;
	u8 priv, rpl;
	u32 eflags, prio;

	int tty_ids[NR_TASKS + NR_NATIVE_PROCS] = {0, 0, 0, 0, 0, 0, 1, 2, 0};

	for(i = 0; i < NR_TASKS + NR_PROCS; ++i, ++proc, ++task)
	{
		if(i >= NR_TASKS + NR_NATIVE_PROCS)
		{
			proc->pflags = FREE_SLOT;
			continue;
		}
		if(i < NR_TASKS)
		{
			task = task_table + i;
			priv = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202;
			prio = 15;
		}
		else
		{
			task = user_proc_table + (i - NR_TASKS);
			priv = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202;
			prio = 5;
		}
		strcpy(proc->pname, task->name);

		proc->pid = i;

		proc->parent = NO_TASK;

		if(strcmp(task->name, "INIT") != 0)
		{
			proc->ldts[INDEX_LDT_C] = gdt[SELECTOR_KERNEL_CS >> 3];
			proc->ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DS >> 3];

			proc->ldts[INDEX_LDT_C].attr1 = DA_C | priv << 5;
			proc->ldts[INDEX_LDT_RW].attr1 = DA_DRW | priv << 5;
		}
		else
		{
			unsigned int kbase, klimit;
			assert(get_kernel_map(&kbase, &klimit) == 0);

			init_descriptor(&proc->ldts[INDEX_LDT_C],
				0,
				(kbase + klimit) >> LIMIT_4K_SHIFT,
				DA_32 | DA_LIMIT_4K | DA_C | priv << 5);

			init_descriptor(&proc->ldts[INDEX_LDT_RW],
				0,
				(kbase + klimit) >> LIMIT_4K_SHIFT,
				DA_32 | DA_LIMIT_4K | DA_DRW | priv << 5);
		}

		proc->regs.cs = (SELECTOR_IN_LDT(0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.ds = (SELECTOR_IN_LDT(1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.es = (SELECTOR_IN_LDT(1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.fs = (SELECTOR_IN_LDT(1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.ss = (SELECTOR_IN_LDT(1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK & SA_TI_MASK) | SA_TIG | rpl;

		proc->regs.eip = (u32)task->initial_eip;
		proc->regs.esp = (u32)(stack_top);
		proc->regs.eflags = eflags;

		proc->ticks = proc->priority = prio;

		proc->pflags = 0;
		proc->pmsg = 0;
		proc->recvfrom = NO_TASK;
		proc->sendto = NO_TASK;
		proc->has_int_msg = FALSE;
		proc->sending = 0;
		proc->next_sending = 0;

		proc->tty_id = tty_ids[i];

		for(j = 0; j < NR_FILES; ++j)
		{
			proc->files[j] = 0;
		}

		stack_top -= task->stacksize;	
	}
	
	ticks = 0;
	k_reenter = 0;

	proc_ready = proc_table;

	clock_init();
	keyboard_init();

	restart();

	while(1);
}

#if 1
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}
#endif

typedef struct posix_tar_header
{						/* byte offset */
	char name[100];		/* 000 */
	char mode[8];			/* 100 */
	char uid[8];			/* 108 */
	char gid[8];			/* 116 */
	char size[12];		/* 124 */
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100];	/* 157 */
	char magic[6];		/* 257 */
	char version[2];		/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];		/* 329 */
	char devminor[8];		/* 337 */
	char prefix[155];		/* 345 */
						/* 500 */
}posix_tar_header;

void untar(const char* filename)
{
	printf("[Extract '%s']\n", filename);
	int fd = open(filename, O_RDWR);
	assert(fd != -1);

	char buf[SECTOR_SIZE * 16];
	int chunk = sizeof(buf);

	while(1)
	{
		read(fd, buf, SECTOR_SIZE);
		if(buf[0] == 0)
		{
			break;
		}

		posix_tar_header* pth = (posix_tar_header*)buf;

		char* p = pth->size;
		int file_size = 0;
		while(*p)
		{
			file_size = (file_size * 8) + (*p++ - '0'); /* octal */
		}

		int bytes_left = file_size;
		int fdout = open(pth->name, O_CREAT | O_RDWR);
		if(fdout == -1)
		{
			printf("[Extract failed]\n");
			return;
		}
		printf("[Extract %s (%d bytes)]\n", pth->name, file_size);
		while(bytes_left)
		{
			int bytes = min(chunk, bytes_left);
			read(fd, buf, ((bytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
			write(fdout, buf, bytes);
			bytes_left -= bytes;
		}
		close(fdout);
	}
	close(fd);

	printf("[Extract done]\n");
}

void shell(const char* tty_name)
{
	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char rdbuf[128];

	while(1)
	{
		write(fd_stdout, "$", 2);
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;

		int argc = 0;
		char* argv[PROC_ORIGIN_STACK];
		char* p = rdbuf;
		char* s;
		bool start_word = FALSE;
		char ch;
		do
		{
			ch = *p;
			if(*p != ' ' && *p != 0 && !start_word)
			{
				s = p;
				start_word = TRUE;
			}
			if((*p == ' ' || *p == 0) && start_word)
			{
				start_word = FALSE;
				argv[argc++] = s;
				*p = 0;
			}
			++p;
		}
		while(ch);
		argv[argc] = 0;

		int fd = open(argv[0], O_RDWR);
		if(fd == -1)
		{
			if(rdbuf[0])
			{
				write(fd_stdout, "Invalid command: ", 18);
				write(fd_stdout, rdbuf, r);
			}
		}
		else
		{
			close(fd);
			int pid = fork();
			if(pid != 0)
			{
				int s;
				wait(&s);
			}
			else
			{
				execv(argv[0], argv);
			}
		}
	}

	clode(fd_stdout);
	close(fd_stdin);
}

void init()
{
	int fd_stdin = open("dev_tty1", O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open("dev_tty1", O_RDWR);
	assert(fd_stdout == 1);

	printf("[INIT] Init is running ...\n");
	untar("/cmd.tar");

	int pid = fork();
#if 0
	printk("init: stdin: inode: %x\n", (&proc_table[5])->files[fd_stdin]->fd_inode);
	printk("init: stdout: inode: %x\n", (&proc_table[5])->files[fd_stdout]->fd_inode);
#endif
	if (pid != 0) 
	{
		printf("[INIT] Parent is running, child pid: %d\n", pid);
		
		int s;
		int child = wait(&s);

		printf("[INIT] Child (%d) exited with status: %d.\n", child, s);
	}
	else 
	{
		printf("[CHILD] Child is running, pid: %d\n", getpid());
		
		exit(123);
	}

	while(1)
	{
		int s;
		int child = wait(&s);
		printf("[INIT] Child (%d) exited with status: %d.\n", child, s);
	}
}

void testA()
{
	while(1);
}

void testB()
{
	while(1);
}

void testC()
{
	while(1);
}

/*
void testA()
{
	while(1)
	{
		printk("<Ticks: %d>", get_ticks());
		milli_delay(1000);
	}
}

void testB()
{
	
	char tty_name[] = "/dev_tty3";

	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1); 

	char rdbuf[128];

	while(1)
	{
		printf("$ ");
		int num = read(fd_stdin, rdbuf, 70);
		rdbuf[num] = 0;

		printf("echo: %s\n", rdbuf);
	}
	assert(0);
}

void testC()
{
	int fd;
	int i, n;
	const char filename[] = "kakit";
	const char bufw[] = "I am kakit";
	const int rd_bytes = 10;
	char bufr[rd_bytes];

	assert(rd_bytes <= strlen(bufw));

	fd = open(filename, O_CREAT | O_RDWR);
	assert(fd != -1);
	printk("[TESTC] File %s created(fd: %d)\n", filename, fd);

	n = write(fd, bufw, strlen(bufw));
	assert(n == strlen(bufw));

	close(fd);

	fd = open(filename, O_RDWR);
	assert(fd != -1);
	printk("[TESTC] File opened(fd: %d)\n", fd);

	n = read(fd, bufr, rd_bytes);
	assert(n == rd_bytes);
	bufr[n] = 0;
	printk("[TESTC] %d bytes read: %s\n", n, bufr);

	close(fd);

	char* filenames[] = {"/a", "/b", "/c"};

	for(i = 0; i < 3; ++i)
	{
		fd = open(filenames[i], O_CREAT | O_RDWR);
		assert(fd != -1);
		printk("[TESTC] File %s created(fd: %d)\n", filenames[i], fd);
		close(fd);
	}

	char* rfilenames[] = {"/c", "/a", "/b"};
	for(i = 0; i < 3; ++i)
	{
		if(unlink(rfilenames[i]) == 0)
		{
			printk("[TESTC] File %s removed\n", rfilenames[i]);
		}
		else
		{
			printk("[TESTC] Failed to remove file %s\n", rfilenames[i]);
		}
	}

	spin("Test C");
}
*/