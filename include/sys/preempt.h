#ifndef	VIOS_PREEMPT_H
#define	VIOS_PREEMPT_H

#include "const.h"
#include "global.h"

#define preempt_disable() (isPreemptDisabled = TRUE)

#define preempt_enable() (isPreemptDisabled = FALSE)

#endif