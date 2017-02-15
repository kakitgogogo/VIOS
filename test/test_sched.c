#include "list.h"
#include "const.h"
#include "type.h"
#include "protect.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
#include "proc.h"
#include "global.h"
#include "bitmap.h"
#include "list.h"
#include "sched.h"

int test_sched()
{
	bitmap_view(&runqueue.active->bm);
}