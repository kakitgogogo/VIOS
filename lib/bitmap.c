#include "bitmap.h"
#include "string.h"
#include "const.h"

PUBLIC void bitmap_init(bitmap* bm, u8* addr, u32 bits_len)
{
	bm->bits = addr;
	bm->bytes_len = bits_len / 8 + 1;
	memset(bm->bits, 0, bm->bytes_len);
}

PUBLIC bool bitmap_get(bitmap* bm, u32 idx)
{
	u32 byte_idx = idx / 8;
	u32 bit_idx = idx % 8;
	return (bm->bits[byte_idx] & (1 << bit_idx));
}

PUBLIC int bitmap_alloc(bitmap* bm, u32 cnt)
{
	u32 byte_idx = 0;
	while(bm->bits[byte_idx] == 0xFF && byte_idx < bm->bytes_len)
	{
		++byte_idx;
	}

	if(byte_idx == bm->bytes_len)
	{
		return -1;
	}

	u32 bit_idx = 0;
	while((u8)(1 << bit_idx) & bm->bits[byte_idx])
	{
		++bit_idx;
	}

	u32 bit_idx_start = byte_idx * 8 + bit_idx;
	u32 bit_left = bm->bytes_len * 8 - bit_idx_start;
	u32 cur_cnt = 0;
	bit_idx = bit_idx_start;

	while(bit_left-- > 0)
	{
		if(!bitmap_get(bm, bit_idx))
		{
			++cur_cnt;
		}
		else
		{
			cur_cnt = 0;
		}

		if(cur_cnt == cnt)
		{
			return bit_idx - cnt + 1;
		}
		++bit_idx;
	}
	return -1;
}

PUBLIC void bitmap_set(bitmap* bm, u32 idx, u8 value)
{
	assert(value == 0 || value == 1);

	u32 byte_idx = idx / 8;
	u32 bit_idx = idx % 8;

	if(value == 1)
	{
		bm->bits[byte_idx] |= (1 << bit_idx);
	}
	else
	{
		bm->bits[byte_idx] &= ~(1 << bit_idx);
	}
}

PUBLIC int bitmap_alloc_and_set(bitmap* bm, u32 cnt)
{
	int bit_idx_start = bitmap_alloc(bm, cnt), i;
	for(i = bit_idx_start; i < bit_idx_start + cnt; ++i)
	{
		bitmap_set(bm, i, 1);
	}
	return bit_idx_start;
}

PUBLIC int bitmap_find_first_one(bitmap* bm)
{	
	int byte_idx = 0;
	while(bm->bits[byte_idx] == 0 && byte_idx < bm->bytes_len)
	{
		++byte_idx;
	}
	if(byte_idx == bm->bytes_len) return -1;
	int bit_idx = 0;
	while(!((u8)(1 << bit_idx) & bm->bits[byte_idx]))
	{
		++bit_idx;
	}
	return byte_idx * 8 + bit_idx;
}

PUBLIC void bitmap_view(bitmap* bm)
{
	int i, j;
	for(i = 0; i < bm->bytes_len; ++i)
	{
		int tmp = bm->bits[i];
		for(j = 0; j < 8; ++j)
		{
			printk("%d ", (tmp & (1 << j)) ? 1 : 0);
		}
		if(i % 2 == 1) printk("\n");
		else printk("\t");
	}
	printk("\n");
}

PUBLIC void bitmap_clear(bitmap* bm)
{
	memset(bm->bits, 0, bm->bytes_len);
}