#ifndef	VIOS_LIST_H
#define	VIOS_LIST_H

#include "kernel.h"

#define LIST_POISON1	((void*)0x501000)
#define LIST_POISON2	((void*)0x600000)

typedef struct list_head
{
	struct list_head *next;
	struct list_head *prev;
}list_head;

#define LIST_HEAD(name) \
	struct list_head name = { &(name), &(name) };

#define INIT_LIST_HEAD(ptr) \
	do\
	{
		(ptr)->next = (ptr);\
		(ptr)->prev = (ptr);\
	}\
	while(0)

static inline void __list_add(list_head* new, list_head* prev, list_head* next)
{
	prev->next = new;
	new->prev = prev;
	new->next = next;
	next->prev = new;
}

static inline list_add(list_head* new, list_head* head)
{
	__list_add(new, head, head->next);
}

static inline list_add_tail(list_head* new, list_head* head)
{
	__list_add(new, head->prev, head);
}

static inline __list_del(list_head* prev, list_head* next)
{
	prev->next = next;
	next->prev = prev;
}

static inline void list_del(list_head* entry)
{
	__list_del(entry->prev, entry->next);
	entry->prev = LIST_POISON1;
	entry->next = LIST_POISON2;
}

static inline void list_del_init(list_head* entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

static inline void list_move(list_head* list, list_head* head)
{
	__list_del(list->prev, list->next);
	list_add(list, head);
}

static inline void list_move_tail(list_head* list, list_head* head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
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
		INIT_LIST_HEAD(list);
	}
}

#define list_entry(ptr, type, member) container_of(ptr, type, member)

static inline void prefetch(const void *x) {;} /*TODO*/

#define list_for_each(pos, head) \
	for(	pos = (head)->next, prefetch(pos->next); \
		pos != (head); \
		pos = pos->next, prefetch(pos->next))

#define __list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head) \
	for(	pos = (head)->prev, prefetch(pos->prev); \
		pos != (head); \
		pos = pos->prev, prefetch(pos->prev))

#define __list_for_each(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_for_each_safe(pos, n, head) \
	for(	pos = (head)->next, n = pos->next; \
		pos != (head); \
		pos = n, n = pos->next)

#define list_for_each_entry(pos, head, member) \
	for(	pos = list_entry((head)->next, typeof(*pos), member), prefetch(pos->member.next);	 \
		&pos->member != (head); \
		pos = list_entry(pos->member.next, typeof(*pos), member), prefetch(pos->member.next))

#define list_for_each_entry_reverse(pos, head, member) \
	for(	pos = list_entry((head)->prev, typeof(*pos), member), prefetch(pos->member.prev);	 \
		&pos->member != (head); \
		pos = list_entry(pos->member.prev, typeof(*pos), member), prefetch(pos->member.prev))

#define list_for_each_entry_continue(pos, head, member) \
	for(	pos = list_entry(pos->member.next, typeof(*pos), member), prefetch(pos->member.next); \
		&pos->member != (head); \
		pos = list_entry(pos->member.next, typeof(*pos), member), prefetch(pos->member.next))

#define list_for_each_safe(pos, n, head, member) \
	for(	pos = list_entry((head)->next, typeof(*pos), member), \
		n = list_entry(pos->member.next, typeof(*pos), member); \
		&pos->member != (head); \
		pos = n, n = list_entry(pos->member.next, typeof(*pos), member); 

