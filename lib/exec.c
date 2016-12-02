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
#include "stdio.h"

PUBLIC int exec(const char* pathname)
{
	MESSAGE msg;
	msg.type = EXEC;
	msg.PATHNAME = (void*)pathname;
	msg.NAME_LEN = strlen(pathname);
	msg.BUF = 0;
	msg.BUF_LEN = 0;

	send_recv(BOTH, TASK_MM, &msg);
	assert(msg.type == SYSCALL_RET);

	return msg.RETVAL;
}

PUBLIC int execl(const char* pathname, const char* arg, ...)
{
	va_list arg_ptr = (va_list)(&arg);
	char** p = (char**)arg_ptr;
	return execv(pathname, p);
}

PUBLIC int execv(const char* pathname, char* argv[])
{
	char** p = argv;
	char arg_stack[PROC_ORIGIN_STACK];
	int stack_len = 0;

	while(*p++)
	{
		assert(stack_len + 2 * sizeof(char*) < PROC_ORIGIN_STACK);
		stack_len += sizeof(char*);
	}

	*((int*)(&arg_stack[stack_len])) = 0;
	stack_len += sizeof(char*);

	char** q = (char**)arg_stack;
	for(p = argv; *p != 0; ++p)
	{
		*q++ = &arg_stack[stack_len];

		assert(stack_len + strlen(*p) + 1 < PROC_ORIGIN_STACK);
		strcpy(&arg_stack[stack_len], *p);
		stack_len += strlen(*p);
		arg_stack[stack_len] = 0;
		++stack_len;		
	}

	MESSAGE msg;
	msg.type = EXEC;
	msg.PATHNAME = (void*)pathname;
	msg.NAME_LEN = strlen(pathname);
	msg.BUF = (void*)arg_stack;
	msg.BUF_LEN = stack_len;

	send_recv(BOTH, TASK_MM, &msg);
	assert(msg.type == SYSCALL_RET);

	return msg.RETVAL;
}