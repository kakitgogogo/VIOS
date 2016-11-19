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
CFLAGS			=	-I include/ -c -fno-builtin -m32 -Wall
LDFLAGS			=	-s -Ttext $(ENTRYPOINT) -m elf_i386 -Map kernal.map
DASMFLAGS		=	-u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

# Object
VIOSBOOT		=	boot/boot.bin boot/loader.bin
VIOSKERNEL		=	kernel.bin
OBJS			=	kernel/kernel.o kernel/start.o kernel/i8259.o \
					kernel/global.o kernel/protect.o kernel/main.o \
					kernel/clock.o kernel/syscall.o kernel/proc.o \
					kernel/keyborad.o kernel/tty.o kernel/console.o \
					kernel/printf.o kernel/vsprintf.o kernel/systask.o \
					kernel/message.o kernel/panic.o kernel/hd.o \
					lib/misc.o lib/klib.o lib/kliba.o lib/string.o \
					lib/open.o lib/close.o lib/read.o lib/write.o\
					fs/main.o fs/misc.o fs/open.o fs/read_write.o
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

kernel/kernel.o:	kernel/kernel.asm include/sconst.inc
		$(ASM) $(ASMKFLAGS) -o $@ $<

$(VIOSKERNEL):	$(OBJS)
		$(LD) $(LDFLAGS) -o $(VIOSKERNEL) $(OBJS)

kernel/start.o:	kernel/start.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/main.o: 	kernel/main.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/clock.o: kernel/clock.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/keyborad.o: kernel/keyboard.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/tty.o: kernel/tty.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/console.o: kernel/console.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/i8259.o: kernel/i8259.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/global.o: kernel/global.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/protect.o: kernel/protect.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/syscall.o: kernel/syscall.asm
		$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/proc.o: kernel/proc.c
		$(CC) $(CFLAGS) -o $@ $<

kernel/printf.o: kernel/printf.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/vsprintf.o: kernel/vsprintf.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/systask.o: kernel/systask.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/panic.o: kernel/panic.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/message.o: kernel/message.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/hd.o: kernel/hd.c
	$(CC) $(CFLAGS) -o $@ $<


lib/klib.o: lib/klib.c
		$(CC) $(CFLAGS) -o $@ $<

lib/kliba.o: lib/kliba.asm
		$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o: lib/string.asm
		$(ASM) $(ASMKFLAGS) -o $@ $<

lib/misc.o: lib/misc.c
		$(CC) $(CFLAGS) -o $@ $<

lib/open.o: lib/open.c
		$(CC) $(CFLAGS) -o $@ $<

lib/close.o: lib/close.c
		$(CC) $(CFLAGS) -o $@ $<

lib/read.o: lib/read.c
		$(CC) $(CFLAGS) -o $@ $<

lib/write.o: lib/write.c
		$(CC) $(CFLAGS) -o $@ $<

fs/main.o: fs/main.c
		$(CC) $(CFLAGS) -o $@ $<

fs/open.o: fs/open.c
		$(CC) $(CFLAGS) -o $@ $<

fs/misc.o: fs/misc.c
		$(CC) $(CFLAGS) -o $@ $<

fs/read_write.o: fs/read_write.c
		$(CC) $(CFLAGS) -o $@ $<