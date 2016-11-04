#include "type.h"
#include "const.h"

int printf(const char *fmt, ...)
{
	int len;
	char buf[256];

	va_list arg = (va_list)((char*)(&fmt) + 4);

	len = vsprintf(buf, fmt, arg);
	write(buf, len);

	return len;
}