#include "config.h"
#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
#include "hd.h"
#include "global.h"
#include "proto.h"

PUBLIC int rw_sector(int io_type, int dev, u64 pos, int bytes, int proc_id, void* buf)
{
	MESSAGE driver_msg;

	driver_msg.type		= io_type;
	driver_msg.DEVICE	= MINOR(dev);
	driver_msg.POSITION	= pos;
	driver_msg.BUF		= buf;
	driver_msg.CNT		= bytes;
	driver_msg.PROC_ID	= proc_id;

	assert(dd_map[MAJOR(dev)].driver_id != INVALID_DRIVER);
	send_recv(BOTH, dd_map[MAJOR(dev)].driver_id, &driver_msg);

	return 0;
}

PRIVATE void mkfs()
{
	MESSAGE driver_msg;
	int i, j;

	int bits_per_sect = SECTOR_SIZE * 8;

	/* Get the Geometry of ROOTDEV */
	struct part_info geo;
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

	dir_entry de;
	sb.dir_entry_inode_off	= (int)&de.inode_id - (int)&de;
	sb.dir_entry_fname_off	= (int)&de.name - (int)&de;

	memset(fsbuf, 0x90, SECTOR_SIZE);
	memcpy(fsbuf, &sb, SUPER_BLOCK_SIZE);

	WR_SECT(ROOT_DEV, 1);

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
	inode *pi = (inode*)fsbuf;
	pi->i_mode = I_DIRECTORY;
	pi->i_size = DIR_ENTRY_SIZE * 4;

	pi->i_start_sect = sb.first_sect;
	pi->i_nr_sects = NR_DEFAULT_FILE_SECTS;
	for(i = 0; i< NR_CONSOLES; ++i)
	{
		pi = (inode*)(fsbuf + (INODE_SIZE * (i + 1)));
		pi->i_mode = I_CHAR_SPECIAL;
		pi->i_size = 0;
		pi->i_start_sect = MAKE_DEV(DEV_CHAR_TTY, i);
		pi->i_nr_sects = 0;
	}
	WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + sb.nr_smap_sects);

	/* '/' */
	memset(fsbuf, 0, SECTOR_SIZE);
	dir_entry *pde = (dir_entry*)fsbuf;

	pde->inode_id = 1;
	strcpy(pde->name, ".");

	for(i = 0; i < NR_CONSOLES; ++i)
	{
		++pde;
		pde->inode_id = i + 2;
		sprintf(pde->name, "dev_tty%d", i);
	}
	WR_SECT(ROOT_DEV, sb.first_sect);

}

PRIVATE void fs_init()
{
	MESSAGE driver_msg;

	driver_msg.type = DEV_OPEN;
	driver_msg.DEVICE = MINOR(ROOT_DEV);
	assert(dd_map[MAJOR(ROOT_DEV)].driver_id != INVALID_DRIVER);
	send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_id, &driver_msg);

	mkfs();
}

PUBLIC void task_fs()
{
	printf("File System Begin.\n");
	
	fs_init();

	spin("FS");
}