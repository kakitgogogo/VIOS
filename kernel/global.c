#define GLOBAL_VARIABLE_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

PUBLIC TASK task_table[NR_TASKS] = {
	{testA, STACK_SIZE_TESTA, "testA"},
	{testB, STACK_SIZE_TESTB, "testB"}
};