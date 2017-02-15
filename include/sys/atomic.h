#ifndef	VIOS_ATOMIC_H
#define	VIOS_ATOMIC_H

#define LOCK "lock ; "

typedef struct { volatile int counter; } atomic_t;

#define	atomic_init(i) {(i)}

#define	atomic_get(v) ((v)->counter)

#define	atomic_set(v, i) (((v)->counter) = (i))

static __inline__ void atomic_add(atomic_t *v, int i)
{
	__asm__ __volatile__(
		LOCK "addl %1,%0"
		:"=m" (v->counter)
		:"ir" (i), "m" (v->counter));
}

static __inline__ void atomic_sub(atomic_t *v, int i)
{
	__asm__ __volatile__(
		LOCK "subl %1,%0"
		:"=m" (v->counter)
		:"ir" (i), "m" (v->counter));
}

static __inline__ void atomic_inc(atomic_t *v)
{
	__asm__ __volatile__(
		LOCK "incl %0"
		:"=m" (v->counter)
		:"m" (v->counter));
}

static __inline__ void atomic_dec(atomic_t *v)
{
	__asm__ __volatile__(
		LOCK "decl %0"
		:"=m" (v->counter)
		:"m" (v->counter));
}

#endif