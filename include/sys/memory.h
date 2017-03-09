#ifndef VIOS_MEMORY_H
#define VIOS_MEMORY_H

#ifdef	VIOS_MEMORY_C
#undef	EXTERN
#define	EXTERN
#endif

#include "type.h"

#define MAX_ORDER	27
#define MIN_ORDER	2
#define LIST_SIZE	(MAX_ORDER+2)

#define MEM_SIZE		(1 << MAX_ORDER)
#define BLOCK_SIZE(i)	(1 << (i))

#define MEM_BASE		0xA00000
#define MEM_OFFSET(b)	((u32)b - MEM_BASE)
#define BUDDY_OF(b, i)	((mem_ptr)((MEM_OFFSET(b) ^ (1 << (i))) + MEM_BASE))

typedef void* mem_ptr;

mem_ptr malloc();
void free(mem_ptr block);
void buddy_init();
void buddy_print();

EXTERN mem_ptr freelist[LIST_SIZE];

#endif