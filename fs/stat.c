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

PUBLIC int do_stat()
{
	char pathname[MAX_PATH];
	char filename[MAX_PATH];

	int name_len = fs_msg.NAME_LEN;
	int src = fs_msg.source;
	assert(name_len < MAX_PATH);
	memcpy((void*)va2la(TASK_FS, pathname), (void*)va2la(src, fs_msg.PATHNAME), name_len);
	pathname[name_len] = 0;

	int inode_id = search_file(pathname);
	if(inode_id == INVALID_INODE)
	{
		printk("[FS] do_stat() : search_file() returns invalid inode: %s\n", pathname);
		return -1;
	}

	inode* inode_ptr = 0;
	inode* dir_inode;
	assert(strip_path(filename, pathname, &dir_inode) == 0);

	inode_ptr = get_inode(dir_inode->i_dev, inode_id);

	stat_t s;
	s.st_dev = inode_ptr->i_dev;
	s.st_ino = inode_ptr->i_num;
	s.st_mode = inode_ptr->i_mode;
	s.st_rdev = is_special(inode_ptr->i_mode) ? inode_ptr->i_start_sect : NO_DEV;
	s.st_size = inode_ptr->i_size;

	dec_inode(inode_ptr);

	memcpy((void*)va2la(src, fs_msg.BUF), (void*)va2la(TASK_FS, &s), sizeof(stat_t));

	return 0;
}