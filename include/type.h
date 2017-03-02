#ifndef	VIOS_TYPE_H
#define	VIOS_TYPE_H

/* Function Type */
#define	PUBLIC
#define	PRIVATE	static

/* EXTERN */
#define	EXTERN	extern

/* Boolean */
typedef enum
{
	FALSE,
	TRUE
}bool;

#define	NULL ((void*)0)

typedef	unsigned long long	u64;
typedef	unsigned int			u32;
typedef	unsigned short		u16;
typedef	unsigned char		u8;

typedef signed char 		s8;
typedef signed short 		s16;
typedef signed int 			s32;
typedef signed long 		s64;

typedef	char*				va_list;

typedef	void	(*int_handler)	();
typedef	void	(*task_f)		();
typedef	void	(*irq_handler)	(int irq);

typedef	void*	system_call;

typedef struct boot_param
{
	int	mem_size;
	unsigned char* kernel_bin;
}boot_params;

#endif 
