#ifndef	VIOS_FS_H
#define	VIOS_FS_H

typedef struct dev_drv_map
{
	int driver_id;
}DRIVER;

/* MAGIC_V1 */
#define	MAGIC_V1		0x111

/* Super Block */
typedef struct super_block
{
	u32	magic;
	u32	nr_inodes;
	u32	nr_sects;
	u32	nr_imap_sects;
	u32	nr_smap_sects;
	u32	first_sect;

	u32	nr_inode_sects;
	u32	root_inode;
	u32	inode_size;
	u32	inode_isize_off;
	u32	inode_start_off;
	u32	dir_entry_size;
	u32	dir_entry_inode_off;
	u32	dir_entry_fname_off;

	int	sb_dev;
}super_block;

#define	SUPER_BLOCK_SIZE		56

/* inode */
typedef struct inode
{
	u32	i_mode;
	u32	i_size;
	u32	i_start_sect;
	u32	i_nr_sects;
	u8	_unused[16];

	int	i_dev;
	int	i_cnt;
	int	i_num;
}inode;

#define	INODE_SIZE		32

/* Directory Entry */
#define	MAX_FILENAME_LEN		12

typedef struct dir_entry
{
	int inode_id;
	char name[MAX_FILENAME_LEN];
}dir_entry;

#define	DIR_ENTRY_SIZE	sizeof(dir_entry)

/* File Descriptor */
typedef struct file_desc
{
	int	fd_mode;
	int	fd_pos;
	inode* fd_inode;
}file_desc;


/* Macro about Sector Read and Write */
#define RD_SECT(dev, sect_id) rw_sector(DEV_READ, \
							dev, \
							(sect_id) * SECTOR_SIZE, \
							SECTOR_SIZE, \
							TASK_FS, \
							fsbuf);

#define WR_SECT(dev, sect_id) rw_sector(DEV_WRITE, \
							dev, \
							(sect_id) * SECTOR_SIZE, \
							SECTOR_SIZE, \
							TASK_FS, \
							fsbuf);

#endif