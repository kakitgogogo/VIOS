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
			dd	BaseOfLoaderPhyAddr + LABEL_GDT
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
	mov		di, memBuf16
.loop:
	mov		eax, 0E820h	
	mov		ecx, 20		
	mov		edx, 0534D4150h	
	int		15h	

	jc		.fail
	add		di, 20
	inc		dword [_dwMCRNumber]

	cmp		ebx, 0
	jne		.loop
	jmp		.end
.fail:
	mov		dword [_dwMCRNumber], 0
.end:

; find kernel.bin
%define stackBase BaseOfKernelFile 
	mov	word [wSectorNo], SectorNoOfRootDirectory
.begin:
	cmp	word [wRootDirSizeForLoop], 0
	jz		.noloader	

	dec	word [wRootDirSizeForLoop]

	mov		ax, BaseOfKernelFile
	mov		es, ax	
	mov		bx, OffsetOfKernelFile
	mov		ax, [wSectorNo]
	mov		cl, 1
	call	readSector

	mov		si, kernel
	mov		di, OffsetOfKernelFile
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
	dec	cx
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
.noloader:
	mov		dh, 2
	call	displayStr	
	jmp		.jumppm
.found:	
	mov		ax, RootDirSectors
	and		di, 0FFE0h	
	add		di, 01Ah	

	mov		cx, word [es:di]
	push	cx	
	add		cx, ax
	add		cx, DeltaSectorNo

	mov		ax, BaseOfKernelFile
	mov		es, ax	
	mov		bx, OffsetOfKernelFile
	mov		ax, cx	
.more:
	mov		cl, 1
	call	readSector
	pop		ax	
	call	getFAT
	cmp		ax, 0FFFh
	jz		.ok

	push	ax	
	mov		dx, RootDirSectors
	add		ax, dx
	add		ax, DeltaSectorNo
	add		bx, [BPB_BytsPerSec]
	jmp		.more
.ok:
	call	killMotor
	mov		dh, 1
	call	displayStr
	
; jump to protect mode
.jumppm:
	lgdt	[GdtPtr]

	cli

	in		al, 92h
	or		al, 00000010b
	out		92h, al

	mov		eax, cr0
	or		eax, 1
	mov		cr0, eax

	jmp	dword SelectorFlatC:(BaseOfLoaderPhyAddr+start32)
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

row			equ	3
;-------------------------------------------------------------------------------------

%include "func.inc"

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

	push	szMemChkTitle
	call	displayStr32
	add		esp, 4

	call	DispMemInfo
	call	SetupPaging

	call	InitKernel

	jmp		SelectorFlatC:KernelEntryPointPhyAddr
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function displayAL
;-------------------------------------------------------------------------------------
displayAL:
	push	ecx
	push	edx
	push	edi

	mov		edi, [dwDispPos]

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

	mov		[dwDispPos], edi

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

	mov		ah, 0Fh
	mov		al, 'h'
	push	edi
	mov		edi, [dwDispPos]
	mov		[gs:edi], ax
	add		edi, 4
	mov		[dwDispPos], edi
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
	mov		edi, [dwDispPos]
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
	mov		[dwDispPos], edi

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
	push	szReturn
	call	displayStr32
	add	esp, 4

	ret
;-------------------------------------------------------------------------------------


;-------------------------------------------------------------------------------------
; function memcpy
;-------------------------------------------------------------------------------------
; void* memcpy(void* es:pDest, void* ds:pSrc, int iSize);
;-------------------------------------------------------------------------------------
memcpy:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	esi, [ebp + 12]	; Source
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	al, [ds:esi]		; ┓
	inc	esi			; ┃
					; ┣ 逐字节移动
	mov	byte [es:edi], al	; ┃
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:
	mov	eax, [ebp + 8]	; 返回值

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回
;-------------------------------------------------------------------------------------


; 显示内存信息 --------------------------------------------------------------
DispMemInfo:
	push	esi
	push	edi
	push	ecx

	mov	esi, MemChkBuf
	mov	ecx, [dwMCRNumber]	;for(int i=0;i<[MCRNumber];i++) // 每次得到一个ARDS(Address Range Descriptor Structure)结构
.loop:					;{
	mov	edx, 5			;	for(int j=0;j<5;j++)	// 每次得到一个ARDS中的成员，共5个成员
	mov	edi, ARDStruct		;	{			// 依次显示：BaseAddrLow，BaseAddrHigh，LengthLow，LengthHigh，Type
.1:					;
	push	dword [esi]		;
	call	displayInt			;		DispInt(MemChkBuf[j*4]); // 显示一个成员
	pop	eax			;
	stosd				;		ARDStruct[j*4] = MemChkBuf[j*4];
	add	esi, 4			;
	dec	edx			;
	cmp	edx, 0			;
	jnz	.1			;	}
	call	endline		;	printf("\n");
	cmp	dword [dwType], 1	;	if(Type == AddressRangeMemory) // AddressRangeMemory : 1, AddressRangeReserved : 2
	jne	.2			;	{
	mov	eax, [dwBaseAddrLow]	;
	add	eax, [dwLengthLow]	;
	cmp	eax, [dwMemSize]	;		if(BaseAddrLow + LengthLow > MemSize)
	jb	.2			;
	mov	[dwMemSize], eax	;			MemSize = BaseAddrLow + LengthLow;
.2:					;	}
	loop	.loop			;}
					;
	call	endline		;printf("\n");
	push	szRAMSize		;
	call	displayStr32			;printf("RAM size:");
	add	esp, 4			;
					;
	push	dword [dwMemSize]	;
	call	displayInt			;DispInt(MemSize);
	add	esp, 4			;

	pop	ecx
	pop	edi
	pop	esi
	ret
; ---------------------------------------------------------------------------

; 启动分页机制 --------------------------------------------------------------
SetupPaging:
	; 根据内存大小计算应初始化多少PDE以及多少页表
	xor	edx, edx
	mov	eax, [dwMemSize]
	mov	ebx, 400000h	; 400000h = 4M = 4096 * 1024, 一个页表对应的内存大小
	div	ebx
	mov	ecx, eax	; 此时 ecx 为页表的个数，也即 PDE 应该的个数
	test	edx, edx
	jz	.no_remainder
	inc	ecx		; 如果余数不为 0 就需增加一个页表
.no_remainder:
	push	ecx		; 暂存页表个数

	; 为简化处理, 所有线性地址对应相等的物理地址. 并且不考虑内存空洞.

	; 首先初始化页目录
	mov	ax, SelectorFlatRW
	mov	es, ax
	mov	edi, PageDirBase	; 此段首地址为 PageDirBase
	xor	eax, eax
	mov	eax, PageTblBase | PG_P  | PG_USU | PG_RWW
.1:
	stosd
	add	eax, 4096		; 为了简化, 所有页表在内存中是连续的.
	loop	.1

	; 再初始化所有页表
	pop	eax			; 页表个数
	mov	ebx, 1024		; 每个页表 1024 个 PTE
	mul	ebx
	mov	ecx, eax		; PTE个数 = 页表个数 * 1024
	mov	edi, PageTblBase	; 此段首地址为 PageTblBase
	xor	eax, eax
	mov	eax, PG_P  | PG_USU | PG_RWW
.2:
	stosd
	add	eax, 4096		; 每一页指向 4K 的空间
	loop	.2

	mov	eax, PageDirBase
	mov	cr3, eax
	mov	eax, cr0
	or	eax, 80000000h
	mov	cr0, eax
	jmp	short .3
.3:
	nop

	ret
; 分页机制启动完毕 ----------------------------------------------------------



; InitKernel ---------------------------------------------------------------------------------
; 将 KERNEL.BIN 的内容经过整理对齐后放到新的位置
; --------------------------------------------------------------------------------------------
InitKernel:	; 遍历每一个 Program Header，根据 Program Header 中的信息来确定把什么放进内存，放到什么位置，以及放多少。
	xor	esi, esi
	mov	cx, word [BaseOfKernelFilePhyAddr + 2Ch]; ┓ ecx <- pELFHdr->e_phnum
	movzx	ecx, cx					; ┛
	mov	esi, [BaseOfKernelFilePhyAddr + 1Ch]	; esi <- pELFHdr->e_phoff
	add	esi, BaseOfKernelFilePhyAddr		; esi <- OffsetOfKernel + pELFHdr->e_phoff
.Begin:
	mov	eax, [esi + 0]
	cmp	eax, 0				; PT_NULL
	jz	.NoAction
	push	dword [esi + 010h]		; size	┓
	mov	eax, [esi + 04h]		;	┃
	add	eax, BaseOfKernelFilePhyAddr	;	┣ ::memcpy(	(void*)(pPHdr->p_vaddr),
	push	eax				; src	┃		uchCode + pPHdr->p_offset,
	push	dword [esi + 08h]		; dst	┃		pPHdr->p_filesz;
	call	memcpy				;	┃
	add	esp, 12				;	┛
.NoAction:
	add	esi, 020h			; esi += pELFHdr->e_phentsize
	dec	ecx
	jnz	.Begin

	ret
; InitKernel ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


; SECTION .data1 之开始 ---------------------------------------------------------------------------------------------
[SECTION .data1]

ALIGN	32

LABEL_DATA:
; 实模式下使用这些符号
; 字符串
_szMemChkTitle:			db	"BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
_szRAMSize:			db	"RAM size:", 0
_szReturn:			db	0Ah, 0
;; 变量
_dwMCRNumber:			dd	0	; Memory Check Result
_dwDispPos:			dd	(80 * 6 + 0) * 2	; 屏幕第 6 行, 第 0 列。
_dwMemSize:			dd	0
_ARDStruct:			; Address Range Descriptor Structure
	_dwBaseAddrLow:		dd	0
	_dwBaseAddrHigh:	dd	0
	_dwLengthLow:		dd	0
	_dwLengthHigh:		dd	0
	_dwType:		dd	0
memBuf16:	times	256	db	0
;
;; 保护模式下使用这些符号
szMemChkTitle		equ	BaseOfLoaderPhyAddr + _szMemChkTitle
szRAMSize		equ	BaseOfLoaderPhyAddr + _szRAMSize
szReturn		equ	BaseOfLoaderPhyAddr + _szReturn
dwDispPos		equ	BaseOfLoaderPhyAddr + _dwDispPos
dwMemSize		equ	BaseOfLoaderPhyAddr + _dwMemSize
dwMCRNumber		equ	BaseOfLoaderPhyAddr + _dwMCRNumber
ARDStruct		equ	BaseOfLoaderPhyAddr + _ARDStruct
	dwBaseAddrLow	equ	BaseOfLoaderPhyAddr + _dwBaseAddrLow
	dwBaseAddrHigh	equ	BaseOfLoaderPhyAddr + _dwBaseAddrHigh
	dwLengthLow	equ	BaseOfLoaderPhyAddr + _dwLengthLow
	dwLengthHigh	equ	BaseOfLoaderPhyAddr + _dwLengthHigh
	dwType		equ	BaseOfLoaderPhyAddr + _dwType
MemChkBuf		equ	BaseOfLoaderPhyAddr + memBuf16


; 堆栈就在数据段的末尾
StackSpace:	times	1000h	db	0
TopOfStack	equ	BaseOfLoaderPhyAddr + $	; 栈顶
; SECTION .data1 之结束 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
