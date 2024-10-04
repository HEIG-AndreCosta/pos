/*******************************************************************
 * access.c
 *
 * Copyright (c) 2020 HEIG-VD, REDS Institute
 *******************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/io.h>

static void *led_ptr = NULL;
static int access_init(void)
{
	printk("Hello!\n");
	led_ptr = ioremap(0x20000000, 8);
	if (!led_ptr) {
		printk("ERROR Remaping led memory\n");
		return 1;
	}
	printk("Turning LEDS 1-4 ON\n");
	writeb(0x1E, led_ptr);

	return 0;
}

static void access_exit(void)
{
	if (led_ptr) {
		printk("Turning LEDS 1-4 OFF\n");
		writew(0, led_ptr);
	}
	printk("Bye bye\n");
}

module_init(access_init);
module_exit(access_exit);

MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");
