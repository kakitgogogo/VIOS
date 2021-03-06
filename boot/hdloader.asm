org  0100h

	jmp	start

%include	"load.inc"
%include	"pm.inc"

BaseOfStack	equ	0100h
TRANS_SECT_NR	equ	2
SECT_BUF_SIZE	equ	TRANS_SECT_NR * 512

disk_address_packet:
	db		0x10
	db		0
sect_cnt:
	db		TRANS_SECT_NR
	db		0
	dw		KernelBinOff		; Destination Address Offset
	dw		KernelBinSeg		; Destination Address Segment
lba_addr:
	dd		0					; LBA Low 32bits
	dd		0					; LBA High 32bits

;-------------------------------------------------------------------------------------
; GDT
;-------------------------------------------------------------------------------------
LABEL_GDT:				
	Descriptor		0,		0,		0
LABEL_DESC_FLAT_C:		
	Descriptor		0,		0fffffh, DA_CR  | DA_32 | DA_LIMIT_4K
LABEL_DESC_FLAT_RW:		
	Descriptor		0,		0fffffh,	DA_DRW | DA_32 | DA_LIMIT_4K
LABEL_DESC_VIDEO:		
	Descriptor		0B8000h,	0ffffh,	DA_DRW | DA_DPL3
;-------------------------------------------------------------------------------------
GdtLen		equ	$ - LABEL_GDT
GdtPtr		dw	GdtLen - 1
			dd	LoaderPhyAddr + LABEL_GDT
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; GDT Selector
;-------------------------------------------------------------------------------------
SelectorFlatC	equ	LABEL_DESC_FLAT_C	- LABEL_GDT
SelectorFlatRW	equ	LABEL_DESC_FLAT_RW	- LABEL_GDT
SelectorVideo	equ	LABEL_DESC_VIDEO		- LABEL_GDT + SA_RPL3
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; Start Loading
;-------------------------------------------------------------------------------------
start:	
	mov		ax, cs
	mov		ds, ax
	mov		es, ax
	mov		ss, ax
	mov		sp, BaseOfStack

; show "Loading..."
	mov		dh, 0
	call	disp_str

; get memory information
	mov		ebx, 0
	mov		di, MemBuf16
.loop:
	mov		eax, 0E820h	
	mov		ecx, 20		
	mov		edx, 0534D4150h	
	int		15h	

	jc		.fail
	add		di, 20
	inc	dword [MCRNumber16]

	cmp		ebx, 0
	jne		.loop
	jmp		.MemDone
.fail:
	mov	dword [MCRNumber16], 0

.MemDone:

	; get the sector_id of `/' (ROOT_INODE)
	mov		eax, [fs:SB_ROOT_INODE]
	call	get_inode

	mov	dword [disk_address_packet + 8], eax
	call	read_sector

; search hdloader.bin
	mov		si, kernel
	push	bx		; <- save
.str_cmp:
	add		bx, [fs:SB_DIR_ENT_FNAME_OFF]
.chr_cmp:
	lodsb	; ds:si -> al
	cmp		al, byte [es:bx]
	jz		.same
	jmp		.diff
.same:
	cmp		al, 0
	jz		.found
	inc		bx
	jmp		.chr_cmp
.diff:
	pop		bx
	add		bx, [fs:SB_DIR_ENT_SIZE]
	sub		ecx, [fs:SB_DIR_ENT_SIZE]
	jz		.not_found

	push	bx
	mov		si, kernel
	jmp		.str_cmp
.not_found:
	mov		dh, 2
	call	disp_str
	jmp		$
.found:
	pop		bx
	add		bx, [fs:SB_DIR_ENT_INODE_OFF]
	mov		eax, [es:bx]
	call	get_inode
	mov	dword [disk_address_packet + 8], eax
.load_kernel:
	call	read_sector
	cmp		ecx, SECT_BUF_SIZE
	jl		.done
	sub		ecx, SECT_BUF_SIZE
	add	word [disk_address_packet + 4], SECT_BUF_SIZE
	jc		.upper64k
	jmp		.jmp_load
.upper64k:
	add	word [disk_address_packet + 6], 1000h	; large than 64k(4k * 16), so seg + 4k
.jmp_load:
	add	dword [disk_address_packet + 8], TRANS_SECT_NR
	jmp		.load_kernel
.done:
	mov		dh, 1
	call	disp_str
	
; jump to protect mode
.jmp_pm:
	lgdt	[GdtPtr]

	cli

	in		al, 92h
	or		al, 00000010b
	out		92h, al

	mov		eax, cr0
	or		eax, 1
	mov		cr0, eax

	jmp	dword SelectorFlatC:(LoaderPhyAddr+start32)
	jmp		$
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; data
;-------------------------------------------------------------------------------------
kernel		db	"kernel.bin", 0
msgLen		equ	10
msg0		db	"Loading..."	
msg1		db	"Done      "	
msg2		db	"No KERNEL."	
msg3		db	"Error 0   "
msg4		db	"Too Large."

row			equ	3
;-------------------------------------------------------------------------------------

%include "hdutils.inc"

;-------------------------------------------------------------------------------------
; function  killMotor
;-------------------------------------------------------------------------------------
killMotor:
	push	dx
	mov		dx, 03F2h
	mov		al, 0
	out		dx, al
	pop		dx
	ret
;-------------------------------------------------------------------------------------

;=================PROTECT MODE==================

;-------------------------------------------------------------------------------------
; 32bits code
;-------------------------------------------------------------------------------------
[SECTION .s32]

ALIGN	32

[BITS	32]

start32:
	mov		ax, SelectorVideo
	mov		gs, ax
	mov		ax, SelectorFlatRW
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		ss, ax
	mov		esp, TopOfStack

	push	Title
	call	displayStr32
	add		esp, 4

	call	displayMemInfo
	call	setupPaging

	call	initKernel

	mov	dword [BOOT_PARAM_ADDR], BOOT_PARAM_MAGIC
	mov		eax, [MemSize]
	mov		[BOOT_PARAM_ADDR + 4], eax
	mov		eax, KernelBinSeg
	shl		eax, 4
	add		eax, KernelBinOff
	mov		[BOOT_PARAM_ADDR + 8], eax

	jmp		SelectorFlatC:KernelPhyAddr
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; function displayAL
;-------------------------------------------------------------------------------------
displayAL:
	push	ecx
	push	edx
	push	edi

	mov		edi, [DispPos]

	mov		ah, 0Fh
	mov		dl, al
	shr		al, 4
	mov		ecx, 2
.begin:
	and		al, 01111b
	cmp		al, 9
	ja		.1
	add		al, '0'
	jmp		.2
.1:
	sub		al, 0Ah
	add		al, 'A'
.2:
	mov		[gs:edi], ax
	add		edi, 2

	mov		al, dl
	loop	.begin
	;add		edi, 2

	mov		[DispPos], edi

	pop		edi
	pop		edx
	pop		ecx

	ret
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; function displayInt
;-------------------------------------------------------------------------------------
displayInt:
	mov		eax, [esp + 4]
	shr		eax, 24
	call	displayAL

	mov		eax, [esp + 4]
	shr		eax, 16
	call	displayAL

	mov		eax, [esp + 4]
	shr		eax, 8
	call	displayAL

	mov		eax, [esp + 4]
	call	displayAL

	mov		bh, 0Fh
	push	edi
	mov		edi, [DispPos]
	mov		[gs:edi], bx
	add		edi, 4
	mov		[DispPos], edi
	pop		edi

	ret
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; function displayStr32
;-------------------------------------------------------------------------------------
displayStr32:
	push	ebp
	mov		ebp, esp
	push	ebx
	push	esi
	push	edi

	mov		esi, [ebp + 8]	
	mov		edi, [DispPos]
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
	mov		[DispPos], edi

	pop		edi
	pop		esi
	pop		ebx
	pop		ebp
	ret
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; function endline
;-------------------------------------------------------------------------------------
endline:
	push	Endl
	call	displayStr32
	add		esp, 4

	ret
;-------------------------------------------------------------------------------------

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
; function displayMemInfo
;-------------------------------------------------------------------------------------
displayMemInfo:
	push	esi
	push	edi
	push	ecx
	push	ebx

	mov		esi, MemBuf
	mov		ecx, [MCRNumber]
.loop:	
	mov		edx, 5			
	mov		edi, ARDStruct	
.1:				
	push	dword [esi]
	mov		bl, 'H'	
	call	displayInt	
	pop		eax		
	stosd	

	add		esi, 4	
	dec		edx	
	cmp		edx, 0	
	jnz		.1	

	call	endline	
	cmp		dword [Type], 1
	jne		.2	

	mov		eax, [BaseAddrLow]	
	add		eax, [LengthLow]
	cmp		eax, [MemSize]
	jb		.2	
	mov		[MemSize], eax	
.2:	
	loop	.loop	

	call	endline	
	push	RAMSize	
	call	displayStr32	
	add		esp, 4		

	mov		eax, [MemSize]
	shr		eax, 20
	push	eax
	mov 		bl, 'M'
	call	displayInt	
	add		esp, 4	

	pop		ebx
	pop		ecx
	pop		edi
	pop		esi
	ret
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; function setupPaging
;-------------------------------------------------------------------------------------
setupPaging:
	xor		edx, edx
	mov		eax, [MemSize]
	mov		ebx, 400000h
	div		ebx
	mov		ecx, eax
	test	edx, edx
	jz		.no_remainder
	inc		ecx	
.no_remainder:
	push	ecx	

; initial PDB
	mov		ax, SelectorFlatRW
	mov		es, ax
	mov		edi, PageDirBase		
	xor		eax, eax
	mov		eax, PageTblBase | PG_P  | PG_USU | PG_RWW
.1:
	stosd
	add		eax, 4096		
	loop	.1

; initial PTB	
	pop		eax	
	mov		ebx, 1024
	mul		ebx
	mov		ecx, eax	
	mov		edi, PageTblBase	
	xor		eax, eax
	mov		eax, PG_P | PG_USU | PG_RWW
.2:
	stosd
	add		eax, 4096
	loop	.2

	mov		eax, PageDirBase
	mov		cr3, eax
	mov		eax, cr0
	or		eax, 80000000h
	mov		cr0, eax
	jmp		short .3
.3:
	nop

	ret
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; function initKernel
;-------------------------------------------------------------------------------------
initKernel:	
	xor		esi, esi
	mov		cx, word [KernelBinPhyAddr + 2Ch]  	;e_phnum
	movzx	ecx, cx	
	mov		esi, [KernelBinPhyAddr + 1Ch]		;e_phoff
	add		esi, KernelBinPhyAddr
.begin:
	mov		eax, [esi + 0]
	cmp		eax, 0			
	jz		.skip
	
	push	dword [esi + 010h]		
	mov		eax, [esi + 04h]		
	add		eax, KernelBinPhyAddr
	push	eax	
	push	dword [esi + 08h]	
	call	memcpy	
	add		esp, 12	
.skip:
	add		esi, 020h	
	dec		ecx
	jnz		.begin

	ret
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; SECTION .data1
;-------------------------------------------------------------------------------------
[SECTION .data1]

ALIGN	32

LABEL_DATA:
; 16 bits
Title16:				db	"BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
RAMSize16:			db	"RAM size:", 0
Endl16:				db	0Ah, 0

MCRNumber16:			dd	0	
DispPos16:			dd	(80 * 6 + 0) * 2
MemSize16:			dd	0

ARDStruct16:		
	BaseAddrLow16:	dd	0
	BaseAddrHigh16:	dd	0
	LengthLow16:		dd	0
	LengthHigh16:	dd	0
	Type16:			dd	0

MemBuf16:	times	1024	db	0

; 32 bits
Title				equ	LoaderPhyAddr + Title16
RAMSize				equ	LoaderPhyAddr + RAMSize16
Endl				equ	LoaderPhyAddr + Endl16

MCRNumber			equ	LoaderPhyAddr + MCRNumber16
DispPos				equ	LoaderPhyAddr + DispPos16
MemSize				equ	LoaderPhyAddr + MemSize16

ARDStruct			equ	LoaderPhyAddr + ARDStruct16
	BaseAddrLow		equ	LoaderPhyAddr + BaseAddrLow16
	BaseAddrHigh	equ	LoaderPhyAddr + BaseAddrHigh16
	LengthLow		equ	LoaderPhyAddr + LengthLow16
	LengthHigh		equ	LoaderPhyAddr + LengthHigh16
	Type			equ	LoaderPhyAddr + Type16

MemBuf				equ	LoaderPhyAddr + MemBuf16

StackSpace:	times	1000h	db	0
TopOfStack	equ	LoaderPhyAddr + $	
;-------------------------------------------------------------------------------------
