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


PUBLIC int send_recv(int function, int src_des, MESSAGE* msg)
{
	int ret = 0;

	if(function == RECEIVE)
	{
		memset(msg, 0, sizeof(MESSAGE));
	}

	switch(function)
	{
	case BOTH:
		ret = sendrec(SEND, src_des, msg);
		if(ret == 0)
			ret = sendrec(RECEIVE, src_des, msg);
		break;
	case SEND:
	case RECEIVE:
		ret = sendrec(function, src_des, msg);
		break;
	default:
		assert((function == BOTH) || (function == SEND) || (function == RECEIVE));
		break;
	}

	return ret;
}


PUBLIC void spin(char *func_name)
{
	printk("[SPIN] Spinning in %s ...\n", func_name);
	while(1);
}

PUBLIC void assertion_failure(char *exp, char *file, char *base_file, int line)
{
	printk("%cassert(%s) failed: file: %s, base_file: %s, line: %d\n",
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

PUBLIC int strncmp(const char* s1, const char* s2, int n)
{
	if((s1 == 0) || (s2 == 0))
	{
		return (s1 - s2);
	}

	int i;
	const char* p1 = s1;
	const char* p2 = s2;

	for(i = 0; i < n && *p1 && *p2; ++i, ++p1, ++p2)
	{
		if(*p1 != *p2)
		{
			break;
		}
	}
	if(i == n) return 0;
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