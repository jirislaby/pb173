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

	return simple_read_from_buffer(buf, count, off, string, strlen(string));
}

static struct file_operations my_rfops = {
	.owner = THIS_MODULE,
	.read = my_read,
};

static struct miscdevice my_rmisc = {
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &my_rfops,
	.name = "my_read",
};

static ssize_t my_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *off)
{
	return count;
}

static struct file_operations my_wfops = {
	.owner = THIS_MODULE,
	.write = my_write,
};

static struct miscdevice my_wmisc = {
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &my_wfops,
	.name = "my_write",
};

static int my_init(void)
{
	int ret = misc_register(&my_rmisc);
	if (ret)
		return ret;

	ret = misc_register(&my_wmisc);
	if (ret) {
		misc_deregister(&my_rmisc);
		return ret;
	}

	return 0;
}

static void my_exit(void)
{
	misc_deregister(&my_rmisc);
	misc_deregister(&my_wmisc);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
