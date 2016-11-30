# Makefile for VIOS

# Entry point of kernel
ENTRYPOINT		=	0x1000

# Offset of entry point in kernel file
ENTRYOFFSET		=	0x400

# Mount point
MOUNTPOINT		=	/mnt/floppy/

# Programs, flags, etc.
ASM				=	nasm
DASM			=	ndisasm
CC				=	gcc
LD				=	ld
AR				=	ar

ASMBFLAGS		=	-I boot/include/
ASMKFLAGS		=	-I include/ -I include/sys/ -f elf
CFLAGS			=	-I include/ -I include/sys/ -c -fno-builtin -m32 -Wall
LDFLAGS			=	-s -Ttext $(ENTRYPOINT) -m elf_i386 -Map kernal.map
DASMFLAGS		=	-u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)
ARFLAGS			=	rcs

# Object
VIOSBOOT		=	boot/boot.bin boot/loader.bin
VIOSKERNEL		=	kernel.bin
VIOSLIB			=	lib/vios_crt.a
OBJS			=	kernel/kernel.o kernel/start.o kernel/i8259.o \
					kernel/global.o kernel/protect.o kernel/main.o \
					kernel/clock.o kernel/proc.o kernel/hd.o \
					kernel/keyborad.o kernel/tty.o kernel/console.o \
					kernel/systask.o kernel/message.o kernel/panic.o \
					fs/main.o fs/misc.o fs/open.o fs/read_write.o \
					fs/link.o fs/stat.o\
					mm/main.o mm/fork_exit.o mm/exec.o
LIBOBJS			=	lib/misc.o lib/klib.o lib/kliba.o lib/string.o \
					lib/open.o lib/close.o lib/read.o lib/write.o\
					lib/getpid.o lib/unlink.o lib/fork.o lib/exit.o\
					lib/wait.o lib/printf.o lib/vsprintf.o lib/syscall.o \
					lib/exec.o lib/stat.o
DASMOUTPUT		=	kernel.bin.asm

# image
VIOSIMAGE		=	vios.img

# All Phony Targets
.PHONY: 	bin image clean realclean disasm building

bin: 	$(VIOSBOOT) $(VIOSKERNEL) clean

image:	bin building final

clean:	
		rm -f $(OBJS) $(LIBOBJS)

final:
		rm -f $(VIOSBOOT)
		sudo umount $(MOUNTPOINT)

disasm:	
		$(DASM) $(DASMFLAGS) $(VIOSKERNEL) > $(DASMOUTPUT)

resetimg:
		sudo dd if=/dev/zero of=$(VIOSIMAGE) bs=512 count=2880

building:
		sudo dd if=boot/boot.bin of=$(VIOSIMAGE) bs=512 count=1 conv=notrunc
		sudo mount -o loop $(VIOSIMAGE) $(MOUNTPOINT)
		sudo cp -fv boot/loader.bin $(MOUNTPOINT)
		sudo cp -fv kernel.bin $(MOUNTPOINT)


boot/boot.bin:  boot/boot.asm
	$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin: boot/loader.asm
	$(ASM) $(ASMBFLAGS) -o $@ $<


$(VIOSKERNEL): $(OBJS) $(VIOSLIB)
	$(LD) $(LDFLAGS) -o $(VIOSKERNEL) $^

$(VIOSLIB): $(LIBOBJS)
	$(AR) $(ARFLAGS) $@ $^

kernel/kernel.o: kernel/kernel.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

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

kernel/proc.o: kernel/proc.c
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

lib/getpid.o: lib/getpid.c
	$(CC) $(CFLAGS) -o $@ $<

lib/unlink.o: lib/unlink.c
	$(CC) $(CFLAGS) -o $@ $<

lib/fork.o: lib/fork.c
	$(CC) $(CFLAGS) -o $@ $<

lib/exit.o: lib/exit.c
	$(CC) $(CFLAGS) -o $@ $<

lib/wait.o: lib/wait.c
	$(CC) $(CFLAGS) -o $@ $<

lib/printf.o: lib/printf.c
	$(CC) $(CFLAGS) -o $@ $<

lib/vsprintf.o: lib/vsprintf.c
	$(CC) $(CFLAGS) -o $@ $<

lib/syscall.o: lib/syscall.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/exec.o: lib/exec.c
	$(CC) $(CFLAGS) -o $@ $<

lib/stat.o: lib/stat.c
	$(CC) $(CFLAGS) -o $@ $<


fs/main.o: fs/main.c
	$(CC) $(CFLAGS) -o $@ $<

fs/open.o: fs/open.c
	$(CC) $(CFLAGS) -o $@ $<

fs/misc.o: fs/misc.c
	$(CC) $(CFLAGS) -o $@ $<

fs/read_write.o: fs/read_write.c
	$(CC) $(CFLAGS) -o $@ $<

fs/link.o: fs/link.c
	$(CC) $(CFLAGS) -o $@ $<

fs/stat.o: fs/stat.c
	$(CC) $(CFLAGS) -o $@ $<


mm/main.o: mm/main.c
	$(CC) $(CFLAGS) -o $@ $<

mm/fork_exit.o: mm/fork_exit.c
	$(CC) $(CFLAGS) -o $@ $<

mm/exec.o: mm/exec.c
	$(CC) $(CFLAGS) -o $@ $<