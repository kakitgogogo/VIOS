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

PUBLIC void reset_msg(MESSAGE* msg)
{
	memset((void*)msg, 0, sizeof(MESSAGE));
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

PUBLIC void inform_int(int task_id)
{
	PROCESS *proc = proc_table + task_id;
	if((proc->pflags & RECEIVING) && ((proc->recvfrom == INTERRUPT) || (proc->recvfrom == ANY)))
	{
		proc->pmsg->source = INTERRUPT;
		proc->pmsg->type = HARD_INT;
		proc->pmsg = 0;
		proc->has_int_msg = FALSE;
		proc->pflags &= ~RECEIVING;

		proc->recvfrom = NO_TASK;
		assert(proc->pflags == 0);
		unblock(proc);

		assert(proc->pflags == 0);
		assert(proc->pmsg == 0);
		assert(proc->recvfrom == NO_TASK);
		assert(proc->sendto == NO_TASK);
	}
	else
	{
		proc->has_int_msg = TRUE;
	}
}

PUBLIC void dump_proc(PROCESS *proc)
{
	char info[STR_DEFAULT_LEN];
	int i;
	int text_color = RED_CHAR;

	int dump_len = sizeof(PROCESS);

	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, 0);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, 0);

	sprintf(info, "byte dump of proc_table[%d]:\n", proc - proc_table); 
	disp_color_str(info, text_color);
	for (i = 0; i < dump_len; i++) {
		sprintf(info, "%x.", ((unsigned char *)proc)[i]);
		disp_color_str(info, text_color);
	}

	disp_color_str("\n\n", text_color);
	sprintf(info, "ANY: %d.\n", ANY); 
	disp_color_str(info, text_color);
	sprintf(info, "NO_TASK: %d.\n", NO_TASK); 
	disp_color_str(info, text_color);
	disp_color_str("\n", text_color);

	sprintf(info, "ldt_selector: 0x%x.  ", proc->ldt_selector); 
	disp_color_str(info, text_color);
	sprintf(info, "ticks: %d.  ", proc->ticks); 
	disp_color_str(info, text_color);
	sprintf(info, "priority: %d.  ", proc->priority); 
	disp_color_str(info, text_color);
	sprintf(info, "pid: %d.  ", proc->pid); 
	disp_color_str(info, text_color);
	sprintf(info, "name: %s.  ", proc->pname); 
	disp_color_str(info, text_color);
	disp_color_str("\n", text_color);
	sprintf(info, "pflags: 0x%x.  ", proc->pflags); 
	disp_color_str(info, text_color);
	sprintf(info, "recvfrom: 0x%x.  ", proc->recvfrom); 
	disp_color_str(info, text_color);
	sprintf(info, "sendto: 0x%x.  ", proc->sendto); 
	disp_color_str(info, text_color);
	sprintf(info, "tty_id: %d.  ", proc->tty_id); 
	disp_color_str(info, text_color);
	disp_color_str("\n", text_color);
	sprintf(info, "has_int_msg: 0x%x.  ", proc->has_int_msg); 
	disp_color_str(info, text_color);
}

PUBLIC void dump_msg(const char * title, MESSAGE* msg)
{
	int packed = 0;
	printf("{%s}<0x%x>{%ssrc:%s(%d),%stype:%d,%s(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)%s}%s",  //, (0x%x, 0x%x, 0x%x)}",
	       title,
	       (int)msg,
	       packed ? "" : "\n        ",
	       proc_table[msg->source].pname,
	       msg->source,
	       packed ? " " : "\n        ",
	       msg->type,
	       packed ? " " : "\n        ",
	       msg->u.m3.m3i1,
	       msg->u.m3.m3i2,
	       msg->u.m3.m3i3,
	       msg->u.m3.m3i4,
	       (int)msg->u.m3.m3p1,
	       (int)msg->u.m3.m3p2,
	       packed ? "" : "\n",
	       packed ? "" : "\n"
		);
}
