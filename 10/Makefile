CC=gcc
LDLD=ld
LD=gcc
CFLAGS=-Wall -O2 -g
LDFLAGS=
OBJCOPY=objcopy

all: pb173 task.bin

pb173: pb173.o

task.bin: task
	$(OBJCOPY) -O binary -j .text -j .rodata $< $@

task: task.o task.x
	$(LDLD) -s -T task.x -o $@ $<

clean:
	rm -f *.o

.PHONY: all clean
