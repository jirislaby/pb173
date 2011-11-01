#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/types.h>

/* ========================================================================= */

/* physical addresses */
#define DMA_ADDRESS	(0x12345678ULL)
#define PCI_DEVICE_ADDR	(0xffffffff80000000ULL)
#define RAM_ADDRESS	(0x1234ULL)
#define RAM_PFN		(0x1234ULL)

/* TODO: fix the types */
static const unsigned long dma_address = DMA_ADDRESS;
static const unsigned long pci_address = PCI_DEVICE_ADDR;
static const unsigned long ram_address = RAM_ADDRESS;
static const unsigned long ram_pfn = RAM_PFN;

/* ========================================================================= */

/*
 * string encoded as big-endian 64bit numbers
 *
 * The string is split into 8B nibbles. Each nibble is converted to big endian.
 * Hello world -> Hello wo -> 0x48656c6c6f20776f -> 0x6f77206f6c6c6548
 *                rld -> 0x726c64 -> 0x646c720000000000
 * Result: 0x6f77206f6c6c6548, 0x646c720000000000
 */
static __be64 string[] = {
	0x4c696e7578204465,
	0x7669636520447269,
	0x7665727320336500
};

static void decode_and_print(const __be64 *str)
{
	/* TODO: implement the body */
}

/* ========================================================================= */

/*
 * TODO: convert to better error handling
 * return EINVAL and ENOMEM instead of NULL (encoded by ERR_PTR)
 * Do not forget to change the caller!
 */
static void *error_handling(int positive)
{
	char *ret;

	if (positive <= 0)
		return NULL;

	ret = kmalloc(100, GFP_KERNEL);
	if (!ret)
		return NULL;

	return ret;
}

/* ========================================================================= */

static int my_init(void)
{
	void *mem;

	decode_and_print(string);

	mem = error_handling(-1);
	printk(KERN_DEBUG "%s: error_handling returned %p\n", __func__, mem);
	if (mem)
		kfree(mem);

	return -EIO;
}

static void my_exit(void)
{
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
