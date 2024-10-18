/*******************************************************************
 * access.c
 *
 * Copyright (c) 2020 HEIG-VD, REDS Institute
 *******************************************************************/

#include "linux/export.h"
#include "linux/ioport.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/io.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#define LED_OFFSET 0x3A
struct vext_data {
	void *base_ptr;
};

static int access_probe(struct platform_device *pdev)
{
	struct resource *iores;
	struct vext_data priv;
	/*struct vext_data *priv = kmalloc(sizeof(struct vext_data), GFP_KERNEL);*/
	/**/
	/*if (!priv) {*/
	/*	return -ENOMEM;*/
	/*}*/

	iores = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv.base_ptr = ioremap(iores->start, iores->end - iores->start + 1);

	if (!priv.base_ptr) {
		printk("ERROR Remaping vext memory\n");
		return 1;
	}
	printk("Turning LEDS 1-4 ON\n");
	writeb(0x1E, priv.base_ptr + LED_OFFSET);
	return 0;
}
static const struct of_device_id access_of_ids[] = { { .compatible =
							       "pos,vext" },
						     {} };

static struct platform_driver access_driver = {
	.probe = access_probe,
	.driver = { .name = "access",
		    .of_match_table = access_of_ids,
		    .owner = THIS_MODULE }
};

static int access_init(void)
{
	platform_driver_register(&access_driver);
	/*printk("Hello!\n");*/

	return 0;
}

static void access_exit(void)
{
	/*if (led_ptr) {*/
	/*	printk("Turning LEDS 1-4 OFF\n");*/
	/*	writew(0, led_ptr);*/
	/*}*/
	/*printk("Bye bye\n");*/
	platform_driver_unregister(&access_driver);
}
module_init(access_init);
module_exit(access_exit);

MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");
