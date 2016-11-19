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

PUBLIC int rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_id, void* buf)
{
	MESSAGE driver_msg;

	driver_msg.type		= io_type;
	driver_msg.DEVICE	= MINOR(dev);
	driver_msg.POSITION	= pos;
	driver_msg.BUF		= buf;
	driver_msg.CNT		= bytes;
	driver_msg.PROC_ID	= proc_id;

	//printf("%d 0x%xla: " ,driver_msg.PROC_ID, driver_msg.BUF);

	assert(dd_map[MAJOR(dev)].driver_id != INVALID_DRIVER);
	send_recv(BOTH, dd_map[MAJOR(dev)].driver_id, &driver_msg);

	return 0;
}

PRIVATE void read_super_block(int dev)
{
	int i;
	/*
	MESSAGE driver_msg;

	driver_msg.type		= DEV_READ;
	driver_msg.DEVICE	= MINOR(dev);
	driver_msg.POSITION	= SECTOR_SIZE * 1;
	driver_msg.BUF		= fsbuf;
	driver_msg.CNT		= SECTOR_SIZE;
	driver_msg.PROC_ID	= TASK_FS;

	assert(dd_map[MAJOR(dev)].driver_id != INVALID_DRIVER);
	send_recv(BOTH, dd_map[MAJOR(dev)].driver_id, &driver_msg);
	*/
	RD_SECT(dev, 1);

	for(i = 0; i < NR_SUPER_BLOCK; ++i)
	{
		if(sb_table[i].sb_dev == NO_DEV)
		{
			break;
		}
	}
	if(i == NR_SUPER_BLOCK)
	{
		panic("Super Block Slots used up");
	}

	assert(i == 0);

	super_block *sb = (super_block*)fsbuf;

	sb_table[i] = *sb;
	sb_table[i].sb_dev = dev;
}

PUBLIC super_block * get_super_block(int dev)
{
	super_block *sb = sb_table;
	for(; sb < &sb_table[NR_SUPER_BLOCK]; ++sb)
	{
		if(sb->sb_dev == dev)
		{
			return sb;
		}
	}

	panic("Super Block of Device %d not found.\n", dev);
	return 0;
}

PUBLIC inode * get_inode(int dev, int num)
{
	if(num == 0)
	{
		return 0;
	}

	inode* p;
	inode* q = 0;
	for(p = &inode_table[0]; p < &inode_table[NR_INODE]; ++p)
	{
		if(p->i_cnt)
		{
			if((p->i_dev == dev) && (p->i_num == num))
			{
				++p->i_cnt;
				return p;
			}
		}
		else
		{
			if(!q)
			{
				q = p;
			}
		}
	}

	if(!q)
	{
		panic("The inode table is full.");
	}

	q->i_dev = dev;
	q->i_num = num;
	q->i_cnt = 1;

	super_block *sb = get_super_block(dev);
	int blk_id = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects + ((num - 1) / (SECTOR_SIZE / INODE_SIZE));
	RD_SECT(dev, blk_id);

	inode* inode_ptr = (inode*)((u8*)fsbuf + (num - 1) % (SECTOR_SIZE / INODE_SIZE) * INODE_SIZE);

	q->i_mode = inode_ptr->i_mode;
	q->i_size = inode_ptr->i_size;
	q->i_start_sect = inode_ptr->i_start_sect;
	q->i_nr_sects = inode_ptr->i_nr_sects;
	return q;
}

PUBLIC void put_inode(inode* inode_ptr)
{
	assert(inode_ptr->i_cnt > 0);
	--inode_ptr->i_cnt;
}

PUBLIC void sync_inode(inode* p)
{
	inode* inode_ptr;
	super_block* sb = get_super_block(p->i_dev);
	int blk_id = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects + ((p->i_num - 1) / (SECTOR_SIZE / INODE_SIZE));
	RD_SECT(p->i_dev, blk_id);

	inode_ptr = (inode*)((u8*)fsbuf + (p->i_num - 1) % (SECTOR_SIZE / INODE_SIZE) * INODE_SIZE);

	inode_ptr->i_mode = p->i_mode;
	inode_ptr->i_size = p->i_size;
	inode_ptr->i_start_sect = p->i_start_sect;
	inode_ptr->i_nr_sects = p->i_nr_sects;
	WR_SECT(p->i_dev, blk_id);
}

PUBLIC inode * new_inode(int dev, int inode_id, int start_sect)
{
	inode* newino = get_inode(dev, inode_id);

	newino->i_mode = I_REGULAR;
	newino->i_size = 0;
	newino->i_start_sect = start_sect;
	newino->i_nr_sects = NR_DEFAULT_FILE_SECTS;

	newino->i_dev = dev;
	newino->i_cnt = 1;
	newino->i_num = inode_id;

	sync_inode(newino);

	return newino;
}

PRIVATE void mkfs()
{
	MESSAGE driver_msg;
	int i, j;

	int bits_per_sect = SECTOR_SIZE * 8;

	/* Get the Geometry of ROOTDEV */
	part_info geo;
	driver_msg.type		= DEV_IOCTL;
	driver_msg.DEVICE	= MINOR(ROOT_DEV);
	driver_msg.REQUEST	= DIOCTL_GET_GEO;
	driver_msg.BUF		= &geo;
	driver_msg.PROC_ID	= TASK_FS;
	assert(dd_map[MAJOR(ROOT_DEV)].driver_id != INVALID_DRIVER);
	send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_id, &driver_msg);

	printf("dev size: %d sectors\n", geo.size);

	/* Initial Super Block */
	super_block sb;
	sb.magic				= MAGIC_V1;
	sb.nr_inodes			= bits_per_sect;
	sb.nr_inode_sects	= sb.nr_inodes * INODE_SIZE / SECTOR_SIZE;
	sb.nr_sects			= geo.size;
	sb.nr_imap_sects		= 1;
	sb.nr_smap_sects		= sb.nr_sects / bits_per_sect + 1;
	sb.first_sect		= 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects + sb.nr_inode_sects;
	sb.root_inode		= ROOT_INODE;
	sb.inode_size		= INODE_SIZE;

	inode x;
	sb.inode_isize_off	= (int)&x.i_size - (int)&x;
	sb.inode_start_off	= (int)&x.i_start_sect - (int)&x;
	
	sb.dir_entry_size	= DIR_ENTRY_SIZE;

	dir_entry de_tmp;
	sb.dir_entry_inode_off	= (int)&de_tmp.inode_id - (int)&de_tmp;
	sb.dir_entry_fname_off	= (int)&de_tmp.name - (int)&de_tmp;

	memset(fsbuf, 0x90, SECTOR_SIZE);
	memcpy(fsbuf, &sb, SUPER_BLOCK_SIZE);

	WR_SECT(ROOT_DEV, 1);

	//RD_SECT(ROOT_DEV, 1);
	//dump(fsbuf, SUPER_BLOCK_SIZE + 4);

	printf("devbase:0x%x00, sb:0x%x00, imap:0x%x00, smap:0x%x00\n"
			"        inodes:0x%x00, 1st_sector:0x%x00\n", 
			(geo.base * 2),
			(geo.base + 1) * 2,
			(geo.base + 1 + 1) * 2,
			(geo.base + 1 + 1 + sb.nr_imap_sects) * 2,
			(geo.base + 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects) * 2,
			(geo.base + sb.first_sect) * 2);

	/* Initial Inode Map */
	memset(fsbuf, 0, SECTOR_SIZE);
	for(i = 0; i < (NR_CONSOLES + 2); ++i)
	{
		fsbuf[0] |= 1 << i;
	}
	assert(fsbuf[0] == 0x1F);

	WR_SECT(ROOT_DEV, 2);

	/* Initial Sector Map */
	memset(fsbuf, 0, SECTOR_SIZE);
	int nr_sects = NR_DEFAULT_FILE_SECTS +1;

	for(i = 0; i< nr_sects / 8; ++i)
	{
		fsbuf[i] = 0xFF;
	}
	for(j = 0; j < nr_sects % 8; ++j)
	{
		fsbuf[i] |= (1 << j);
	}
	WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects);

	memset(fsbuf, 0, SECTOR_SIZE);
	for(i = 1; i< sb.nr_smap_sects; ++i)
	{
		WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + i);
	}

	/* Initial Inode of '/' */
	memset(fsbuf, 0, SECTOR_SIZE);
	inode *inode_ptr = (inode*)fsbuf;
	inode_ptr->i_mode = I_DIRECTORY;
	inode_ptr->i_size = DIR_ENTRY_SIZE * 4;

	inode_ptr->i_start_sect = sb.first_sect;
	inode_ptr->i_nr_sects = NR_DEFAULT_FILE_SECTS;
	for(i = 0; i< NR_CONSOLES; ++i)
	{
		inode_ptr = (inode*)(fsbuf + (INODE_SIZE * (i + 1)));
		inode_ptr->i_mode = I_CHAR_SPECIAL;
		inode_ptr->i_size = 0;
		inode_ptr->i_start_sect = MAKE_DEV(DEV_CHAR_TTY, i);
		inode_ptr->i_nr_sects = 0;
	}
	WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + sb.nr_smap_sects);

	/* '/' */
	memset(fsbuf, 0, SECTOR_SIZE);
	dir_entry *de = (dir_entry*)fsbuf;

	de->inode_id = 1;
	strcpy(de->name, ".");
	
	for(i = 0; i < NR_CONSOLES; ++i)
	{
		++de;
		de->inode_id = i + 2;
		sprintf(de->name, "dev_tty%d", i);
	}
	WR_SECT(ROOT_DEV, sb.first_sect);
}

PRIVATE void fs_init()
{
	int i;

	for(i = 0; i < NR_FILE_DESC; ++i)
	{
		memset(&fd_table[i], 0, sizeof(file_desc));
	}

	for(i = 0; i < NR_INODE; ++i)
	{
		memset(&inode_table[i], 0, sizeof(inode));
	}

	super_block* sb = sb_table;
	for(; sb < &sb_table[NR_SUPER_BLOCK]; ++sb)
	{
		sb->sb_dev = NO_DEV;
	}

	MESSAGE driver_msg;

	driver_msg.type = DEV_OPEN;
	driver_msg.DEVICE = MINOR(ROOT_DEV);
	assert(dd_map[MAJOR(ROOT_DEV)].driver_id != INVALID_DRIVER);
	send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_id, &driver_msg);

	mkfs();

	read_super_block(ROOT_DEV);

	sb = get_super_block(ROOT_DEV);
	assert(sb->magic == MAGIC_V1);

	root_inode = get_inode(ROOT_DEV, ROOT_INODE);
}

PUBLIC void task_fs()
{
	printf("File System Begin.\n");

	fs_init();
	//clear_console();
	while(1)
	{
		send_recv(RECEIVE, ANY, &fs_msg);

		int src = fs_msg.source;
		fs_caller = &proc_table[src];

		//printf("fs_caller: %s\n", fs_caller->pname);

		switch(fs_msg.type)
		{
		case OPEN:
			fs_msg.FD = do_open();
			break;
		case CLOSE:
			fs_msg.RETVAL = do_close();
			break;
		case READ:
		case WRITE:
			fs_msg.CNT = do_rdwt();
			break;
		default:
			dump_msg("FS::unknown message: ",&fs_msg);
			assert(0);
			break;
		}

		fs_msg.type = SYSCALL_RET;
		send_recv(SEND, src, &fs_msg);
	}

	spin("FS");
}