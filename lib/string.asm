[SECTION .text]

global	memcpy
global	memset
global	strcpy
global	strlen

global	strncpy

;-------------------------------------------------------------------------------------
; function memcpy
;-------------------------------------------------------------------------------------
; void* memcpy(void* des, void* src, int size);
;-------------------------------------------------------------------------------------
memcpy:
	push	ebp
	mov		ebp, esp

	push	esi
	push	edi
	push	ecx

	mov		edi, [ebp + 8]	
	mov		esi, [ebp + 12]
	mov		ecx, [ebp + 16]
.1:
	cmp		ecx, 0
	jz		.2	

	mov		al, [ds:esi]	
	inc		esi			
					
	mov		byte [es:edi], al	
	inc		edi	

	dec		ecx	
	jmp		.1	
.2:
	mov		eax, [ebp + 8]	

	pop		ecx
	pop		edi
	pop		esi
	mov		esp, ebp
	pop		ebp

	ret	
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; function memset
;-------------------------------------------------------------------------------------
; void* memset(void* des, char ch, int size);
;-------------------------------------------------------------------------------------
memset:
	push	ebp
	mov		ebp, esp

	push	esi
	push	edi
	push	ecx

	mov		edi, [ebp + 8]
	mov		edx, [ebp + 12]
	mov		ecx, [ebp + 16]
.1:
	cmp		ecx, 0
	jz		.2

	mov	byte [edi], dl
	inc		edi

	dec		ecx
	jmp		.1
.2:
	pop		ecx
	pop		edi
	pop		esi
	mov		esp, ebp
	pop		ebp

	ret
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function strcpy
;-------------------------------------------------------------------------------------
; char* strcpy(char* dst, char* src);
;-------------------------------------------------------------------------------------
strcpy:
	push	ebp
	mov		ebp, esp

	mov		edi, [ebp + 8]
	mov		esi, [ebp + 12]
.1:
	mov		al, [esi]
	inc		esi

	mov	byte [edi], al
	inc		edi

	cmp		al, 0
	jnz		.1

	mov		eax, [ebp + 8]

	pop		ebp
	ret
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; function strlen
;-------------------------------------------------------------------------------------
; int* strlen(char* str);
;-------------------------------------------------------------------------------------
strlen:
	push	ebp
	mov		ebp, esp

	mov		eax, 0
	mov		esi, [ebp + 8]
.1:
	cmp	byte [esi], 0
	jz		.2

	inc		esi
	inc		eax
	jmp		.1
.2:
	pop		ebp
	ret
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function strncpy
;-------------------------------------------------------------------------------------
; char* strncpy(char* dst, char* src, int n);
;-------------------------------------------------------------------------------------
strncpy:
	push	ebp
	mov		ebp, esp

	mov		edi, [ebp + 8]
	mov		esi, [ebp + 12]
	mov		ecx, [ebp + 16]
.1:
	mov		al, [esi]
	inc		esi

	mov	byte [edi], al
	inc		edi

	cmp		al, 0
	jnz		.1

	loop	.1

	mov		eax, [ebp + 8]

	pop		ebp
	ret
;-------------------------------------------------------------------------------------
