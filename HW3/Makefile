CC := gcc
CC_FLAG := -c -g -Wall -fno-stack-protector -fPIC -nostdlib

ASM64 := yasm -f elf64 -DYASM -D__x86_64__ -DPIC

BUILD_SO := libmini.so

all: libmini64.o libmini.o
	ld -shared -o $(BUILD_SO) $^

%.o: %.asm
	$(ASM64) $? -o $@

%.o: %.c
	$(CC) $(CC_FLAG) $? -o $@

.PHONY: clean clean_all

clean:
	rm -f *.o

clean_all:
	rm -f *.o
	rm -f $(BUILD_SO)