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
BIN			=	echo pwd

.PHONY: bin clean install all

bin:		$(BIN)

clean:
		rm -f $(BIN) *.o kernel.bin

install:	bin
		cp ../kernel.bin ./ -v
		tar vcf cmd.tar kernel.bin $(BIN)
		sudo dd if=cmd.tar of=$(HD) seek=`echo "obase=10;ibase=16;(\`egrep -e '^ROOT_BASE' ../boot/include/load.inc | sed -e 's/.*0x//g'\`+\`egrep -e '#define[[:space:]]*INSTALL_START_SECT' ../include/config.h | sed -e 's/.*0x//g'\`)*200" | bc` bs=1 count=`ls -l cmd.tar | awk -F " " '{print $$5}'` conv=notrunc

all:		install clean

kernel.bin:
	cp ../kernel.bin ./ -v

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
