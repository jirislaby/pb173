KDIR=/lib/modules/$(shell uname -r)/build
KBUILD=$(MAKE) -C $(KDIR) M=$(PWD)

obj-m := pb173.o

modules:
	$(KBUILD) modules

clean:
	$(KBUILD) clean
	@rm -f Module.symvers

modules_install:
	$(KBUILD) modules_install

cscope:
	cscope -b `find $(src) -type f -name '*.[ch]'`

.PHONY: modules clean modules_install cscope
