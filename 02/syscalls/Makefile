CFLAGS=-O2 -Wall

all: asm c

asm: asm.o
	$(LD) -o $@ $<

asm.o: asm.S

%.o: %.S
	$(AS) -o $@ $<

clean:
	@rm -f *.o c asm

.PHONY: all clean
