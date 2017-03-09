DA_32		equ	4000h	
DA_LIMIT_4K	equ	8000h	

DA_DPL0		equ	  00h	
DA_DPL1		equ	  20h	
DA_DPL2		equ	  40h	
DA_DPL3		equ	  60h	

DA_DR		equ	90h	
DA_DRW		equ	92h	
DA_DRWA		equ	93h	
DA_C			equ	98h	
DA_CR		equ	9Ah	
DA_CCO		equ	9Ch	
DA_CCOR		equ	9Eh	

DA_LDT		equ	  82h	
DA_TaskGate	equ	  85h	
DA_386TSS	equ	  89h	
DA_386CGate	equ	  8Ch	
DA_386IGate	equ	  8Eh	
DA_386TGate	equ	  8Fh	

SA_RPL0		equ	0	
SA_RPL1		equ	1	
SA_RPL2		equ	2	
SA_RPL3		equ	3	

SA_TIG		equ	0	
SA_TIL		equ	4	

PG_P		equ	1	
PG_RWR		equ	0	
PG_RWW		equ	2	
PG_USS		equ	0	
PG_USU		equ	4	

%macro Descriptor 3
	dw	%2 & 0FFFFh				
	dw	%1 & 0FFFFh				
	db	(%1 >> 16) & 0FFh			
	dw	((%2 >> 8) & 0F00h) | (%3 & 0F0FFh)	
	db	(%1 >> 24) & 0FFh			
%endmacro 

%macro Gate 4
	dw	(%2 & 0FFFFh)				
	dw	%1					
	dw	(%3 & 1Fh) | ((%4 << 8) & 0FF00h)	
	dw	((%2 >> 16) & 0FFFFh)			
%endmacro 

