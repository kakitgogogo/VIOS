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

PRIVATE void cleanup(PROCESS* proc)
{
	MESSAGE msg;
	msg.type = SYSCALL_RET;
	msg.PID = proc2pid(proc);
	msg.STATUS = proc->exit_status;
	send_recv(SEND, proc->parent, &msg);

	proc->pflags = FREE_SLOT;

	deactivate_task(&proc, this_rq());

	printk("[MM] do_exit :: %s (%d) has been cleaned up.\n", proc->pname, proc2pid(proc));
}

PUBLIC int do_fork()
{
	PROCESS* proc = proc_table;
	int i;
	for(i = 0; i < NR_TASKS + NR_PROCS; ++i, ++proc)
	{
		if(proc->pflags == FREE_SLOT)
		{
			break;
		}
	}

	int child_pid = i;

	assert(proc == &proc_table[child_pid]);
	assert(child_pid >= NR_TASKS + NR_NATIVE_PROCS);
	if(i == NR_TASKS + NR_PROCS)
	{
		return -1;
	}
	assert(i < NR_TASKS + NR_PROCS);

	int pid = mm_msg.source;
	u16 child_ldt_selector = proc->ldt_selector;
	*proc = proc_table[pid];
	proc->ldt_selector = child_ldt_selector;
	proc->parent = pid;
	sprintf(proc->pname, "%s_%d", proc_table[pid].pname, child_pid);

	DESCRIPTOR* desc;

	/* Text Segment */
	desc = &proc_table[pid].ldts[INDEX_LDT_C];

	int T_base  = reassembly(desc->base_high, 24,
					desc->base_mid,  16,
					desc->base_low);
	int T_limit = reassembly(0, 0,
					(desc->limit_high_attr2 & 0xF), 16,
					desc->limit_low);
	int T_size  = ((T_limit + 1) *
					((desc->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ?
					4096 : 1)); 

	/* Data & Stack Segment */
	desc = &proc_table[pid].ldts[INDEX_LDT_RW];
	
	int DS_base  = reassembly(desc->base_high, 24,
					desc->base_mid,  16,
					desc->base_low);
	int DS_limit = reassembly(0, 0,
					(desc->limit_high_attr2 & 0xF), 16,
					desc->limit_low);
	int DS_size  = ((DS_limit + 1) *
					((desc->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ?
					4096 : 1)); 

	/* child's LDT */
	int child_base = alloc_mem(child_pid, T_size);
	printk("[MM] do_fork :: 0x%x <- 0x%x (0x%x bytes).\n", child_base, T_base, T_size);

	memcpy((void*)child_base, (void*)T_base, T_size);

	printk("%s->time_slice: %dms, prio: %d\n", proc->pname, proc->time_slice, proc->prio);

	init_descriptor(&proc->ldts[INDEX_LDT_C], 
		child_base,
		(PROC_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		DA_LIMIT_4K | DA_32 | DA_C | PRIVILEGE_USER << 5);

	init_descriptor(&proc->ldts[INDEX_LDT_RW], 
		child_base,
		(PROC_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		DA_LIMIT_4K | DA_32 | DA_DRW | PRIVILEGE_USER << 5);

	/* to FS */
	MESSAGE msg2fs;
	msg2fs.type = FORK;
	msg2fs.PID = child_pid;
	send_recv(BOTH, TASK_FS, &msg2fs);

#if 0
	printk("init: stdin: fd_cnt: %d\n", (&proc_table[5])->files[0]->fd_cnt);
	printk("init: stdout: fd_cnt: %d\n", (&proc_table[5])->files[1]->fd_cnt);
	printk("init: stdin_inode: %x\n", (&proc_table[5])->files[0]->fd_inode);
	printk("init: stdout_inode: %x\n", (&proc_table[5])->files[1]->fd_inode);
	printk("init: stdin: %x\n", (&proc_table[5])->files[0]);
	printk("init: stdout: %x\n", (&proc_table[5])->files[1]);
	printk("child: stdin: %x\n", (&proc_table[child_pid])->files[0]);
	printk("child: stdout: %x\n", (&proc_table[child_pid])->files[1]);
#endif
	/* child PID will be returned to the parent proc */
	mm_msg.PID = child_pid;

	activate_task(proc, this_rq());
	printk("%s->time_slice: %dms, prio: %d\n", proc->pname, proc->time_slice, proc->prio);

	/* to child */
	MESSAGE msg2child;
	msg2child.type = SYSCALL_RET;
	msg2child.RETVAL = 0;
	msg2child.PID = 0;
	send_recv(SEND, child_pid, &msg2child);

	return 0;
}

PUBLIC void do_exit(int status)
{
	int i;
	int pid = mm_msg.source;
	int parent_pid = proc_table[pid].parent;
	PROCESS* proc = &proc_table[pid];

	MESSAGE msg2fs;
	msg2fs.type = EXIT;
	msg2fs.PID = pid;
	send_recv(BOTH, TASK_FS, &msg2fs);

	free_mem(pid);

	proc->exit_status = status;

	if(proc_table[parent_pid].pflags & WAITING)
	{
		printk("[MM] do_exit :: %s (%d) is WAITING, %s (%d) will be cleaned up.\n",
			proc_table[parent_pid].pname, parent_pid,
			proc->pname, pid);

		printk("[MM] do_exit :: proc_table[parent_pid].pflags: 0x%x.\n",
			proc_table[parent_pid].pflags);

		proc_table[parent_pid].pflags &= ~WAITING;
		cleanup(&proc_table[pid]);
	}
	else
	{
		printk("[MM] do_exit :: %s (%d) is not WAITING, %s (%d) will be HANGING.\n",
			proc_table[parent_pid].pname, parent_pid,
			proc->pname, pid);

		proc_table[pid].pflags |= HANGING;
	}

	for(i = 0; i < NR_TASKS + NR_PROCS; ++i)
	{
		if(proc_table[i].parent == pid)
		{
			proc_table[i].parent = INIT;
			printk("[MM] do_exit :: %s (%d) exit, so %s (%d) is INIT's child now.\n",
				proc->pname, pid, proc_table[i].pname, i);
			printk("[MM] do_exit :: proc_table[INIT].pflags: 0x%x.\n",
				proc_table[INIT].pflags);

			if((proc_table[INIT].pflags & WAITING) && (proc_table[i].pflags & HANGING))
			{
				proc_table[INIT].pflags &= ~WAITING;
				cleanup(&proc_table[i]);
			}
		}
	}
}

PUBLIC void do_wait()
{
	printk("[MM] do_wait\n");
	int pid = mm_msg.source;

	int i;
	int children = 0;
	PROCESS* proc = proc_table;

	for(i = 0; i < NR_TASKS + NR_PROCS; ++i, ++proc)
	{
		if(proc->parent == pid)
		{
			++children;
			if(proc->pflags & HANGING)
			{
				printk("[MM] do_wait :: %s (%d) is HANGING, so let's clean it up.\n",
					proc->pname, i);
				cleanup(proc);
				return;
			}
		}
	}

	if(children)
	{
		proc_table[pid].pflags |= WAITING;
		printk("[MM] do_wait :: %s (%d) is WAITING for child to exit.\n", 
			proc_table[pid].pname, pid);
	}
	else
	{
		printk("[MM] do_wait :: %s (%d) has no child at all.\n",
			proc_table[pid].pname, pid);

		MESSAGE msg;
		msg.type = SYSCALL_RET;
		msg.PID = NO_TASK;
		send_recv(SEND, pid, &msg);
	}
}