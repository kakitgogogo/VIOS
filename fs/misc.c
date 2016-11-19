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

PUBLIC int strip_path(char* filename, const char* pathname, inode** inode_ptr_ptr)
{
	const char* s = pathname;
	char* t = filename;

	if(s == 0)
	{
		return -1;
	}

	if(*s == '/')
	{
		++s;
	}

	while(*s)
	{
		if(*s == '/')
		{
			return -1;
		}
		*t++ = *s++;

		if(t - filename >= MAX_FILENAME_LEN)
		{
			break;
		}
	}

	*t = 0;

	*inode_ptr_ptr = root_inode;

	return 0;
}

PUBLIC int search_file(char* path)
{
	int i, j;

	char filename[MAX_PATH];
	memset(filename, 0, MAX_FILENAME_LEN);
	inode* dir_inode;

	if(strip_path(filename, path, &dir_inode) != 0)
	{
		return 0;
	}

	if(filename[0] == 0)
	{
		return dir_inode->i_num;
	}

	int dir_blk0_id = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;

	int m = 0;
	dir_entry *de;
	for(i = 0; i < nr_dir_blks; ++i)
	{
		RD_SECT(dir_inode->i_dev, dir_blk0_id + i);
		
		de = (dir_entry*)fsbuf;

		for(j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; ++j, ++de)
		{
			if(memcmp(filename, de->name, MAX_FILENAME_LEN) == 0)
				return de->inode_id;
			if(++m > nr_dir_entries)
				break;
		}
		if(m > nr_dir_entries)
		{
			break;
		}
	}

	return 0;
}