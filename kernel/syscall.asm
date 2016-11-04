%include "sconst.inc"

GET_TICKS_ID		equ	0
WRITE_ID			equ	1
INT_VECTOR_SYS_CALL	equ	0x90

global	get_ticks
global	write

bits 32
[section .text]

;-------------------------------------------------------------------------------------
; system call get_ticks
;-------------------------------------------------------------------------------------
; void get_ticks()
;-------------------------------------------------------------------------------------
get_ticks:
	mov		eax, GET_TICKS_ID
	int		INT_VECTOR_SYS_CALL
	ret
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; system call write
;-------------------------------------------------------------------------------------
; void write(char* buf, int len);
;-------------------------------------------------------------------------------------
write:
	mov		eax, WRITE_ID
	mov		ebx, [esp + 4]
	mov		ecx, [esp + 8]
	int		INT_VECTOR_SYS_CALL
	ret
;-------------------------------------------------------------------------------------
