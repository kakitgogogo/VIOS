org  0100h

	jmp	start

%include	"fat12.inc"
%include	"load.inc"
%include	"pm.inc"

BaseOfStack	equ	0100h

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
SelectorFlatC		equ	LABEL_DESC_FLAT_C - LABEL_GDT
SelectorFlatRW		equ	LABEL_DESC_FLAT_RW - LABEL_GDT
SelectorVideo		equ	LABEL_DESC_VIDEO - LABEL_GDT + SA_RPL3
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; Main
;-------------------------------------------------------------------------------------
start:	
	mov		ax, cs
	mov		ds, ax
	mov		es, ax
	mov		ss, ax
	mov		sp, BaseOfStack

; show "Loading..."
	mov		dh, 0
	call	displayStr

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
	jmp		.findkernel
.fail:
	mov	dword [MCRNumber16], 0
.findkernel:

; find kernel.bin
%define StackBase KernelBinSeg
	mov	word [wSectorNo], SectorNoOfRootDirectory
	xor		ah, ah
	xor		dl, dl
	int		13h
.begin:
	cmp	word [wRootDirSizeForLoop], 0
	jz		.nokernel	

	dec	word [wRootDirSizeForLoop]

	mov		ax, KernelBinSeg
	mov		es, ax	
	mov		bx, KernelBinOff
	mov		ax, [wSectorNo]
	mov		cl, 1
	call	readSector

	mov		si, kernel
	mov		di, KernelBinOff
	cld
	mov		dx, 10h
.search:
	cmp		dx, 0
	jz		.nextSector
	dec		dx	
	mov		cx, 11
.cmpName:
	cmp		cx, 0
	jz		.found
	dec		cx
	lodsb
	cmp		al, byte [es:di]
	jz		.next
	jmp		.diff	
.next:
	inc		di
	jmp		.cmpName
.diff:
	and		di, 0FFE0h	
	add		di, 20h	
	mov		si, kernel	
	jmp		.search
.nextSector:
	add	word [wSectorNo], 1
	jmp		.begin
.nokernel:
	mov		dh, 2
	call	displayStr	
	jmp		$
.found:	
	mov		ax, RootDirSectors
	and		di, 0FFE0h	

	push	eax
	mov		eax, [es : di + 01Ch]		
	mov dword [dwKernelSize], eax
	cmp		eax, KERNEL_VALID_SPACE
	ja		.toolarge
	pop		eax
	jmp		.valid
.toolarge:
	mov		dh, 3
	call	displayStr
	jmp		$
.valid:
	add		di, 01Ah	
	mov		cx, word [es:di]
	push	cx			; 保存此 Sector 在 FAT 中的序号
	add		cx, ax
	add		cx, DeltaSectorNo

	mov		ax, KernelBinSeg
	mov		es, ax	
	mov		bx, KernelBinOff
	mov		ax, cx	
.more:
	mov		cl, 1
	call	readSector
	pop		ax			; 取出此 Sector 在 FAT 中的序号
	call	getFAT
	cmp		ax, 0FFFh
	jz		.finish

	push	ax			; 保存此 Sector 在 FAT 中的序号
	mov		dx, RootDirSectors
	add		ax, dx
	add		ax, DeltaSectorNo
	add		bx, [BPB_BytsPerSec]
	jc		.upper64k	; 如果 bx 重新变成 0，说明内核大于 64K（bx最大为0xFFFF，即64K）
	jmp		.jmp_more
.upper64k:
	push	ax
	mov		ax, es
	add		ax, 1000h
	mov		es, ax
	pop		ax
.jmp_more:
	jmp		.more

.finish:
	call	killMotor
	mov		dh, 1
	call	displayStr
	
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
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; data
;-------------------------------------------------------------------------------------
wRootDirSizeForLoop	dw	RootDirSectors
wSectorNo			dw	0	
isOdd				db	0	
dwKernelSize		dd	0

kernel		db	"KERNEL  BIN", 0
msgLen		equ	10
msg0		db	"Loading..."	
msg1		db	"Done      "	
msg2		db	"No KERNEL."	
msg3		db	"Too Large."

row			equ	3
;-------------------------------------------------------------------------------------

%include "utility.inc"

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
	mov		eax, PG_P  | PG_USU | PG_RWW
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
