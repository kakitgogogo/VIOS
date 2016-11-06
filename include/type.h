#ifndef	VISO_TYPE_H
#define	VIOS_TYPE_H

typedef	unsigned long long	u64;
typedef	unsigned int			u32;
typedef	unsigned short		u16;
typedef	unsigned char		u8;

typedef	char*				va_list;

typedef	void	(*int_handler)	();
typedef void		(*task_f)		();
typedef	void	(*irq_handler)	(int irq);

typedef	void*	system_call;

#endif 
