#include "stdio.h"
#include "sys/fs.h"

int main()
{
	int fin = open("/", O_RDWR);
	char rdbuf[128];
	dir_entry* entry;

	do
	{
		int r = read(fin, rdbuf, sizeof(dir_entry));
		entry = (dir_entry*)rdbuf;
		if(entry->inode_id != 0)
		{
			printf("%s\n", entry->name);
		}
	}
	while(entry->inode_id);

	close(fin);
	return 0;
}