#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>

static DEFINE_SPINLOCK(my_spinlock);
static DEFINE_MUTEX(my_mutex);

static int my_init(void)
{
	void *buf;

	/* this may fail after some time the system is running */
	buf = kmalloc(2 << 20, GFP_KERNEL);
	kfree(buf);

	/* SPINLOCK */

	spin_lock(&my_spinlock);
	/* I cannot sleep here */

	/* for big allocations, this may fail */
	buf = kmalloc(50, GFP_ATOMIC);
	kfree(buf);

	/* for higher orders, this may fail */
	buf = (void *)__get_free_pages(GFP_ATOMIC, 2);
	free_pages((unsigned long)buf, 2);
	spin_unlock(&my_spinlock);

	/* MUTEX */

	mutex_lock(&my_mutex);
	/* I can sleep here */

	buf = kmalloc(50, GFP_KERNEL);
	kfree(buf);

	/* this may fail after some time the system is running */
	buf = (void *)__get_free_pages(GFP_KERNEL, 10);
	free_pages((unsigned long)buf, 10);

	/* this will likely fail on some architectures (i386) */
	buf = vmalloc(128 << 20);
	vfree(buf);
	mutex_unlock(&my_mutex);

	/* don't allow loading of the module */
	return -EIO;
}

static void my_exit(void)
{
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
