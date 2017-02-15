#include "list.h"
#include "const.h"
#include "type.h"
#include "protect.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
#include "proc.h"
#include "global.h"

int test_list()
{
	int i;
	list_head list;
	PROCESS* pos;

	list_init(&list);
	for(i = 0; i < NR_TASKS; ++i)
	{
		list_head* new = &proc_table[i].run_list;
		list_add(&list, new);
		printk("list.next: %s\n", list_entry(list.next->next, PROCESS, run_list)->pname);
	}

	list_del(&proc_table[TASK_MM].run_list);
	
	printk("proc_list:");
	list_for_each_entry(&list, pos, run_list) printk("%s->", pos->pname);
	printk("list_tail\n");

	list_head* cur = list_find(&list, &(proc_ready->run_list));
	if(cur != NULL) printk("current: %s\n", list_entry(cur, PROCESS, run_list)->pname);

}