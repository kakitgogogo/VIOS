#ifndef	VIOS_COMPILER_H
#define	VIOS_COMPILER_H

#define barrier()		__asm__ __volatile__("":::"memory")

#define likely(x)		__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

static inline void prefetch(const void *ptr)  
{ 
	__builtin_prefetch(ptr, 0, 3);
}

static inline void prefetchw(const void *ptr)  
{
	__builtin_prefetch(ptr, 1, 3);
}

#endif