
;-------------------------------------------------------------------------------------
; function displayStr
;-------------------------------------------------------------------------------------
displayStr:
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
; fucntion readSector
;-------------------------------------------------------------------------------------
readSector:
	; -----------------------------------------------------------------------
	; 怎样由扇区号求扇区在磁盘中的位置 (扇区号 -> 柱面号, 起始扇区, 磁头号)
	; -----------------------------------------------------------------------
	; 设扇区号为 x
	;                           ┌ 柱面号 = y >> 1
	;       x           ┌ 商 y ┤
	; -------------- => ┤      └ 磁头号 = y & 1
	;  每磁道扇区数     │
	;                   └ 余 z => 起始扇区号 = z + 1
	push	bp
	mov		bp, sp
	sub		esp, 2	

	mov	byte [bp-2], cl
	push	bx	
	mov		bl, [BPB_SecPerTrk]
	div		bl	
	inc		ah	
	mov		cl, ah
	mov		dh, al
	shr		al, 1
	mov		ch, al	
	and		dh, 1
	pop		bx

	mov		dl, [BS_DrvNum]
.s:
	mov		ah, 2
	mov		al, byte [bp-2]
	int		13h
	jc		.s

	add	esp, 2
	pop	bp

	ret

;----------------------------------------------------------------------------
; function getFAT
;----------------------------------------------------------------------------
; 作用:
;	找到序号为 ax 的 Sector 在 FAT 中的条目, 结果放在 ax 中
;	需要注意的是, 中间需要读 FAT 的扇区到 es:bx 处, 所以函数一开始保存了 es 和 bx
getFAT:
	push	es
	push	bx
	push	ax
	mov		ax, StackBase	
	sub		ax, 0100h		
	mov		es, ax	
	pop	ax

	mov		byte [isOdd], 0
	mov		bx, 3
	mul		bx
	mov		bx, 2
	div		bx	
	cmp		dx, 0
	jz		.even
	mov	byte [isOdd], 1
.even:
	xor		dx, dx
	mov		bx, [BPB_BytsPerSec]
	div		bx

	push	dx
	mov		bx, 0
	add		ax, SectorNoOfFAT1
	mov		cl, 2
	call	readSector
	pop		dx

	add		bx, dx
	mov		ax, [es:bx]
	cmp		byte [isOdd], 1
	jnz		.even2
	shr		ax, 4
.even2:
	and		ax, 0FFFh

ok:
	pop		bx
	pop		es
	ret
;----------------------------------------------------------------------------