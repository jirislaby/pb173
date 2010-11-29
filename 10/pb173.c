#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/cdev.h>

#define PHANTOM_MAX_MINORS	8

#define PHN_CONT		16     /* control byte in iaddr space */
#define PHN_CONTROL		0x6     /* control byte in iaddr space */
#define PHN_IRQCTL		0x4c    /* irq control in caddr space */
#define  PHN_CTL_IRQ		0x10    /*   is irq enabled */

static DEFINE_MUTEX(phantom_mutex);
static struct class *phantom_class;
static int phantom_major;

struct phantom_device {
	unsigned int opened;
	void __iomem *caddr;
	u32 __iomem *iaddr;
	unsigned long status;

	struct cdev cdev;
	struct mutex open_lock;
};

struct phm_reg { 
	__u32 reg; 
	__u32 value; 
}; 

static unsigned char phantom_devices[PHANTOM_MAX_MINORS];

/*
 * File ops
 */

static long phantom_write(struct file *file, const char __user *buf,
		size_t count, loff_t *offp)
{
	struct phantom_device *dev = file->private_data;
	struct phm_reg r;

	if (count != sizeof(r))
		return -EINVAL;

	memcpy(&r, buf, sizeof(r));

	iowrite32(r.value, dev->iaddr + r.reg);
	ioread32(dev->iaddr); /* PCI posting */

	return count;
}

static int phantom_open(struct inode *inode, struct file *file)
{
	struct phantom_device *dev = container_of(inode->i_cdev,
			struct phantom_device, cdev);

	mutex_lock(&phantom_mutex);

	if (mutex_lock_interruptible(&dev->open_lock)) {
		mutex_unlock(&phantom_mutex);
		return -ERESTARTSYS;
	}

	if (dev->opened) {
		mutex_unlock(&dev->open_lock);
		mutex_unlock(&phantom_mutex);
		return -EBUSY;
	}

	file->private_data = dev;

	dev->opened++;
	mutex_unlock(&dev->open_lock);
	mutex_unlock(&phantom_mutex);

	return 0;
}

static int phantom_release(struct inode *inode, struct file *file)
{
	struct phantom_device *dev = file->private_data;

	mutex_lock(&dev->open_lock);
	dev->opened = 0;
	mutex_unlock(&dev->open_lock);

	return 0;
}

static const struct file_operations phantom_file_ops = {
	.open = phantom_open,
	.release = phantom_release,
	.write = phantom_write,
};

static irqreturn_t phantom_isr(int irq, void *data, struct pt_regs *regs)
{
	struct phantom_device *dev = data;
	u32 ctl;

	ctl = ioread32(dev->iaddr + PHN_CONTROL);
	if (!(ctl & PHN_CTL_IRQ))
		return IRQ_NONE;

	if (dev->iaddr[PHN_CONTROL] << PHN_CONT && printk_ratelimit())
		printk(KERN_ERR "phantom: wrong interrupt\n");

	iowrite32(0, dev->iaddr);
	ioread32(dev->iaddr); /* PCI posting */

	return IRQ_HANDLED;
}

/*
 * Init and deinit driver
 */

static unsigned int __devinit phantom_get_free(void)
{
	unsigned int i;

	for (i = 0; i < PHANTOM_MAX_MINORS; i++)
		if (phantom_devices[i] == 0)
			break;

	return i;
}

static int phantom_probe(struct pci_dev *pdev,
		const struct pci_device_id *pci_id)
{
	struct phantom_device *pht;
	unsigned int minor;
	int retval;

	retval = pci_enable_device(pdev);
	if (retval) {
		dev_err(&pdev->dev, "pci_enable_device failed!\n");
		goto err;
	}

	minor = phantom_get_free();
	if (minor == PHANTOM_MAX_MINORS) {
		dev_err(&pdev->dev, "too many devices found!\n");
		retval = -EIO;
		goto err_dis;
	}

	phantom_devices[minor] = 1;

	retval = pci_request_regions(pdev, "phantom");
	if (retval) {
		dev_err(&pdev->dev, "pci_request_regions failed!\n");
		goto err_null;
	}

	retval = -ENOMEM;
	pht = kzalloc(sizeof(*pht), GFP_KERNEL);
	if (pht == NULL) {
		dev_err(&pdev->dev, "unable to allocate device\n");
		goto err_reg;
	}

	pht->caddr = pci_iomap(pdev, 0, 0);
	if (pht->caddr == NULL) {
		dev_err(&pdev->dev, "can't remap conf space\n");
		goto err_fr;
	}
	pht->iaddr = pci_iomap(pdev, 2, 0);
	if (pht->iaddr == NULL) {
		dev_err(&pdev->dev, "can't remap input space\n");
		goto err_unmc;
	}

	mutex_init(&pht->open_lock);
	cdev_init(&pht->cdev, &phantom_file_ops);
	pht->cdev.owner = THIS_MODULE;

	iowrite32(0, pht->caddr + PHN_IRQCTL);
	ioread32(pht->caddr + PHN_IRQCTL); /* PCI posting */
	retval = request_irq(pdev->irq, phantom_isr, IRQF_SHARED,
			"phantom", pht);
	if (retval) {
		dev_err(&pdev->dev, "can't establish ISR\n");
		goto err_unmi;
	}

	retval = cdev_add(&pht->cdev, MKDEV(phantom_major, minor), 1);
	if (retval) {
		dev_err(&pdev->dev, "chardev registration failed\n");
		goto err_irq;
	}

	if (IS_ERR(device_create(phantom_class, &pdev->dev,
				 MKDEV(phantom_major, minor),
				 "phantom%u", minor)))
		dev_err(&pdev->dev, "can't create device\n");

	pci_set_drvdata(pdev, pht);

	return 0;
err_irq:
	free_irq(pdev->irq, pht);
err_unmi:
	pci_iounmap(pdev, pht->iaddr);
err_unmc:
	pci_iounmap(pdev, pht->caddr);
err_fr:
	kfree(pht);
err_reg:
	pci_release_regions(pdev);
err_null:
	phantom_devices[minor] = 0;
err_dis:
	pci_disable_device(pdev);
err:
	return retval;
}

static void __devexit phantom_remove(struct pci_dev *pdev)
{
	struct phantom_device *pht = pci_get_drvdata(pdev);
	unsigned int minor = MINOR(pht->cdev.dev);

	device_destroy(phantom_class, MKDEV(phantom_major, minor));

	cdev_del(&pht->cdev);

	iowrite32(0, pht->caddr + PHN_IRQCTL);
	ioread32(pht->caddr + PHN_IRQCTL); /* PCI posting */
	free_irq(pdev->irq, pht);

	pci_iounmap(pdev, pht->iaddr);
	pci_iounmap(pdev, pht->caddr);

	kfree(pht);

	pci_release_regions(pdev);

	phantom_devices[minor] = 0;

	pci_disable_device(pdev);
}

static struct pci_device_id phantom_pci_tbl[] __devinitdata = {
	{ .vendor = PCI_VENDOR_ID_PLX, .device = PCI_DEVICE_ID_PLX_9050,
	  .subvendor = PCI_VENDOR_ID_PLX, .subdevice = PCI_DEVICE_ID_PLX_9050,
	  .class = PCI_CLASS_BRIDGE_OTHER << 8, .class_mask = 0xffff00 },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, phantom_pci_tbl);

static struct pci_driver phantom_pci_driver = {
	.name = "phantom",
	.id_table = phantom_pci_tbl,
	.probe = phantom_probe,
	.remove = __devexit_p(phantom_remove),
};

static int __init phantom_init(void)
{
	int retval;
	dev_t dev;

	phantom_class = class_create(THIS_MODULE, "phantom");
	if (IS_ERR(phantom_class)) {
		retval = PTR_ERR(phantom_class);
		printk(KERN_ERR "phantom: can't register phantom class\n");
		goto err;
	}

	retval = alloc_chrdev_region(&dev, 0, PHANTOM_MAX_MINORS, "phantom");
	if (retval) {
		printk(KERN_ERR "phantom: can't register character device\n");
		goto err_class;
	}
	phantom_major = MAJOR(dev);

	retval = pci_register_driver(&phantom_pci_driver);
	if (retval) {
		printk(KERN_ERR "phantom: can't register pci driver\n");
		goto err_unchr;
	}

	return 0;
err_unchr:
	unregister_chrdev_region(dev, PHANTOM_MAX_MINORS);
err_class:
	class_destroy(phantom_class);
err:
	return retval;
}

static void __exit phantom_exit(void)
{
	pci_unregister_driver(&phantom_pci_driver);

	unregister_chrdev_region(MKDEV(phantom_major, 0), PHANTOM_MAX_MINORS);

	class_destroy(phantom_class);
}

module_init(phantom_init);
module_exit(phantom_exit);

MODULE_LICENSE("GPL");
