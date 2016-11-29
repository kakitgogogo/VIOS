#include "type.h"
#include "const.h"
#include "string.h"

PRIVATE char* i2a(int val, int base, char **ps)
{
	int m = val % base;
	int q = val / base;
	if(q)
	{
		i2a(q, base, ps);
	}
	*(*ps)++ = (m < 10) ? (m + '0') : (m - 10 + 'A');

	return *ps;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
	char* p;
	char tmp[STR_DEFAULT_LEN];
	char cs;
	int align_nr;
	int num;

	va_list next_arg = args;

	for(p = buf; *fmt; ++fmt)
	{
		if(*fmt != '%')
		{
			*p++ = *fmt;
			continue;
		}
		else
		{
			align_nr = 0;
		}
		++fmt;

		if(*fmt == '%')
		{
			*p++ = *fmt;
			continue;
		}
		else if(*fmt == '0')
		{
			cs = '0';
			++fmt;
		}
		else
		{
			cs = ' ';
		}
		while(((unsigned char)(*fmt) >= '0')&&((unsigned char)(*fmt) <= '9'))
		{
			align_nr *= 10;
			align_nr += *fmt - '0';
			++fmt;
		}

		char *q = tmp;
		memset(q, 0, sizeof(tmp));

		switch(*fmt)
		{
		case 'c':
			*q++ = *((char*)next_arg);
			next_arg += 4;
			break;
		case 'x':
			num = *((int*)next_arg);
			i2a(num, 16, &q);
			next_arg += 4;
			break;
		case 'd':
			num = *((int*)next_arg);
			if(num < 0)
			{
				num *= (-1);
				*q++ = '-';
			}
			i2a(num, 10, &q);
			next_arg += 4;
			break;
		case 's':
			strcpy(q, (*((char**)next_arg)));
			q += strlen(*((char**)next_arg));
			next_arg += 4;
			break;
		default:
			break;
		}

		int k;
		int len = ((align_nr > strlen(tmp)) ? (align_nr - strlen(tmp)) : 0);
		for(k = 0; k < len; ++k)
		{
			*p++ = cs;
		}
		q = tmp;
		while(*q)
		{
			*p++ = *q++;
		}
	}
	*p = 0;
	return (p - buf);
}

PUBLIC int sprintf(char* buf, const char *fmt, ...)
{
	va_list arg = (va_list)((char*)(&fmt) + 4);
	return vsprintf(buf, fmt, arg);
}