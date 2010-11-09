#include <linux/io.h>
#include <linux/module.h>
#include <linux/delay.h>

static void beep(unsigned int hz)
{
	u8 enable;

	if (!hz) {
		enable = 0x00;		/* Turn off speaker */
	} else {
		u16 div = 1193181/hz;

		outb(0xb6, 0x43);	/* Ctr 2, squarewave, load, binary */
		udelay(10);
		outb(div, 0x42);	/* LSB of counter */
		udelay(10);
		outb(div >> 8, 0x42);	/* MSB of counter */
		udelay(10);

		enable = 0x03;		/* Turn on speaker */
	}
	inb(0x61);		/* Dummy read of System Control Port B */
	udelay(10);
	outb(enable, 0x61);	/* Enable timer 2 output to speaker */
	udelay(10);
}

static int my_init(void)
{
	beep(1000);
	msleep(500);
	beep(1500);
	msleep(500);
	beep(1800);
	msleep(500);
	beep(300);
	msleep(500);
	beep(0);

	return -EIO;
}

static void my_exit(void)
{
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
