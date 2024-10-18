/*******************************************************************
 * access.c
 *
 * Copyright (c) 2020 HEIG-VD, REDS Institute
 *******************************************************************/

#include "rpisense.h"
#include "linux/export.h"
#include "linux/ioport.h"
#include "linux/irqreturn.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/io.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

#define DEVICE_NAME		"vext"
#define SWITCH_OFFSET		0x12
#define IRQ_CTRL_REG_OFFSET	0x18
#define LED_OFFSET		0x3A

#define VEXT_TIMER_ENABLE_MASK	(0x1)
#define VEXT_TIMER_CLEAR_MASK	(0x2)
#define VEXT_IRQ_CTRL_BTN_MASK	(0xE)
#define VEXT_IRQ_CTRL_BTN_SHIFT (0x1)

struct vext_data {
	void *base_ptr;
};
#if 0
static irqreturn_t on_switch_press_top_half(int irq, void *raw)
{
	struct vext_data *data = (struct vext_data *)raw;
	uint8_t irq_reg = readb(data->base_ptr + IRQ_CTRL_REG_OFFSET);
	uint8_t btn_pressed = (irq_reg & VEXT_IRQ_CTRL_BTN_MASK) >>
			      VEXT_IRQ_CTRL_BTN_SHIFT;

	writeb(0x81, data->base_ptr + IRQ_CTRL_REG_OFFSET);
	printk("IRQ Button: %d\n", btn_pressed);
	return IRQ_HANDLED;
}
#endif

static int access_probe(struct platform_device *pdev)
{
	int i;
#if 0
	struct resource *iores;
	int irq;
	int ret;
	struct vext_data *priv = kmalloc(sizeof(struct vext_data), GFP_KERNEL);

	if (!priv) {
		printk("Couldn't allocate memory for vext data\n");
		;
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, priv);

	iores = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq = platform_get_irq(pdev, 0);

	printk("Requesting IRQ\n");
	ret = request_irq(irq, on_switch_press_top_half, IRQF_TRIGGER_HIGH,
			  DEVICE_NAME, priv);
	if (ret < 0) {
		printk("Couldn't request irq\n");
		kfree(priv);
		return ret;
	}
	priv->base_ptr = ioremap(iores->start, iores->end - iores->start + 1);

	if (!priv->base_ptr) {
		printk("ERROR Remaping vext memory\n");
		return 1;
	}
	printk("Turning LEDS 1-4 ON\n");
	writeb(0x1E, priv->base_ptr + LED_OFFSET);
	//enable interrupts
	printk("Enabling interrupts\n");
	writeb(0x80, priv->base_ptr + IRQ_CTRL_REG_OFFSET);
#endif
	rpisense_init();
	for (i = 0; i < 5; i += 2) {
		display_led(i, 1);
	}

	return 0;
}
static int access_remove(struct platform_device *pdev)
{
	struct vext_data *priv = (struct vext_data *)platform_get_drvdata(pdev);
	if (!priv) {
		return 0;
	}
	kfree(priv->base_ptr);
	return 0;
}
static const struct of_device_id access_of_ids[] = { { .compatible =
							       "pos,vext" },
						     {} };

static struct platform_driver access_driver = {
	.probe = access_probe,
	.remove = access_remove,
	.driver = { .name = DEVICE_NAME,
		    .of_match_table = access_of_ids,
		    .owner = THIS_MODULE }
};

module_platform_driver(access_driver);

MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");
