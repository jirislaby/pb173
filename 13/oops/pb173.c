#include <linux/kernel.h>
#include <linux/module.h>

static unsigned int crash_type;
module_param(crash_type, uint, S_IRUSR);

int *bad_int = (void *)0x1233;
void (*bad_fun)(void) = (void *)(~0UL - 5);
unsigned int zero;

static int my_init(void)
{
	switch (crash_type) {
	case 0:
		printk("%d\n", *bad_int);
		break;
	case 1:
		printk("%u\n", zero/zero);
		break;
	case 2:
		bad_fun();
		break;
	}
	BUG();
	return 0;
}

static void my_exit(void)
{
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
