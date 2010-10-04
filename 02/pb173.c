#include <linux/module.h>
#include <linux/sched.h>
#include <linux/uaccess.h>

static int my_init(void)
{
	char buf[100];
	void *copy_to;

	copy_to = NULL;
	printk(KERN_INFO "copy to NULL: %d\n",
			(int)copy_to_user(copy_to, buf, sizeof(buf)));

	copy_to = current;
	printk(KERN_INFO "copy to kernel structures: %d\n",
			(int)copy_to_user(copy_to, buf, sizeof(buf)));

	/* this way it won't load */
	return -EIO;
}

static void my_exit(void)
{
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
