#include "type.h"
#include "const.h"

int printf(const char *fmt, ...)
{
	int len;
	char buf[STR_DEFAULT_LEN];

	va_list arg = (va_list)((char*)(&fmt) + 4);

	len = vsprintf(buf, fmt, arg);
	buf[len] = 0;
	int wlen = write(1, buf, len);

	assert(len == wlen);

	return len;
}

int printk(const char *fmt, ...)
{
	int len;
	char buf[STR_DEFAULT_LEN];

	va_list arg = (va_list)((char*)(&fmt) + 4);

	len = vsprintf(buf, fmt, arg);
	buf[len] = 0;
	printx(buf);

	return len;
}