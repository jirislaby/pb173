#include <linux/kernel.h>
#include <linux/module.h>

static int my_init(void)
{
	unsigned int a;

	for (a = 0; a <= 7; a++)
		printk("<%u>" "level %u\n", a, a);

	return -EIO;
}

static void my_exit(void)
{
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
