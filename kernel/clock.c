#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "string.h"
#include "global.h"

PUBLIC void clock_handler(int irq)
{
	disp_str(".");
	proc_ready++;
	if(proc_ready >= proc_table + NR_TASKS)
	{
		proc_ready = proc_table;
	}
}