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
#include "sched.h"

PUBLIC int kernel_main()
{
	disp_str("Welcome to VIOS\n");

	PROCESS* proc = proc_table;
	TASK* task = task_table;
	char* stack_top = task_stack + STACK_SIZE_TOTAL;

	int i, j;
	u8 priv, rpl;
	u32 eflags, prio;

	int tty_ids[NR_TASKS + NR_NATIVE_PROCS] = {0, 0, 0, 0, 0, 0, 0};
	int task_prios[NR_TASKS] = {110, 110, 110, 110, 110};
	/* 
	task prios:
		1.TTY 110
		2.SYS 110
		3.HD  110
		4.FS  110
		5.MM  110
	*/

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
			prio = task_prios[i];
		}
		else
		{
			task = user_proc_table + (i - NR_TASKS);
			priv = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202;
			prio = 130;
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

		proc->prio = prio;
		proc->static_prio = prio;
		proc->time_slice = BASE_TIMESLICE(proc);

		list_init(&proc->run_list);
		proc->sleep_avg = 0;
		proc->timestamp = 0;
		proc->activated = 0;
		proc->interactive_credit = 0;
		proc->array = NULL;

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
	sched_init();

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
	int count = 0;

	while(1)
	{
		assert(read(fd, buf, SECTOR_SIZE) == SECTOR_SIZE);

		if(buf[0] == 0)
		{
			if(count == 0)
			{
				printf("[INIT] need not untar.\n");
			}
			break;
		}
		++count;

		posix_tar_header* pth = (posix_tar_header*)buf;

		char* p = pth->size;
		int file_size = 0;
		while(*p)
		{
			file_size = (file_size * 8) + (*p++ - '0'); /* octal */
		}

		int bytes_left = file_size;
		int fdout = open(pth->name, O_CREAT | O_RDWR | O_TRUNC);
		if(fdout == -1)
		{
			printf("[Extract failed]\n");
			close(fd);
			return;
		}
		printf("[Extract %s (%d bytes)]\n", pth->name, file_size);
		while(bytes_left)
		{
			int bytes = min(chunk, bytes_left);
			read(fd, buf, ((bytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
			assert(write(fdout, buf, bytes) == bytes);
			bytes_left -= bytes;
		}
		close(fdout);
	}

	if(count)
	{
		lseek(fd, 0, SEEK_SET);
		buf[0] = 0;
		assert(write(fd, buf, 1) == 1);
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

#if 1
	int child_pid = getpid(), i;
	printk("[SHELL] Child id: %d\n", child_pid);
	printk("[SHELL] Child->prio: %d\n", proc_table[child_pid].prio);
	printk("[SHELL] Child->pflags: %x\n", proc_table[child_pid].pflags);
	printk("[SHELL] stdin: inode: %x\n", (&proc_table[child_pid])->files[fd_stdin]->fd_inode);
	printk("[SHELL] stdout: inode: %x\n", (&proc_table[child_pid])->files[fd_stdout]->fd_inode);	
#endif

	char rdbuf[128];

	while(1)
	{
		write(fd_stdout, "$", 2);
		int r = read(fd_stdin, rdbuf, 70);
		if(r == 0) continue;
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
				write(fd_stdout, "Invalid Command: ", 17);
				write(fd_stdout, rdbuf, r);
				write(fd_stdout, "\n", 1);
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

	close(fd_stdout);
	close(fd_stdin);
}

void init()
{
	int fd_stdin = open("/dev_tty1", O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open("/dev_tty1", O_RDWR);
	assert(fd_stdout == 1);

	printf("[INIT] Init is running ...\n");
	untar("/cmd.tar");

	char* tty_list[] = {
		"/dev_tty2",
		"/dev_tty3"
	};

	int i;
	for(i = 0; i < 2; ++i)
	{
		int pid = fork();

		if (pid != 0) 
		{
			printf("[INIT] Parent is running, child pid: %d\n", pid);
		}
		else 
		{
			printf("[CHILD] Child is running, pid: %d\n", getpid());
			close(fd_stdin);
			close(fd_stdout);

			shell(tty_list[i]);
			assert(0);
		}
	}

	while(1)
	{
		int s;
		int child = wait(&s);
		printf("[INIT] Child (%d) exited with status: %d.\n", child, s);
	}

	assert(0);
}

void test()
{
	while(1)
	{
		milli_delay(10000);
	}
}
