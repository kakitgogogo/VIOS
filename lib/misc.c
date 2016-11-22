#include "const.h"
#include "type.h"
#include "protect.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include "keyboard.h"
#include "stdio.h"

PUBLIC void spin(char *func_name)
{
	printk("\nSpinning in %s ...\n", func_name);
	while(1);
}

PUBLIC void assertion_failure(char *exp, char *file, char *base_file, int line)
{
	printk("%cassert(%s) failed: file: %s, base_file: %s, line: %d",
			MAG_CH_ASSERT, exp, file, base_file, line);

	spin("assertion_failure()");

	__asm__ __volatile__("ud2");
}

PUBLIC int memcmp(const void* s1, const void *s2, int n)
{
	if((s1 == 0) || (s2 == 0))
	{
		return (s1 -s2);
	}

	const char* p1 = (const char*)s1;
	const char* p2 = (const char*)s2;
	int i;
	for(i = 0; i < n; ++i, ++p1, ++p2)
	{
		if(*p1 != *p2)
		{
			return (*p1 - *p2);
		}
	}
	return 0;
}

PUBLIC int strcmp(const char* s1, const char* s2)
{
	if((s1 == 0) || (s2 == 0))
	{
		return (s1 - s2);
	}

	const char* p1 = s1;
	const char* p2 = s2;

	for(; *p1 && *p2; ++p1, ++p2)
	{
		if(*p1 != *p2)
		{
			break;
		}
	}
	return (*p1 - *p2);
}

PUBLIC char* strcat(char *s1, const char *s2)
{
	if((s1 == 0) || (s2 == 0))
	{
		return 0;
	}

	char *p1 = s1;
	for(; *p1; ++p1);

	const char *p2 = s2;
	for(; *p2; ++p1, ++p2)
	{
		*p1 = *p2;
	}
	*p1 = 0;

	return s1;
}