BaseOfLoader			equ	 09000h
OffsetOfLoader			equ	  0100h

BaseOfLoaderPhyAddr		equ	BaseOfLoader * 10h

BaseOfKernel		equ	 08000h
OffsetOfKernel		equ	 0h

BaseOfKernelPhyAddr	equ	BaseOfKernel * 10h
KernelEntryPhyAddr	equ	030400h

PageDirBase				equ	200000h	
PageTblBase				equ	201000h
