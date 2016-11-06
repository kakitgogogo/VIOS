#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

PUBLIC void schedule()
{
	PROCESS *p;
	int greatest_ticks = 0;

	while(!greatest_ticks)
	{
		for(p = &FIRST_PROC; p <= &LAST_PROC; ++p)
		{
			if(p->pflags == 0)
			{
				if(p->ticks > greatest_ticks)
				{
					greatest_ticks = p->ticks;
					proc_ready = p;
				}
			}
		}
		if(!greatest_ticks)
		{
			for(p = &FIRST_PROC; p <= &LAST_PROC; ++p)
			{
				if(p->pflags == 0)
				{
					p->ticks = p->priority;
				}
			}
		}
	}
}

PUBLIC int sys_get_ticks()
{
	return ticks;
}

PUBLIC int sys_write(int _unused1, char* buf, int len, PROCESS* proc)
{
	tty_write(&tty_table[proc->tty_id], buf, len);
	return 0;
}


PUBLIC int ldt_seg_linear(PROCESS *proc, int idx)
{
	DESCRIPTOR *desc = &proc->ldts[idx];

	return desc->base_high << 24 | desc->base_mid << 16 | desc->base_low;
}

PUBLIC void* va2la(int pid, void* va)
{
	PROCESS* proc = &proc_table[pid];

	u32 seg_base = ldt_seg_linear(proc, INDEX_LDT_RW);
	u32 la = seg_base + (u32)va;

	if(pid < NR_TASKS + NR_PROCS)
	{
		assert(la == (u32)va);
	}

	return (void*)la;
}

PUBLIC void reset_msg(MESSAGE* msg)
{
	memset(msg, 0, sizeof(MESSAGE));
}

PRIVATE void block(PROCESS *proc)
{
	assert(proc->pflags);
	schedule();
}

PRIVATE void unblock(PROCESS *proc)
{
	assert(proc->pflags == 0);
}

PRIVATE bool deadlock(int src, int des)
{
	PROCESS *proc = proc_table + des;
	while(1)
	{
		if(proc->pflags & SENDING)
		{
			if(proc->sendto == src)
			{
				proc = proc_table + des;
				printf("DeadLock: %s", proc->pname);
				do
				{
					assert(proc->pmsg);
					proc = proc_table + proc->sendto;
					printf("->%s", proc->pname);
				} while (proc != proc_table + src);

				return TRUE;
			}
			proc = proc_table + proc->sendto;
		}
		else 
		{
			break;
		}
	}
	return FALSE;
}

PRIVATE int msg_send(PROCESS* cur, int des, MESSAGE* msg)
{
	PROCESS* sender = cur;
	PROCESS* target = proc_table + des;

	assert(proc2pid(sender) != des);

	if(deadlock(proc2pid(sender), des))
	{
		panic(">>DEADLOCK<< %s->%s", sender->pname, target->pname);
	}

	if((target->pflags & RECEIVING) && (target->recvfrom == proc2pid(sender) || target->recvfrom == ANY))
	{
		assert(target->pmsg);
		assert(msg);

		memcpy(va2la(des, target->pmsg), va2la(proc2pid(sender), msg), sizeof(MESSAGE));
		target->pmsg = 0;
		target->pflags &= ~RECEIVING;
		target->recvfrom = NO_TASK;
		unblock(target);

		assert(target->pflags == 0);
		assert(target->pmsg == 0);
		assert(target->recvfrom == NO_TASK);
		assert(target->sendto == NO_TASK);
		assert(sender->pflags == 0);
		assert(sender->pmsg == 0);
		assert(sender->recvfrom == NO_TASK);
		assert(sender->sendto == NO_TASK);
	}
	else
	{
		sender->pflags |= SENDING;
		assert(sender->pflags == SENDING);
		sender->sendto = des;
		sender->pmsg = msg;

		PROCESS *p;
		if(target->sending)
		{
			p = target->sending;
			while(p->next_sending)
			{
				p = p->next_sending;
			}
			p->next_sending = sender;
		}
		else
		{
			target->sending = sender;
		}
		sender->next_sending = 0;

		block(sender);

		assert(sender->pflags == SENDING);
		assert(sender->pmsg != 0);
		assert(sender->recvfrom == NO_TASK);
		assert(sender->sendto == des);
	}

	return 0;
}

PRIVATE int msg_receive(PROCESS* cur, int src, MESSAGE* msg)
{
	PROCESS *receiver = cur;
	PROCESS *from = 0;
	PROCESS *prev = 0;
	bool copyok = FALSE;

	assert(proc2pid(receiver) != src);

	if((receiver->has_int_msg) && ((src == ANY) || (src == INTERRUPT)))
	{
		MESSAGE tmp;
		reset_msg(&tmp);
		tmp.source = INTERRUPT;
		tmp.type = HARD_INT;
		assert(msg);
		memcpy(va2la(proc2pid(receiver), msg), &tmp, sizeof(MESSAGE));

		receiver->has_int_msg = FALSE;

		assert(receiver->pflags == 0);
		assert(receiver->pmsg == 0);
		assert(receiver->sendto == NO_TASK);
		assert(receiver->has_int_msg == 0);

		return 0;
	}

	if(src == ANY)
	{
		if(receiver->sending)
		{
			from = receiver->sending;
			copyok = TRUE;

			assert(receiver->pflags == 0);
			assert(receiver->pmsg == 0);
			assert(receiver->recvfrom == NO_TASK);
			assert(receiver->sendto == NO_TASK);
			assert(receiver->sending != 0);
			assert(from->pflags == SENDING);
			assert(from->pmsg != 0);
			assert(from->recvfrom == NO_TASK);
			assert(from->sendto == proc2pid(receiver));
		}
	}
	else
	{
		from = &proc_table[src];

		if((from->pflags & SENDING) && (from->sendto == proc2pid(receiver)))
		{
			copyok = TRUE;
			PROCESS *p = receiver->sending;
			assert(p);
			while(p)
			{
				assert(from->pflags & SENDING);
				if(proc2pid(p) == src)
				{
					from = p;
					break;
				}
				prev = p;
				p = p->next_sending;
			}

			assert(receiver->pflags == 0);
			assert(receiver->pmsg == 0);
			assert(receiver->recvfrom == NO_TASK);
			assert(receiver->sendto == NO_TASK);
			assert(receiver->sending != 0);
			assert(from->pflags == SENDING);
			assert(from->pmsg != 0);
			assert(from->recvfrom == NO_TASK);
			assert(from->sendto == proc2pid(receiver));
		}
	}

	if(copyok)
	{
		if(from == receiver->sending)
		{
			assert(prev == 0);
			receiver->sending = from->next_sending;
			from->next_sending = 0;
		}
		else
		{
			assert(prev);
			prev->next_sending = from->next_sending;
			from->next_sending = 0;
		}
		assert(msg);
		assert(from->pmsg);
		memcpy(va2la(proc2pid(receiver), msg), va2la(proc2pid(from), from->pmsg), sizeof(MESSAGE));

		from->pmsg = 0;
		from->sendto = NO_TASK;
		from->pflags &= ~SENDING;
		unblock(from);
	}
	else
	{
		receiver->pflags |= RECEIVING;
		receiver->pmsg = msg;

		if(src == ANY)
			receiver->recvfrom = ANY;
		else
			receiver->recvfrom = proc2pid(from);

		block(receiver);

		assert(receiver->pflags == RECEIVING);
		assert(receiver->pmsg != 0);
		assert(receiver->recvfrom != NO_TASK);
		assert(receiver->sendto == NO_TASK);
		assert(receiver->has_int_msg == FALSE);
	}
	return 0;
}

PUBLIC int sys_sendrec(int function, int src_des, MESSAGE* msg, PROCESS* proc)
{
	assert(k_reenter == 0);

	//printf("function = %d; src_des = %d; ANY = %d\n", function, src_des, ANY);
	assert((src_des >= 0 && src_des < NR_TASKS + NR_PROCS) || src_des == ANY || src_des == INTERRUPT);
	
	int ret = 0;
	int caller = proc2pid(proc);
	MESSAGE *mla = (MESSAGE*)va2la(caller, msg);
	mla->source = caller;

	assert(mla->source != src_des);

	if(function == SEND)
	{
		ret = msg_send(proc, src_des, msg);
		if(ret != 0)
			return ret;
	}
	else if(function == RECEIVE)
	{
		ret = msg_receive(proc, src_des, msg);
		if(ret != 0)
			return ret;
	}
	else
	{
		panic("{sys_sendrec} invalid function: %d (SEND:%d, RECEIVE:%d).", function, SEND, RECEIVE);
	}

	return 0;
}

PUBLIC int send_recv(int function, int src_des, MESSAGE* msg)
{
	int ret = 0;

	if(function == RECEIVE)
	{
		memset(msg, 0, sizeof(MESSAGE));
	}

	switch(function)
	{
	case BOTH:
		ret = sendrec(SEND, src_des, msg);
		if(ret == 0)
			ret = sendrec(RECEIVE, src_des, msg);
		break;
	case SEND:
	case RECEIVE:
		ret = sendrec(function, src_des, msg);
		break;
	default:
		assert((function == BOTH) || (function == SEND) || (function == RECEIVE));
		break;
	}

	return ret;
}

PUBLIC int sys_printx(int _unused1, int _unused2, char* s, PROCESS* proc)
{
	const char *p;
	char ch;

	char reenter_err[] = "? k_reenter is incorrect for unknown reason";
	reenter_err[0] = MAG_CH_PANIC;

	if(k_reenter == 0)
		p = va2la(proc2pid(proc), s);
	else if(k_reenter > 0)
		p = s;
	else
		p = reenter_err;

	if((*p == MAG_CH_PANIC) || (*p == MAG_CH_ASSERT && proc_ready < &proc_table[NR_TASKS]))
	{
		disable_int();
		char *v = (char*)V_MEM_BASE;
		const char *q = p + 1;

		while(v < (char*)(V_MEM_BASE + V_MEM_SIZE))
		{
			*v++ = *q++;
			*v++ = RED_CHAR;
			if(!*q)
			{
				while(((int)v - V_MEM_BASE) % (SCREEN_WIDTH * 8))
				{
					*v++ = ' ';
					*v++ = RED_CHAR;
				}
				break;
			}
		}
		__asm__ __volatile__("hlt");
	}

	while((ch = *p++) != 0)
	{
		if(ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT)
			continue;

		out_char(tty_table[proc->tty_id].console, ch);
	}

	return 0;
}