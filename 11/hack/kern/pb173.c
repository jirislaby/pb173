#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/uaccess.h>

static void my_fun(const uid_t *uid)
{
	printk(KERN_INFO "%s\n", __func__);
}

struct my_struct {
	char str[10];
	void (*fun)(const uid_t *uid);
} my_s;

static ssize_t my_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *off)
{
	my_s.fun = my_fun;

	if (copy_from_user(&my_s.str, buf, count))
		return -EFAULT;

#ifdef NEW_KERNEL
	my_s.fun(&current->cred->uid);
#else
	my_s.fun(&current->uid);
#endif

	return count;
}

static struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.write = my_write,
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
