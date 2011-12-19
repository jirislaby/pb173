OBJCOPY=objcopy
CFLAGS=-nostdinc -O2 -Wall

all: firmware_32.bin firmware_64.bin

%.bin: %.o
	$(OBJCOPY) -O binary -j .text $^ $@

%_32.o: %.c
	$(CC) $(CFLAGS) -m32 -c -o $@ $^

%_64.o: %.c
	$(CC) $(CFLAGS) -m64 -c -o $@ $^

clean:
	@rm -f *.o *.bin

.PHONY: all clean
