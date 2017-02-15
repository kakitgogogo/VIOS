#ifndef	VIOS_BITMAP_H
#define	VIOS_BITMAP_H

#include "type.h"

typedef struct bitmap
{
	u32	bytes_len;
	u8*	bits;
}bitmap;

PUBLIC void	bitmap_init(bitmap* bm, u8* addr, u32 len);
PUBLIC bool	bitmap_get(bitmap* bm, u32 idx);
PUBLIC int	bitmap_alloc(bitmap* bm, u32 cnt);
PUBLIC void	bitmap_set(bitmap* bm, u32 bit_idx, u8 value);
PUBLIC int	bitmap_alloc_and_set(bitmap* bm, u32 cnt);
PUBLIC int	bitmap_find_first_one(bitmap* bm);
PUBLIC void	bitmap_view(bitmap* bm);
PUBLIC void	bitmap_clear(bitmap* bm);

#endif