org		07c00h

	jmp short start
	nop		; 这个 nop 不可少

%include "fat12.inc"
%include "load.inc"

BaseOfStack		equ	07c00h

;-------------------------------------------------------------------------------------
; Start Booting
;-------------------------------------------------------------------------------------
start:
	mov		ax, cs
	mov		ds, ax
	mov		es, ax
	mov		ss, ax
	mov		sp, BaseOfStack

; clear screen
	mov		ax, 0600h
	mov		bx, 0700h
	mov		cx, 0
	mov		dx, 0184fh
	int		10h

; show "Booting..."
	mov		dh, 0
	call	disp_str

; reset floppy
	xor		ah, ah
	xor		dl, dl
	int		13h

; search loader.bin
%define StackBase LoaderSeg
	mov	word [wSectorNo], SectorNoOfRootDirectory
.begin:
	cmp	word [wRootDirSizeForLoop], 0
	jz		.noloader	

	dec	word [wRootDirSizeForLoop]

	mov		ax, LoaderSeg
	mov		es, ax	
	mov		bx, LoaderOff
	mov		ax, [wSectorNo]
	mov		cl, 1
	call	read_sector

	mov		si, loader
	mov		di, LoaderOff
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
	mov		si, loader	
	jmp		.search
.nextSector:
	add	word [wSectorNo], 1
	jmp		.begin
.noloader:
	mov		dh, 2
	call	disp_str	
	jmp		$
.found:	
	mov		ax, RootDirSectors
	and		di, 0FFE0h	
	add		di, 01Ah	

	mov		cx, word [es:di]
	push	cx	
	add		cx, ax
	add		cx, DeltaSectorNo

	mov		ax, LoaderSeg
	mov		es, ax	
	mov		bx, LoaderOff
	mov		ax, cx	
.more:
	mov		cl, 1
	call	read_sector
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
	mov		dh, 1
	call	disp_str	

	jmp		LoaderSeg:LoaderOff
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; data
;-------------------------------------------------------------------------------------
wRootDirSizeForLoop	dw	RootDirSectors
wSectorNo	dw	0	
isOdd		db	0	

loader		db	"LOADER  BIN", 0
msgLen		equ	10
msg0		db	"Booting..."	
msg1		db	"Done      "	
msg2		db	"No LOADER "	

row			equ	0
;-------------------------------------------------------------------------------------

%include "utils.inc"

;----------------------------------------------------------------------------
; end
;----------------------------------------------------------------------------
times 	510 - ($ - $$)	db	0
dw 		0xaa55
;----------------------------------------------------------------------------