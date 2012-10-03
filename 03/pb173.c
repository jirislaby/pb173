#include <linux/kernel.h>
#include <linux/module.h>

static int my_init(void)
{
#define prn(x) printk(x "level " #x " %c\n", (x)[1]);
	prn(KERN_EMERG);
	prn(KERN_ALERT);
	prn(KERN_CRIT);
	prn(KERN_ERR);
	prn(KERN_WARNING);
	prn(KERN_NOTICE);
	prn(KERN_INFO);
	prn(KERN_DEBUG);

	return -EIO;
}

static void my_exit(void)
{
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
