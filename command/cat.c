#include "stdio.h"
#include "sys/fs.h"

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: %s file\n", argv[0]);
		return 1;
	}
	int fin = open(argv[1], O_RDWR);
	if(fin == -1)
	{
		printf("Failed to open %s", argv[1]);
	}
	char rdbuf[128];

	do
	{
		int r = read(fin, rdbuf, 128);
		printf("%s\n", rdbuf);
	}
	while(0);

	close(fin);
	return 0;
}