# Makefile for commands

HD			=	../80m.img

ASM			=	nasm
DASM		=	objdump
CC			=	gcc
LD			=	ld

ASMFLAGS	=	-I ../include/ -f elf
CFLAGS		=	-I ../include/ -c -fno-builtin -fno-stack-protector -m32 -Wall
LDFLAGS		=	-Ttext 0x1000 -m elf_i386
DASMFLAGS	=	-D
LIB			=	../lib/vios_crt.a
BIN			=	echo pwd clear ls cat edit

.PHONY: bin clean install all

all:		install clean

bin:		$(BIN)

clean:
		rm -f $(BIN) *.o kernel.bin hdloader.bin

install:	bin
		cp ../boot/hdloader.bin ./ -v
		cp ../kernel.bin ./ -v
		tar vcf cmd.tar hdloader.bin kernel.bin $(BIN)
		sudo dd if=cmd.tar of=$(HD) seek=`echo "obase=10;ibase=16;(\`egrep -e '^ROOT_BASE' ../boot/include/load.inc | sed -e 's/.*0x//g'\`+\`egrep -e '#define[[:space:]]*INSTALL_START_SECT' ../include/sys/config.h | sed -e 's/.*0x//g'\`)*200" | bc` bs=1 count=`ls -l cmd.tar | awk -F " " '{print $$5}'` conv=notrunc

start.o: start.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

echo.o: echo.c
	$(CC) $(CFLAGS) -o $@ $<

echo: echo.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

pwd.o: pwd.c
	$(CC) $(CFLAGS) -o $@ $<

pwd: pwd.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

clear.o: clear.c
	$(CC) $(CFLAGS) -o $@ $<

clear: clear.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

ls.o: ls.c
	$(CC) $(CFLAGS) -o $@ $<

ls: ls.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

cat.o: cat.c
	$(CC) $(CFLAGS) -o $@ $<

cat: cat.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

edit.o: edit.c
	$(CC) $(CFLAGS) -o $@ $<

edit: edit.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?