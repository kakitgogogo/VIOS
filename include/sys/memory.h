#ifndef VIOS_KERNEL_H
#define VIOS_KERNEL_H

typedef enum 
{
	KERNEL_POOL = 1,
	USER_POOL = 2
}pool_flag;

#define	PG_P_1	1	// 页表项或页目录项存在属性位
#define	PG_P_0	0	// 页表项或页目录项存在属性位
#define	PG_RW_R	0	// R/W 属性位值, 读/执行
#define	PG_RW_W	2	// R/W 属性位值, 读/写/执行
#define	PG_US_S	0	// U/S 属性位值, 系统级
#define	PG_US_U	4	// U/S 属性位值, 用户级

typedef struct pool
{
	bitmap pool_bitmap;
	u32 phy_addr_start;
	u32 pool_size;
}pool;

typedef struct virtual_addr
{
	bitmap vaddr_bitmap;
	u32 vaddr_start;
}vaddr;

extern struct pool kernel_pool, user_pool;
PUBLIC void	mem_init(void);
PUBLIC void*	get_kernel_pages(u32 pg_cnt);
PUBLIC void*	malloc_page(pool_flag pf, u32 pg_cnt);
PUBLIC void	malloc_init(void);
PUBLIC u32*	pte_ptr(u32 vaddr);
PUBLIC u32*	pde_ptr(u32 vaddr);

#endif