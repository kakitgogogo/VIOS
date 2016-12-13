#include "stdio.h"
#include "sys/fs.h"

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: %s file\n", argv[0]);
		return 1;
	}

	int fout = open(argv[1], O_CREAT | O_TRUNC | O_RDWR);
	if(fout == -1)
	{
		printf("Failed to open %s", argv[1]);
		return 1;
	}

	int r, w = 0;
	char rdbuf[1024];
	char* buf_ptr = rdbuf;
	printf("\n");
	while(1)
	{
		r = read(0, buf_ptr, 128);
		buf_ptr[r++] = '\n';
		if(strncmp(buf_ptr, "#", 1) == 0)
		{
			buf_ptr += 1;
			if(strncmp(buf_ptr, "save", 4) == 0)
			{
				printf("saved.\n\n", argv[0]);
				write(fout, rdbuf, w);
				break;
			}
			else if(strncmp(buf_ptr, "quit", 4) == 0)
			{
				printf("bye.\n\n", argv[0]);
				break;
			}
			buf_ptr -= 1;
		}
		buf_ptr += r;
		w += r;
	}

	close(fout);
	return 0;
}