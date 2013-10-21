CFLAGS  := -m32 -fno-stack-protector -fno-builtin -nostdinc -O -g -Wall -I.
LDFLAGS := -melf_i386 -nostdlib -Wl,-N -Wl,-Ttext -Wl,100000

all:	/boot/kernel.bin

/boot/kernel.bin:	start.o	main.o scrn.o kbd.o io.o
	ld -melf_i386 -T link.ld -o /boot/kernel.bin start.o main.o scrn.o kbd.o io.o
	@echo Done!

kernel2.bin: start.asm main.c scrn.c start.o
	gcc -o kernel.bin $(CFLAGS) start.o main.c scrn.c $(LDFLAGS)


start.o: start.asm
	nasm  -f elf -o start.o start.asm

main.o: main.c system.h
	gcc $(CFLAGS) -c -o main.o main.c

scrn.o: scrn.c system.h
	gcc  $(CFLAGS) -c -o scrn.o scrn.c

kbd.o: kbd.c system.h
	gcc $(CFLAGS) -c -o kbd.o kbd.c

io.o: io.c system.h
	gcc $(CFLAGS) -c -o io.o io.c

clean: 
	rm -f *.o *.bin
	rm -f /boot/kernel.bin
