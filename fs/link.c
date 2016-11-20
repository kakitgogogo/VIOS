#include "config.h"
#include "const.h"
#include "type.h"
#include "protect.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
#include "hd.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include "stdio.h"

PUBLIC int do_unlink()
{
	char pathname[MAX_PATH];

	int name_len = fs_msg.NAME_LEN;

	int src = fs_msg.source;
	assert(name_len < MAX_PATH);
	memcpy((void*)va2la(TASK_FS, pathname), (void*)va2la(src, fs_msg.PATHNAME), name_len);
	pathname[name_len] = 0;

	if(strcmp(pathname, "/" == 0))
	{
		printf("FS: do_unlink() : cannot unlink the root\n");
		return -1;
	}

	int inode_id = search_file(pathname);
	if(inode_id == INVALID_INODE)
	{
		printf("FS: do_unlink() : search_file() returns invalid inode: %s\n", 
			pathname);
		return -1;
	}

	char filename[MAX_PATH];
	inode* dir_inode;
	if(strip_path(filename, pathname, &dir_inode) != 0)
		return -1;

	inode* inode_ptr = get_inode(dir_inode->i_dev, inode_id);

	if(inode_ptr->i_mode != I_REGULAR)
	{
		printf("Cannot remove file %s. It is not a regular file.\n", 
			pathname);
		return -1;
	}

	if(inode_ptr->i_cnt > 1)
	{
		printf("Cannot remove file %s. It was opened(inode->i_cnt = %d).\n", 
			pathname, inode_ptr->i_cnt);
		return -1;
	}

	super_block* sb = get_super_block(inode_ptr->i_dev);

	/* free the bit in i-map */
	int byte_idx = inode_id / 8;
	int bit_idx = inode_id % 8;
	assert(byte_idx < SECTOR_SIZE);

	RD_SECT(inode_ptr->i_dev, 2);
	assert(fsbuf[byte_idx % SECTOR_SIZE] & (1 << bit_idx));
	fsbuf[byte_idx % SECTOR_SIZE] &= ~(1 << bit_idx);
	WR_SECT(inode_ptr, 2);

	/* free the bits in s-map */
	bit_idx = inode_ptr->i_start_sect - sb->first_sect + 1;
	byte_idx = bit_idx / 8;
	int bits_left = inode_ptr->i_nr_sects;
	int byte_cnt = (bits_left - (8 - (bit_idx % 8))) / 8;

	int cur_sect = 2 + sb->nr_imap_sects + byte_idx / SECTOR_SIZE;
	RD_SECT(inode_ptr->i_dev, cur_sect);

	int i;
	for(i = bit_idx % 8; (i < 8) && bits_left; ++i, --bits_left)
	{
		assert((fsbuf[byte_idx % SECTOR_SIZE] >> i & 1) == 1);
		fsbuf[byte_idx % SECTOR_SIZE] &= ~(i << i);
	}

	int k;
	i = (byte_idx % SECTOR_SIZE) + 1;

	for(k = 0; k < byte_cnt; ++k, ++i, bits_left -= 8)
	{
		if(i == SECTOR_SIZE)
		{
			i = 0;
			WR_SECT(inode_ptr->i_dev, cur_sect);
			RD_SECT(inode_ptr->i_dev, ++cur_sect);
		}
		assert(fsbuf[i] = 0xFF);
		fsbuf[i] = 0;
	}

	if(i == SECTOR_SIZE)
	{
		i = 0;
		WR_SECT(inode_ptr->i_dev, cur_sect);
		RD_SECT(inode_ptr->i_dev, ++cur_sect);
	}
	unsigned char mask = ~((unsigned char)(~0) << bits_left);
	assert((fsbuf[i] & mask) == mask);
	fsbuf[i] &= (~0) << bits_left;
	WR_SECT(inode_ptr->i_dev, cur_sect);

	/* clear the i-node */
	inode_ptr->i_mode = 0;
	inode_ptr->i_size = 0;
	inode_ptr->i_start_sect = 0;
	inode_ptr->i_nr_sects = 0;
	sync_inode(inode_ptr);
	put_inode(inode_ptr);

	/* delete directory entry */
	int dir_blk0_id = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE) / SECTOR_SIZE;
	int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;

	int m = 0;
	dir_entry* de = 0;
	bool flg = FALSE;
	int dir_size = 0;

	for(i = 0; i < nr_dir_blks; ++i)
	{
		RD_SECT(dir_inode->i_dev, dir_blk0_id + i);

		de = (dir_entry*)fsbuf;
		int j;
		for(j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; ++j, ++de)
		{
			if(++m > nr_dir_entries)
				break;
			if(de->inode_id == inode_id)
			{
				memset(de, 0, DIR_ENTRY_SIZE);
				WR_SECT(dir_inode->i_dev, dir_blk0_id + i);
				flg = TRUE;
				break;
			}
			if (de->inode_id != INVALID_INODE)
				dir_size += DIR_ENTRY_SIZE;
		}
		if(m > nr_dir_entries || flg)
		{
			break;
		}
	}
	assert(flg);
	if(m == nr_dir_entries)
	{
		dir_inode->i_size = dir_size;
		sync_inode(dir_inode);
	}

	return 0;
}