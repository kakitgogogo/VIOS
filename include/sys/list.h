#ifndef	VIOS_LIST_H
#define	VIOS_LIST_H

#include "compiler.h"
#include "kernel.h"
#include "type.h"
#include "stdio.h"

#define LIST_POISON	((void*)0x0)

struct list_head
{
	struct list_head *next;
	struct list_head *prev;
};

typedef struct list_head list_head;

#define LIST_HEAD(name) \
	struct list_head name = { &(name), &(name) }

#define list_init(ptr) \
	do\
	{\
		(ptr)->next = (ptr);\
		(ptr)->prev = (ptr);\
	}\
	while(0)

static inline void __list_add(list_head* new, list_head* prev, list_head* next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(list_head* list, list_head* new)
{
	__list_add(new, list, list->next);
}

static inline void list_add_tail(list_head* list, list_head* new)
{
	__list_add(new, list->prev, list);
}

static inline void __list_del(list_head* prev, list_head* next)
{
	prev->next = next;
	next->prev = prev;
}

static inline void list_del(list_head* entry)
{
	__list_del(entry->prev, entry->next);
	entry->prev = LIST_POISON;
	entry->next = LIST_POISON;
}

static inline void list_del_init(list_head* entry)
{
	__list_del(entry->prev, entry->next);
	list_init(entry);
}

static inline void list_move(list_head* pos1, list_head* pos2)  //move pos1 to pos2
{
	__list_del(pos1->prev, pos1->next);
	list_add(pos2, pos1);
}

static inline void list_move_tail(list_head* pos1, list_head* pos2)
{
	__list_del(pos1->prev, pos1->next);
	list_add_tail(pos2, pos1);
}

static inline bool list_empty(const list_head* head)
{
	return head->next == head;
}

static inline bool list_empty_careful(const list_head* head)
{
	return (head->next == head) && (head->next == head->prev);
}

static inline void __list_splice(list_head* list, list_head* head)
{
	list_head* first = list->next;
	list_head* last = list->prev;
	list_head* at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

static inline void list_splice(list_head* list, list_head* head)
{
	if(!list_empty(list))
	{
		__list_splice(list, head);
	}
}

static inline void list_splice_init(list_head* list, list_head* head)
{
	if(!list_empty(list))
	{
		__list_splice(list, head);
		list_init(list);
	}
}

#define list_entry(ptr, type, member) container_of(ptr, type, member)

#define list_for_each(head, pos) \
	for(	pos = (head)->next, prefetch(pos->next); \
		pos != (head); \
		pos = pos->next, prefetch(pos->next))

#define __list_for_each(head, pos) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(head, pos) \
	for(	pos = (head)->prev, prefetch(pos->prev); \
		pos != (head); \
		pos = pos->prev, prefetch(pos->prev))

#define __list_for_each_prev(head, pos) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_for_each_safe(head, pos, n) \
	for(	pos = (head)->next, n = pos->next; \
		pos != (head); \
		pos = n, n = pos->next)

#define list_for_each_entry(head, pos, member) \
	for(	pos = list_entry((head)->next, typeof(*pos), member), prefetch(pos->member.next);	 \
		&pos->member != (head); \
		pos = list_entry(pos->member.next, typeof(*pos), member), prefetch(pos->member.next))

#define list_for_each_entry_reverse(head, pos, member) \
	for(	pos = list_entry((head)->prev, typeof(*pos), member), prefetch(pos->member.prev);	 \
		&pos->member != (head); \
		pos = list_entry(pos->member.prev, typeof(*pos), member), prefetch(pos->member.prev))

#define list_for_each_entry_continue(head, pos, member) \
	for(	pos = list_entry(pos->member.next, typeof(*pos), member), prefetch(pos->member.next); \
		&pos->member != (head); \
		pos = list_entry(pos->member.next, typeof(*pos), member), prefetch(pos->member.next))

#define list_for_each_entry_safe(head, pos, n, member) \
	for(	pos = list_entry((head)->next, typeof(*pos), member), \
		n = list_entry(pos->member.next, typeof(*pos), member); \
		&pos->member != (head); \
		pos = n, n = list_entry(pos->member.next, typeof(*pos), member)

static inline list_head* list_find(list_head* list, list_head* head)
{
	list_head* pos;
	list_for_each(list, pos) if(pos == head) return pos;
	return LIST_POISON;
}

#endif