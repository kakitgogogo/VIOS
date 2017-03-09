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

PRIVATE int turn;
PRIVATE bool interestd[2] = {FALSE, FALSE};

PUBLIC void enterRegion(int process)
{
	int other = 1 - process;

	interestd[process] = TRUE;

	turn = process;

	while(turn == process && interestd[other] == TRUE);
}

PUBLIC void leaveRegion(int process)
{
	interestd[process] = FALSE;
}

/* Reader and Writer Question */

typedef int semaphore;

semaphore mutex = 1;
semaphore db = 1;
int rc = 0;

PRIVATE void down(semaphore sem)
{
	while(sem <= 0);
	--sem;
}

PRIVATE void up(semaphore sem)
{
	++sem;
}

PUBLIC void reader(event read_data, event use_data)
{
	while(TRUE)
	{
		down(&mutex);
		rc++;
		if(rc == 1) down(&db);
		up(&mutex);
		read_data();
		down(&mutex);
		rc--;
		if(rc == 0) up(&db);
		up(&mutex);
		use_data();
	}
}

PUBLIC void writer(event create_data, event write_data)
{
	while(TRUE)
	{
		create_data();
		down(&db);
		write_data();
		up(&db);
	}
}