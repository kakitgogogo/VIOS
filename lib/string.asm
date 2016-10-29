[SECTION .text]

global	memcpy
global	memset

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
.2
	pop		ecx
	pop		edi
	pop		esi
	mov		esp, ebp
	pop		ebp

	ret
;-------------------------------------------------------------------------------------