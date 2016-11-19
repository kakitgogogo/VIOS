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

PRIVATE int alloc_imap_bit(int dev)
{
	int inode_id = 0;
	int i, j, k;

	int imap_blk0_id = 1 + 1;

	super_block* sb = get_super_block(dev);

	for(i = 0; i < sb->nr_imap_sects; ++i)
	{
		RD_SECT(dev, imap_blk0_id + i);

		for(j = 0; j < SECTOR_SIZE; ++j)
		{
			if(fsbuf[j] == 0xFF)
				continue;

			for(k = 0; ((fsbuf[j] >> k) & 1) != 0; ++k);

			inode_id = (i * SECTOR_SIZE + j) * 8 + k;
			fsbuf[j] |= (1<<k);

			WR_SECT(dev, imap_blk0_id + i);
			break;
		}

		return inode_id;
	}

	panic("inode map is empty or full.\n");

	return 0;
}

PRIVATE int alloc_smap_bit(int dev, int nr_sects_to_alloc)
{
	int i, j, k;

	super_block* sb = get_super_block(dev);

	int smap_blk0_id = 1 + 1 + sb->nr_imap_sects;
	int free_sect_id = 0;

	for(i = 0; i < sb->nr_smap_sects; ++i)
	{
		RD_SECT(dev, smap_blk0_id + i);

		for(j = 0; j < SECTOR_SIZE && nr_sects_to_alloc > 0; ++j)
		{
			k = 0;
			if(!free_sect_id)
			{
				if(fsbuf[j] == 0xFF) continue;
				for(; ((fsbuf[j] >> k) & 1) != 0; ++k);

				free_sect_id = (i * SECTOR_SIZE + j) * 8 + k - 1 + sb->first_sect;
			}

			for(; k < 8; ++k)
			{
				assert(((fsbuf[j] >> k) & 1) == 0);
				fsbuf[j] |= (1 << k);
				if(--nr_sects_to_alloc == 0)
					break;
			}
		}

		if(free_sect_id)
		{
			WR_SECT(dev, smap_blk0_id + i);
		}

		if(nr_sects_to_alloc == 0)
		{
			break;
		}
	}

	assert(nr_sects_to_alloc == 0);

	return free_sect_id;
}

PRIVATE void new_dir_entry(inode* dir_inode, int inode_id, char* filename)
{
	int dir_blk0_id = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE) / SECTOR_SIZE;
	int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;

	int m = 0;
	dir_entry* de;
	dir_entry* new_de = 0;

	int i, j;
	for(i = 0; i< nr_dir_blks; ++i)
	{
		RD_SECT(dir_inode->i_dev, dir_blk0_id + i);

		de = (dir_entry*)fsbuf;
		for(j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; ++j, ++de)
		{
			if(++m > nr_dir_entries)
				break;
			if(de->inode_id == 0)
			{
				new_de = de;
				break;
			}
		}
		if(m > nr_dir_entries || new_de)
		{
			break;
		}
	}
	if(!new_de)
	{
		new_de = de;
		dir_inode->i_size += DIR_ENTRY_SIZE;
	}
	new_de->inode_id = inode_id;
	strcpy(new_de->name, filename);

	WR_SECT(dir_inode->i_dev, dir_blk0_id + i);

	sync_inode(dir_inode);
}

PRIVATE inode* create_file(char* path, int flags)
{
	char filename[MAX_PATH];
	inode* dir_inode;

	if(strip_path(filename, path, &dir_inode) != 0)
	{
		return 0;
	}

	int inode_id = alloc_imap_bit(dir_inode->i_dev);

	int free_sect_id = alloc_smap_bit(dir_inode->i_dev, NR_DEFAULT_FILE_SECTS);

	inode* newino = new_inode(dir_inode->i_dev, inode_id, free_sect_id);

	new_dir_entry(dir_inode, newino->i_num, filename);

	return newino;
}

PUBLIC int do_open()
{
	int fd = -1;

	char pathname[MAX_PATH];

	int flags = fs_msg.FLAGS;
	int name_len = fs_msg.NAME_LEN;
	int src = fs_msg.source;
	assert(name_len < MAX_PATH);
	memcpy((void*)va2la(TASK_FS, pathname), (void*)va2la(src, fs_msg.PATHNAME), name_len);
	pathname[name_len] = 0;

	int i;
	for(i = 0; i < NR_FILES; ++i)
	{
		if(fs_caller->files[i] == 0)
		{
			fd = i;
			break;
		}
	}
	if((fd < 0) || (fd >= NR_FILES))
	{
		panic("files[] is full (PID:%d)", proc2pid(fs_caller));
	}

	for(i = 0; i < NR_FILE_DESC; ++i)
	{
		if(fd_table[i].fd_inode == 0)
			break;
	}

	if(i >= NR_FILE_DESC)
	{
		panic("fd_table[] is full (PID:%d)", proc2pid(fs_caller));
	}
	
	int inode_id = search_file(pathname);

	inode* inode_ptr = 0;
	if(flags & O_CREAT)
	{
		if(inode_id)
		{
			printf("File Exists.\n");
			return -1;
		}
		else
		{
			inode_ptr = create_file(pathname, flags);
		}
	}
	else
	{
		assert(flags & O_RDWR);

		char filename[MAX_PATH];
		inode* dir_inode;
		if(strip_path(filename, pathname, &dir_inode) != 0)
			return -1;
		inode_ptr = get_inode(dir_inode->i_dev, inode_id);
	}

	if(inode_ptr)
	{
		fs_caller->files[fd] = &fd_table[i];

		fd_table[i].fd_inode = inode_ptr;
		fd_table[i].fd_mode = flags;
		fd_table[i].fd_pos = 0;

		int imode = inode_ptr->i_mode & I_TYPE_MASK;

		if(imode == I_CHAR_SPECIAL)
		{
			MESSAGE driver_msg;

			driver_msg.type = DEV_OPEN;
			int dev = inode_ptr->i_start_sect;
			driver_msg.DEVICE = MINOR(dev);
			assert(MAJOR(dev) == DEV_CHAR_TTY);
			assert(dd_map[MAJOR(dev)].driver_id != INVALID_DRIVER);

			send_recv(BOTH, dd_map[MAJOR(dev)].driver_id, &driver_msg);
		}
		else if(imode == I_DIRECTORY)
		{
			assert(inode_ptr->i_num == ROOT_INODE);
		}
		else
		{
			assert(inode_ptr->i_mode == I_REGULAR);
		}
	}
	else
	{
		return -1;
	}
	return fd;
}

PUBLIC int do_close()
{
	int fd = fs_msg.FD;
	put_inode(fs_caller->files[fd]->fd_inode);
	fs_caller->files[fd]->fd_inode = 0;
	fs_caller->files[fd] = 0;

	return 0;
}

PUBLIC int do_lssek()
{
	int fd = fs_msg.FD;
	int off = fs_msg.OFFSET;
	int whence = fs_msg.WHENCE;

	int pos = fs_caller->files[fd]->fd_pos;
	int fsize = fs_caller->files[fd]->fd_inode->i_size;

	switch(whence)
	{
	case SEEK_SET:
		pos = off;
		break;
	case SEEK_CUR:
		pos += off;
		break;
	case SEEK_END:
		pos = fsize + off;
		break;
	default:
		return -1;
		break;
	}
	fs_caller->files[fd]->fd_pos = pos;
	return pos;
}