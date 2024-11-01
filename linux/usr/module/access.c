/*******************************************************************
 * access.c
 *
 * Copyright (c) 2020 HEIG-VD, REDS Institute
 *******************************************************************/

#include "linux/types.h"
#include <linux/export.h>
#include <linux/ioport.h>
#include <linux/irqreturn.h>
#include <linux/leds.h>
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

#define NO_LEDS			5

const static char *led_names[NO_LEDS] = { "mydev_led0", "mydev_led1",
					  "mydev_led2", "mydev_led3",
					  "mydev_led4" };

struct led_data {
	struct led_classdev cdev;
	int led_no;
};

struct vext_data {
	void *base_ptr;
	struct led_data leds[NO_LEDS];
	uint8_t led_status;
};

void led_set(struct led_classdev *raw_dev, enum led_brightness brightness)
{
	struct led_data *data = container_of(raw_dev, struct led_data, cdev);
	struct platform_device *pdev =
		container_of(raw_dev->dev->parent, struct platform_device, dev);
	struct vext_data *drv_data = platform_get_drvdata(pdev);
	const uint8_t mask = 1 << data->led_no;

	if (brightness) {
		drv_data->led_status |= mask;
	} else {
		drv_data->led_status &= ~mask;
	}
	iowrite8(drv_data->led_status, drv_data->base_ptr + LED_OFFSET);
}
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
	int i, ret;
	struct resource *iores;
	struct vext_data *priv = kzalloc(sizeof(struct vext_data), GFP_KERNEL);
	BUG_ON(!priv);

	platform_set_drvdata(pdev, priv);

	iores = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	BUG_ON(!iores);

	priv->base_ptr = ioremap(iores->start, iores->end - iores->start + 1);
	BUG_ON(!priv->base_ptr);

#if 0
	irq = platform_get_irq(pdev, 0);
	printk("Requesting IRQ\n");
	ret = request_irq(irq, on_switch_press_top_half, IRQF_TRIGGER_HIGH,
			  DEVICE_NAME, priv);
	if (ret < 0) {
		printk("Couldn't request irq\n");
		kfree(priv);
		return ret;
	}
#endif

	for (i = 0; i < NO_LEDS; ++i) {
		priv->leds[i].cdev.name = led_names[i];
		priv->leds[i].cdev.brightness_set = led_set;
		priv->leds[i].led_no = i;
		ret = led_classdev_register(&pdev->dev, &priv->leds[i].cdev);
		BUG_ON(ret);
	}
#if 0
	printk("Turning LEDS 1-4 ON\n");
	writeb(0x1E, priv->base_ptr + LED_OFFSET);
	//enable interrupts
	printk("Enabling interrupts\n");
	writeb(0x80, priv->base_ptr + IRQ_CTRL_REG_OFFSET);
#endif
#if 0
	rpisense_init();
	for (i = 0; i < 5; i += 2) {
		display_led(i, 1);
	}
#endif

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
