# Makefile for VIOS

# Entry point of kernel
ENTRYPOINT		=	0x30400

# Offset of entry point in kernel file
ENTRYOFFSET		=	0x400

# Mount point
MOUNTPOINT		=	/mnt/floppy/

# Programs, flags, etc.
ASM				=	nasm
DASM			=	ndisasm
CC				=	gcc
LD				=	ld
ASMBFLAGS		=	-I boot/include/
ASMKFLAGS		=	-I include/ -f elf
CFLAGS			=	-I include/ -c -fno-builtin -m32
LDFLAGS			=	-s -Ttext $(ENTRYPOINT) -m elf_i386
DASMFLAGS		=	-u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

# Object
VIOSBOOT		=	boot/boot.bin boot/loader.bin
VIOSKERNEL		=	kernel.bin
OBJS			=	kernel/kernel.o kernel/start.o kernel/i8259.o kernel/global.o kernel/protect.o lib/klib.o lib/kliba.o lib/string.o
DASMOUTPUT		=	kernel.bin.asm

# image
VIOSIMAGE		=	vios.img

# All Phony Targets
.PHONY: 	bin image clean realclean disasm building

bin: 	$(VIOSBOOT) $(VIOSKERNEL) clean

image:	bin building cleanall

clean:	
		rm -f $(OBJS) 

cleanall:
		rm -f $(VIOSBOOT) $(VIOSKERNEL)
		sudo umount $(MOUNTPOINT)

disasm:	$(DASM) $(DASMFLAGS) $(VIOSKERNEL) > $(DASMOUTPUT)

resetimg:
		dd if=/dev/zero of=$(VIOSIMAGE) bs=512 count=2880

building:
		dd if=boot/boot.bin of=$(VIOSIMAGE) bs=512 count=1 conv=notrunc
		sudo mount -o loop $(VIOSIMAGE) $(MOUNTPOINT)
		sudo cp -fv boot/loader.bin $(MOUNTPOINT)
		sudo cp -fv kernel.bin $(MOUNTPOINT)

boot/boot.bin: 	boot/boot.asm boot/include/load.inc \
				boot/include/fat12.inc boot/include/func.inc 
		$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin: 	boot/loader.asm boot/include/load.inc \
				boot/include/fat12.inc boot/include/pm.inc \
				boot/include/func.inc 
		$(ASM) $(ASMBFLAGS) -o $@ $<

kernel/kernel.o:	kernel/kernel.asm
		$(ASM) $(ASMKFLAGS) -o $@ $<

$(VIOSKERNEL):	$(OBJS)
		$(LD) $(LDFLAGS) -o $(VIOSKERNEL) $(OBJS)

kernel/start.o:	kernel/start.c include/type.h include/const.h \
				include/protect.h include/proto.h include/string.h
		$(CC) $(CFLAGS) -o $@ $<

kernel/i8259.o: 	kernel/i8259.c include/type.h include/const.h \
				include/protect.h include/proto.h
		$(CC) $(CFLAGS) -o $@ $<

kernel/global.o:	kernel/global.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/protect.o:kernel/protect.c
		$(CC) $(CFLAGS) -o $@ $<

lib/klib.o: 		lib/klib.c
		$(CC) $(CFLAGS) -o $@ $<

lib/kliba.o:		lib/kliba.asm
		$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o:	lib/string.asm
		$(ASM) $(ASMKFLAGS) -o $@ $<
