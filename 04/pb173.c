#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/uaccess.h>

static ssize_t my_read(struct file *filp, char __user *buf, size_t count,
		loff_t *off)
{
	static const char string[] = "to return\n";

	if (*off >= strlen(string) || !count)
		return 0;

	count = 1;

	if (put_user(string[*off], buf))
		return -EFAULT;

	*off += count;
	return count;
}

static struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.read = my_read,
};

static struct miscdevice my_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &my_fops,
	.name = "my_name",
};

static int my_init(void)
{
	return misc_register(&my_misc);
}

static void my_exit(void)
{
	misc_deregister(&my_misc);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
