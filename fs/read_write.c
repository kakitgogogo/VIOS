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

PUBLIC int do_rdwt()
{
	int fd = fs_msg.FD;
	void* buf = fs_msg.BUF;
	int len = fs_msg.CNT;

	int src = fs_msg.source;

	assert((fs_caller->files[fd] >= &fd_table[0]) && (fs_caller->files[fd] < &fd_table[NR_FILE_DESC]));

	if(!(fs_caller->files[fd]->fd_mode & O_RDWR))
	{
		return -1;
	}

	int pos = fs_caller->files[fd]->fd_pos;

	inode* inode_ptr = fs_caller->files[fd]->fd_inode;

	assert(inode_ptr >= &inode_table[0] && inode_ptr < &inode_table[NR_INODE]);

	int imode = inode_ptr->i_mode & I_TYPE_MASK;

	if(imode == I_CHAR_SPECIAL)
	{
		int type = fs_msg.type == READ ? DEV_READ : DEV_WRITE;
		fs_msg.type = type;

		int dev = inode_ptr->i_start_sect;
		assert(MAJOR(dev) == 4);

		fs_msg.DEVICE = MINOR(dev);
		fs_msg.BUF = buf;
		fs_msg.CNT = len;
		fs_msg.PROC_ID = src;
		assert(dd_map[MAJOR(dev)].driver_id != INVALID_DRIVER);
		send_recv(BOTH, dd_map[MAJOR(dev)].driver_id, &fs_msg);
		assert(fs_msg.CNT == len);

		return fs_msg.CNT;
	}
	else
	{
		assert(inode_ptr->i_mode == I_REGULAR || inode_ptr->i_mode == I_DIRECTORY);
		assert((fs_msg.type == READ) || (fs_msg.type == WRITE));

		int pos_end;
		if(fs_msg.type == READ)
		{
			pos_end = min(pos + len, inode_ptr->i_size);
		}
		else
		{
			pos_end = min(pos + len, inode_ptr->i_nr_sects * SECTOR_SIZE);
		}

		int off = pos % SECTOR_SIZE;
		int rw_sect_min = inode_ptr->i_start_sect + (pos >> SECTOR_SIZE_SHIFT);
		int rw_sect_max = inode_ptr->i_start_sect + (pos_end >> SECTOR_SIZE_SHIFT);

		int chunk = min(rw_sect_max - rw_sect_min + 1, FSBUF_SIZE >> SECTOR_SIZE_SHIFT);

		int bytes_rw = 0;
		int bytes_left = len;
		int i;
		for(i = rw_sect_min; i <= rw_sect_max; i += chunk)
		{
			int bytes = min(bytes_left, chunk * SECTOR_SIZE - off);

			rw_sector(DEV_READ, inode_ptr->i_dev, i* SECTOR_SIZE, chunk * SECTOR_SIZE, TASK_FS, fsbuf);

			if(fs_msg.type == READ)
			{
				memcpy((void*)va2la(src, buf + bytes_rw), (void*)va2la(TASK_FS, fsbuf + off), bytes);
			}
			else
			{
				memcpy((void*)va2la(TASK_FS, fsbuf + off), (void*)va2la(src, buf + bytes_rw), bytes);

				rw_sector(DEV_WRITE, inode_ptr->i_dev, i* SECTOR_SIZE, chunk * SECTOR_SIZE, TASK_FS, fsbuf);
			}
			off = 0;
			bytes_rw += bytes;
			fs_caller->files[fd]->fd_pos += bytes;
			bytes_left -= bytes;
		}

		if(fs_caller->files[fd]->fd_pos > inode_ptr->i_size)
		{
			inode_ptr->i_size = fs_caller->files[fd]->fd_pos;

			sync_inode(inode_ptr);
		}

		return bytes_rw;
	}
}