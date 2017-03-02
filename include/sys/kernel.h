#ifndef	VIOS_KERNEL_H
#define	VIOS_KERNEL_H

#include "stddef.h"

#define container_of(ptr, type, member) ({ \
        const typeof( ((type *)0)->member ) *__mptr = (ptr); \
        (type *)( (char *)__mptr - offsetof(type, member) );})

#endif