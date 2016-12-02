
;-------------------------------------------------------------------------------------
; function disp_str
;-------------------------------------------------------------------------------------
disp_str:
	mov		ax, msgLen
	mul		dh
	add		ax, msg0

	mov		bp, ax
	mov		ax, ds
	mov		es, ax

	mov		cx, msgLen
	mov		ax, 01301h	
	mov		bx, 000Fh
	mov		dl, 0
	add		dh, row
	int		10h	
	ret
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; fucntion err
;-------------------------------------------------------------------------------------
err:
	mov		dh, 3
	call	disp_str
	jmp		$
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; fucntion read_sector
;-------------------------------------------------------------------------------------
; Entry:
;     - disk_address_packet
; Exit:
;     - es:bx -> data read
; registers changed:
;     - eax, ebx, dl, si, es
read_sector:
	xor		ebx, ebx

	mov		ah, 0x42
	mov		dl, 0x80
	mov		si, disk_address_packet
	int		0x13

	mov		ax, [disk_address_packet + 6]
	mov		es, ax
	mov		bx, [disk_address_packet + 4]

	ret
;-------------------------------------------------------------------------------------

;-------------------------------------------------------------------------------------
; function get_inode
;-------------------------------------------------------------------------------------
; Entry:
;     - eax    : inode id.
; Exit:
;     - eax    : sector id.
;     - ecx    : the_inode.i_size
;     - es:ebx : inodes sector buffer
; registers changed:
;     - eax, ebx, ecx, edx
get_inode:
	dec		eax
	mov		bl, [fs:SB_INODE_SIZE]
	mul		bl		; eax <- (inode_id - 1) * INODE_SIZE
	mov		edx, SECTOR_BUF_SIZE
	sub		edx, dword [fs:SB_INODE_SIZE]
	cmp		eax, edx
	jg		err
	push	eax

	mov		ebx, [fs:SB_NR_IMAP_SECTS]
	mov		edx, [fs:SB_NR_SMAP_SECTS]
	lea		eax, [ebx + edx + ROOT_BASE + 2]
	mov	dword [disk_address_packet + 8], eax
	call	read_sector

	pop		eax		; [es:ebx+eax] -> the inode

	mov		edx, dword [fs:SB_INODE_SIZE_OFF]
	add		edx, ebx
	add		edx, eax
	mov		ecx, [es:edx]		; [es:edx] -> the_inode.i_size

	add		ax, word [fs:SB_INODE_START_OFF]

	add		bx, ax
	mov		eax, [es:bx]
	add		eax, ROOT_BASE
	ret
;-------------------------------------------------------------------------------------