LoaderBase			equ	0x9000
LoaderOffset		equ	0x100
LoaderPhyAddr		equ	LoaderBase * 0x10

KernelBinBase		equ	0x7000
KernelBinOffset		equ	0x0
KernelBinPhyAddr	equ	KernelBinBase * 0x10

KernelPhyAddr		equ	0x1000

PageDirBase			equ	100000h	
PageTblBase			equ	101000h

KERNEL_SPACE			equ	LoaderPhyAddr - KernelBinPhyAddr

; SuperBlock is stored at [0x700,0x900)
SuperBlockBase		equ	0x70

BOOT_PARAM_ADDR		equ	0x900
BOOT_PARAM_MAGIC		equ	0xB007

ROOT_BASE			equ	0x4EFF

; Corresponding with include/sys/fs.h
SB_MAGIC_V1			equ	0x111
SB_MAGIC				equ	4 *  0
SB_NR_INODES			equ	4 *  1
SB_NR_SECTS			equ	4 *  2
SB_NR_IMAP_SECTS		equ	4 *  3
SB_NR_SMAP_SECTS		equ	4 *  4
SB_FIRST_SECT		equ	4 *  5
SB_NR_INODE_SECTS	equ	4 *  6
SB_ROOT_INODE		equ	4 *  7
SB_INODE_SIZE		equ	4 *  8
SB_INODE_ISIZE_OFF	equ	4 *  9
SB_INODE_START_OFF	equ	4 * 10
SB_DIR_ENT_SIZE		equ	4 * 11
SB_DIR_ENT_INODE_OFF	equ	4 * 12
SB_DIR_ENT_FNAME_OFF	equ	4 * 13