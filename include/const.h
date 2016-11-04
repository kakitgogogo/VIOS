#ifndef	VIOS_CONST_H
#define	VIOS_CONST_H

#define	EXTERN	extern

/* Function Type */
#define	PUBLIC
#define	PRIVATE	static

/* Boolean */
typedef enum
{
	FALSE,
	TRUE
}bool;

/* Color */
#define	BLACK	0x0
#define	WHITE	0x7
#define	RED		0x4
#define	GREEN	0x2
#define	BLUE	0x1
#define	FLASH	0x80
#define	BRIGHT	0x08
#define COLOR(x, y) ((x << 4) | y)

/* GDT Size and LDT Size */
#define	GDT_SIZE		128
#define	IDT_SIZE		256

/* PRIVILEGE */
#define	PRIVILEGE_KRNL 	0
#define	PRIVILEGE_TASK 	1
#define	PRIVILEGE_USER 	3

/* Request Privilege Level */
#define	RPL_KRNL			SA_RPL0
#define	RPL_TASK			SA_RPL1
#define	RPL_USER			SA_RPL3

/* 8259A */
#define	INT_M_CTL		0x20
#define	INT_M_CTLMASK	0x21
#define	INT_S_CTL		0xA0
#define	INT_S_CTLMASK	0xA1

/* 8253/8254 */
#define	TIMER0			0x40
#define	TIMER_MODE		0x43
#define	RATE_GENERATOR	0x34
#define	TIMER_FREQ		1193182L
#define	HZ				100

/* 8042 */
#define	KB_DATA			0x60
#define	KB_CMD			0x64
#define	LED_CODE			0xED
#define	KB_ACK			0xFA

/* Hardware Interrupt Request */
#define	NR_IRQ			16
#define	CLOCK_IRQ		0
#define	KEYBOARD_IRQ		1
#define	CASCADE_IRQ		2
#define	ETHER_IRQ		3
#define	SECONDARY_IRQ	3
#define	RS232_IRQ		4
#define	XI_WINI_IRQ		5
#define	FLOPPY_IRQ		6
#define	PRINTER_IRQ		7
#define	AT_WINI_IRQ		14

/* VGA */
#define	CRTC_ADDR_REG	0x3D4
#define	CRTC_DATA_REG	0x3D5
#define	START_ADDR_H		0xC
#define	START_ADDR_L		0xD
#define	CURSOR_H			0xE
#define	CURSOR_L			0xF
#define	V_MEM_BASE		0xB8000
#define	V_MEM_SIZE		0x8000

/* TTY */
#define	NR_CONSOLES		3

/* System Call */
#define	NR_SYS_CALL		2

#endif