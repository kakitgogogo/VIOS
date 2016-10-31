%include "sconst.inc"

GET_TICKS_ID		equ	0
INT_VECTOR_SYS_CALL	equ	0x90

global	get_ticks

bits 32
[section .text]

;-------------------------------------------------------------------------------------
; function get_ticks
;-------------------------------------------------------------------------------------
get_ticks:
	mov		eax, GET_TICKS_ID
	int		INT_VECTOR_SYS_CALL
	ret