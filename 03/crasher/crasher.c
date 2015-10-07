#include <linux/module.h>

int my_init(void)
{
	char *ptr = (void *)5;

	pr_emerg("%s: ALIVE\n", __func__);
	*ptr = 6;
	pr_emerg("%s: DEAD\n", __func__);

	return 0;
}

module_init(my_init);

MODULE_LICENSE("GPL");
