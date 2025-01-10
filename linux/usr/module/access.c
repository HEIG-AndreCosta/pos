/*******************************************************************
 * access.c
 *
 * Copyright (c) 2020 HEIG-VD, REDS Institute
 *******************************************************************/

#include <linux/string.h>
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
#include <linux/input.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include "rpisense.h"

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

const static int keys[] = { KEY_ENTER, KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN };

struct led_data {
	struct led_classdev cdev;
	int led_no;
};

struct vext_data {
	void *base_ptr;
	struct led_data leds[NO_LEDS];
	uint8_t led_status;
	bool is_virt32; // It's either virt32 or rpi4
	struct input_dev *input_dev;
	uint8_t key_pressed_index;
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
	if (drv_data->is_virt32) {
		iowrite8(drv_data->led_status, drv_data->base_ptr + LED_OFFSET);
	} else {
		display_led(data->led_no, brightness);
	}
}

static irqreturn_t on_switch_press_top_half(int irq, void *raw)
{
	struct vext_data *data = (struct vext_data *)raw;
	uint8_t irq_reg = ioread8(data->base_ptr + IRQ_CTRL_REG_OFFSET);
	uint8_t btn_pressed = (irq_reg & VEXT_IRQ_CTRL_BTN_MASK) >>
			      VEXT_IRQ_CTRL_BTN_SHIFT;
	writeb(0x81, data->base_ptr + IRQ_CTRL_REG_OFFSET);
	data->key_pressed_index = btn_pressed;
	return IRQ_WAKE_THREAD;
}

static void propagate_event(struct vext_data *data)
{
	input_report_key(data->input_dev, keys[data->key_pressed_index], 1);
	input_report_key(data->input_dev, keys[data->key_pressed_index], 0);
	input_sync(data->input_dev);
}

static irqreturn_t on_switch_press_bottom_half(int irq, void *raw)
{
	struct vext_data *data = (struct vext_data *)raw;
	propagate_event(data);
	return IRQ_HANDLED;
}
static void joystick_handler(struct platform_device *pdev, int key)
{
	struct vext_data *data = platform_get_drvdata(pdev);
	if (key == 0) {
		return;
	}
	switch (key) {
	case CENTER:
		data->key_pressed_index = 0;
		break;
	case LEFT:
		data->key_pressed_index = 1;
		break;
	case UP:
		data->key_pressed_index = 2;
		break;
	case RIGHT:
		data->key_pressed_index = 3;
		break;
	case DOWN:
		data->key_pressed_index = 4;
		break;
	}
	propagate_event(data);
}

static int access_probe(struct platform_device *pdev)
{
	struct device_node *dt_node;
	const char *platform_type;
	struct vext_data *priv;
	struct resource *iores;
	struct input_dev *input_dev;
	int i, ret, irq;

	priv = kzalloc(sizeof(struct vext_data), GFP_KERNEL);
	BUG_ON(!priv);

	input_dev = input_allocate_device();
	BUG_ON(!input_dev);
	priv->input_dev = input_dev;
	platform_set_drvdata(pdev, priv);

	dt_node = of_find_node_by_name(NULL, DEVICE_NAME);
	BUG_ON(!dt_node);

	ret = of_property_read_string(dt_node, "platform_type", &platform_type);
	BUG_ON(ret);

	priv->is_virt32 = strcmp(platform_type, "virt32") == 0;

	if (priv->is_virt32) {
		iores = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		BUG_ON(!iores);

		priv->base_ptr =
			ioremap(iores->start, iores->end - iores->start + 1);
		BUG_ON(!priv->base_ptr);
	}

	if (priv->is_virt32) {
		irq = platform_get_irq(pdev, 0);
		printk("Requesting IRQ\n");
		ret = request_threaded_irq(irq, on_switch_press_top_half,
					   on_switch_press_bottom_half,
					   IRQF_TRIGGER_HIGH, DEVICE_NAME,
					   priv);
		if (ret < 0) {
			printk("Couldn't request irq\n");
			kfree(priv);
			return ret;
		}
	}
	input_dev->name = DEVICE_NAME "_input";
	input_dev->dev.parent = &pdev->dev;
	for (i = 0; i < ARRAY_SIZE(keys); ++i) {
		input_set_capability(input_dev, EV_KEY, keys[i]);
	}

	ret = input_register_device(input_dev);
	BUG_ON(ret);

	for (i = 0; i < NO_LEDS; ++i) {
		priv->leds[i].cdev.name = led_names[i];
		priv->leds[i].cdev.brightness_set = led_set;
		priv->leds[i].led_no = i;
		ret = led_classdev_register(&pdev->dev, &priv->leds[i].cdev);
		BUG_ON(ret);
	}
	if (priv->is_virt32) {
		writeb(0x80, priv->base_ptr + IRQ_CTRL_REG_OFFSET);
	} else {
		rpisense_init();
		rpisense_joystick_handler_register(pdev, joystick_handler);
	}

	return 0;
}
static int access_remove(struct platform_device *pdev)
{
	struct vext_data *priv = (struct vext_data *)platform_get_drvdata(pdev);
	int i;
	if (!priv) {
		return 0;
	}
	for (i = 0; i < NO_LEDS; ++i) {
		led_classdev_unregister(&priv->leds[i].cdev);
	}
	input_unregister_device(priv->input_dev);
	input_free_device(priv->input_dev);
	if (priv->is_virt32) {
		writeb(0x00, priv->base_ptr + IRQ_CTRL_REG_OFFSET);
		iounmap(priv->base_ptr);
		free_irq(platform_get_irq(pdev, 0), priv);
	}
	kfree(priv);
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
