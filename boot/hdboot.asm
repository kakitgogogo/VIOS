org		07c00h

	jmp		start

%include "load.inc"

BaseOfStack		equ	07c00h
TRANS_SECT_NR	equ	2
SECT_BUF_SIZE	equ	TRANS_SECT_NR * 512

disk_address_packet:
	db		0x10
	db		0
	dc		TRANS_SECT_NR
	db		0
	dw		0					; Destination Address Offset
	dw		SUPER_BLK_SEG		; Destination Address Segment
	dd		0					; LBA Low 32bits
	dd		0					; LBA High 32bits


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

; read the super block to SUPER_BLK_SEG:0
	mov	dword [disk_address_packet + 8], ROOT_BASE + 1
	call	read_sector
	mov		ax, SUPER_BLK_SEG
	mov		fs, ax

	mov	dword [disk_address_packet + 4], LoaderOff
	mov	dword [disk_address_packet + 6], LoaderSeg

	; get the sector_id of `/' (ROOT_INODE)
	mov		eax, [fs:SB_ROOT_INODE]
	call	get_inode

	mov	dword [disk_address_packet + 8], eax
	call	read_sector

; search hdloader.bin
	mov		si, loader
	push	bx
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

	mov		dx, SECT_BUF_SIZE
	cmp		bx, dx
	jge		.not_found

	push	bx
	mov		si, load
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
.load_loader:
	call	read_sector
	cmp		ecx, SECT_BUF_SIZE
	jl		.done
	sub		ecx, SECT_BUF_SIZE
	add	word [disk_address_packet + 4], SECT_BUF_SIZE
	jc		err
	add	dword [disk_address_packet + 8], TRANS_SECT_NR
	jmp		.load_loader
.done:
	mov		dh, 1
	call	disp_str
	jmp		LoaderSeg:LoaderOff
	jmp		$
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; data
;-------------------------------------------------------------------------------------
wRootDirSizeForLoop	dw	RootDirSectors
wSectorNo	dw	0	
isOdd		db	0	

loader		db	"hdloader.bin", 0
msgLen		equ	10
msg0		db	"Booting..."	
msg1		db	"Done      "	
msg2		db	"No LOADER "	
msgErr		db	"Error 0   "

row			equ	0
;-------------------------------------------------------------------------------------

%include "hdutils.inc"

;----------------------------------------------------------------------------
; end
;----------------------------------------------------------------------------
times 	510 - ($ - $$)	db	0
dw 		0xaa55
;----------------------------------------------------------------------------