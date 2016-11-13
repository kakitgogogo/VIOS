#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
#include "global.h"
#include "proto.h"

PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printf("%c !!panic!! %s", MAG_CH_PANIC, buf);

	__asm__ __volatile__("ud2");
}