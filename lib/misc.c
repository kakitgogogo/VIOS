#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
#include "keyboard.h"

PUBLIC void spin(char *func_name)
{
	printf("\nspinning in %s ...\n", func_name);
	while(1);
}

PUBLIC void assertion_failure(char *exp, char *file, char *base_file, int line)
{
	printf("%cassert(%s) failed: file: %s, base_file: %s, line: %d",
			MAG_CH_ASSERT, exp, file, base_file, line);

	spin("assertion_failure()");

	__asm__ __volatile__("ud2");
}