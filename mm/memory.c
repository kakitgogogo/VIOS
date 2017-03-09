#define VIOS_MEMORY_C

#include "type.h"
#include "memory.h"

mem_ptr malloc(int size)
{
	u8 i, order;
	mem_ptr block, buddy;

	i = 0;
	while(BLOCK_SIZE(i) < size + 1) ++i;

	order = i = (i < MIN_ORDER) ? MIN_ORDER : i;

	for(;;++i)
	{
		if(i > MAX_ORDER)
		{
			return NULL;
		}
		if(freelist[i])
		{
			break;
		}
	}

	block = freelist[i];
	freelist[i] = *(mem_ptr*)freelist[i];

	while(i-- > order) 
	{
		buddy = BUDDY_OF(block, i);
		freelist[i] = buddy;
	}

	*((u8*)block - 1) = order;
	return block;
}


void free(mem_ptr block)
{
	u8 i;
	mem_ptr buddy;
	mem_ptr* p;

	i = *((u8*)block - 1);

	for(;;++i)
	{
		buddy = BUDDY_OF(block, i);
		p = &(freelist[i]);

		// find buddy in list
		while((*p != NULL) && (*p != buddy))
		{
			p = (mem_ptr*) *p;
		}

		// not found, insert into list
		if(*p != buddy) 
		{
			*(mem_ptr*) block = freelist[i];
			freelist[i] = block;
			return;
		}
		// found, merged block starts from the lower one
		block = (block < buddy) ? block : buddy;
		// remove buddy out of list
		*p = *(mem_ptr*) *p;
	}
}

void buddy_init()
{
	memset(freelist, 0, sizeof(mem_ptr)*LIST_SIZE);
	freelist[MAX_ORDER] = MEM_BASE;
}