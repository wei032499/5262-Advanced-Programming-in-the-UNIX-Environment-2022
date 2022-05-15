CC := gcc
CC_FLAG := -c -g -Wall -fno-stack-protector -nostdlib

ASM64 := yasm -f elf64 -DYASM -D__x86_64__ -DPIC

objects = write1 alarm1 alarm2 alarm3 jmp1

all: $(objects)

$(objects): %: %.o ../start.o
	ld -m elf_x86_64 --dynamic-linker /lib64/ld-linux-x86-64.so.2 -o $@ $^ -L. -L.. -lmini
	rm $<

%.o: %.asm
	$(ASM64) $? -o $@

%.o: %.c
	$(CC) $(CC_FLAG) -I. -I.. -DUSEMINI $? -o $@



.PHONY: clean clean_all

clean:
	rm -f *.o ../start.o
clean_all:
	rm -f *.o ../start.o
	rm -f $(objects)