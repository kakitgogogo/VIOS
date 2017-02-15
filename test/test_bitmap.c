#include "bitmap.h"
#include "const.h"
#include "type.h"
#include "global.h"

int test_bitmap()
{

	bitmap local_bitmap;
	bitmap_init(&local_bitmap, global_bitmap, 32);
	bitmap_alloc_and_set(&local_bitmap, 16);
	bitmap_set(&local_bitmap, 15, 0);
	bitmap_set(&local_bitmap, 17, 1);
	bitmap_alloc_and_set(&local_bitmap, 8);
	if(bitmap_get(&local_bitmap, 17)) bitmap_view(&local_bitmap);

	bitmap_clear(&local_bitmap);
	bitmap_set(&local_bitmap, 8, 1);
	int idx = bitmap_find_first_one(&local_bitmap);
	printk("first one: %d\n", idx);

	return 0;
}