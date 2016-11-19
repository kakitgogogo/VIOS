%include "sconst.inc"
extern	disp_pos

[SECTION .text]

global	disp_str
global	disp_color_str
global	out_byte
global	in_byte
global	enable_irq
global	disable_irq
global	enable_int
global	disable_int
global	port_read
global	port_write
global	glitter


;-------------------------------------------------------------------------------------
; function disp_str
;-------------------------------------------------------------------------------------
; void disp_str(char *pszInfo);
;-------------------------------------------------------------------------------------
disp_str:
	push	ebp
	mov		ebp, esp

	mov		esi, [ebp + 8]
	mov		edi, [disp_pos]
	mov		ah, 0Fh
.1:
	lodsb
	test	al, al
	jz		.2
	cmp		al, 0Ah
	jnz		.3
	push	eax
	mov		eax, edi
	mov		bl, 160
	div		bl
	and		eax, 0FFh
	inc		eax
	mov		bl, 160
	mul		bl
	mov		edi, eax
	pop		eax
	jmp		.1
.3:
	mov		[gs:edi], ax
	add		edi, 2
	jmp		.1
.2:
	mov		[disp_pos], edi

	pop		ebp
	ret
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function disp_color_str
;-------------------------------------------------------------------------------------
; void disp_color_str(char * info, int color);
;-------------------------------------------------------------------------------------
disp_color_str:
	push	ebp
	mov		ebp, esp

	mov		esi, [ebp + 8]
	mov		edi, [disp_pos]
	mov		ah, [ebp + 12]
.1:
	lodsb
	test	al, al
	jz		.2
	cmp		al, 0Ah
	jnz		.3
	push	eax
	mov		eax, edi
	mov		bl, 160
	div		bl
	and		eax, 0FFh
	inc		eax
	mov		bl, 160
	mul		bl
	mov		edi, eax
	pop		eax
	jmp		.1
.3:
	mov		[gs:edi], ax
	add		edi, 2
	jmp		.1
.2:
	mov		[disp_pos], edi

	pop		ebp
	ret
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function out_byte
;-------------------------------------------------------------------------------------
; void out_byte(u16 port, u8 value);
;-------------------------------------------------------------------------------------
out_byte:
	mov		edx, [esp + 4]	
	mov		al, [esp + 4 + 4]	
	out		dx, al
	nop
	nop
	ret
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function in_byte
;-------------------------------------------------------------------------------------
; u8 in_byte(u16 port);
;-------------------------------------------------------------------------------------
in_byte:
	mov	edx, [esp + 4]		
	xor	eax, eax
	in	al, dx
	nop	
	nop
	ret
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function enable_irq
;-------------------------------------------------------------------------------------
; void enable_irq(int irq);
;-------------------------------------------------------------------------------------
enable_irq:
	mov		ecx, [esp + 4]
	pushf
	cli
	mov		ah, ~1
	rol		ah, cl

	cmp		cl, 8
	jae		.enable_slave
.enable_master:
	in		al, INT_M_CTLMASK	
	and		al, ah
	out		INT_M_CTLMASK, al

	popf
	ret
.enable_slave:
	in		al, INT_S_CTLMASK	
	and		al, ah
	out		INT_S_CTLMASK, al

	popf
	ret	
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function disable_irq
;-------------------------------------------------------------------------------------
; void disable_irq(int irq);
;-------------------------------------------------------------------------------------
disable_irq:
	mov		ecx, [esp + 4]
	pushf
	cli
	mov		ah, 1
	rol		ah, cl

	cmp		cl, 8
	jae		.disable_slave
.disable_master:
	in		al, INT_M_CTLMASK	
	or		al, ah
	out		INT_M_CTLMASK, al

	popf
	ret
.disable_slave:
	in		al, INT_S_CTLMASK	
	or		al, ah
	out		INT_S_CTLMASK, al

	popf
	ret	
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function enable_int
;-------------------------------------------------------------------------------------
; void enable_int();
;-------------------------------------------------------------------------------------
enable_int:
	sti
	ret
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function disable_int
;-------------------------------------------------------------------------------------
; void disable_int();
;-------------------------------------------------------------------------------------
disable_int:
	cli
	ret
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function port_read
;-------------------------------------------------------------------------------------
; void port_read(u16 port, void* buf, int n);
;-------------------------------------------------------------------------------------
port_read:
	mov		edx, [esp + 4]
	mov		edi, [esp + 8]
	mov		ecx, [esp + 12]
	shr		ecx, 1
	cld
	rep		insw
	ret
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function port_write
;-------------------------------------------------------------------------------------
; void port_write(u16 port, void* buf, int n);
;-------------------------------------------------------------------------------------
port_write:
	mov		edx, [esp + 4]
	mov		esi, [esp + 8]
	mov		ecx, [esp + 12]
	shr		ecx, 1
	cld
	rep		outsw
	ret
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function glitter
;-------------------------------------------------------------------------------------
; void glitter(int row, int col);
;-------------------------------------------------------------------------------------
glitter:
	push	eax
	push	ebx
	push	edx

	mov		eax, [.current_char]
	inc		eax
	cmp		eax, .strlen
	je		.1
	jmp		.2
.1:
	xor		eax, eax
.2:
	mov		[.current_char], eax
	mov	byte dl, [eax + .glitter_str]

	xor		eax, eax
	mov		al, [esp + 16]
	mov		bl, .line_width
	mul		bl
	mov		bx, [esp + 20]
	add		ax, bx
	shl		ax, 1
	movzx	eax, ax

	mov		[gs:eax], dl

	inc		eax
	mov	byte [gs:eax], 4

	jmp		.end

.current_char:	dd	0
.glitter_str:	db	'-\|/'
				db	'1234567890'
				db	'abcdefghijklmnopqrstuvwxyz'
				db	'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
.strlen			equ	$ - .glitter_str
.line_width		equ	80
.end:
	pop		edx
	pop		ebx
	pop		eax
	ret
;-------------------------------------------------------------------------------------
