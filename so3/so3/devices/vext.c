/*
 * Copyright (C) 2016-2018 Daniel Rossier <daniel.rossier@heig-vd.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/* Vext Driver */

#include "common.h"
#include "completion.h"
#include "device/device.h"
#include "device/fdt.h"
#include "device/irq.h"
#include "heap.h"
#include "libfdt/libfdt.h"
#include "memory.h"
#include <vfs.h>

#include <uapi/linux/input.h>
#include <asm/io.h>
#include <device/driver.h>

#include <device/arch/rpisense.h>

struct vext_data {
	void *base_addr;
	uint8_t led_status;
	uint32_t led_offset;
	uint32_t sw_offset;
	uint32_t irqctrl_offset;
	completion_t sw_read_cplt;
	uint8_t key_index;
	bool is_virt32;
};

#define VEXT_IRQ_CTRL_BTN_MASK	(0xE)
#define VEXT_IRQ_CTRL_BTN_SHIFT (0x1)

const static int KEYS[] = { KEY_ENTER, KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN };

static void on_key_press(struct vext_data *priv, int key)
{
	priv->key_index = key;
	complete(&priv->sw_read_cplt);
}

static irq_return_t irq_handler(int irq, void *data)
{
	struct vext_data *priv = (struct vext_data *)data;
	uint8_t irq_reg = ioread8(priv->base_addr + priv->irqctrl_offset);
	uint8_t btn_pressed = (irq_reg & VEXT_IRQ_CTRL_BTN_MASK) >>
			      VEXT_IRQ_CTRL_BTN_SHIFT;

	iowrite8(priv->base_addr + priv->irqctrl_offset, 0x81);

	on_key_press(priv, btn_pressed);
	printk("IRQ Handler %x\n", btn_pressed);
	return IRQ_BOTTOM;
}
static irq_return_t irq_differed_handler(int irq, void *data)
{
	printk("Differed IRQ Handler\n");
	return IRQ_COMPLETED;
}

static void joystick_handler(void *arg, int key)
{
	int key_index;
	printk("Joystick handler %x\n", key);
	switch (key) {
	case 0x01:
		key_index = 4;
		break;
	case 0x02:
		key_index = 3;
		break;
	case 0x04:
		key_index = 2;
		break;
	case 0x08:
		key_index = 0;
		break;
	case 0x10:
		key_index = 1;
		break;
	default:
		return;
	}
	on_key_press((struct vext_data *)arg, key_index);
}
static int vext_led_write(int fd, const void *buffer, int count)
{
	struct devclass *dev;
	struct vext_data *priv;
	uint8_t mask;
	uint8_t led_no;
	int is_off;
	printk("Vext Write\n");
	if (count < 2) {
		return 0;
	}
	is_off = ((char *)buffer)[0] == '0';
	dev = devclass_by_fd(fd);
	priv = (struct vext_data *)devclass_get_priv(dev);

	led_no = devclass_fd_to_id(fd);
	if (priv->is_virt32) {
		mask = 1 << devclass_fd_to_id(fd);

		if (is_off) {
			priv->led_status &= ~mask;
		} else {
			priv->led_status |= mask;
		}

		iowrite8(priv->base_addr + priv->led_offset, priv->led_status);
	} else {
		display_led(led_no, !is_off);
	}
	return 2;
}

static int vext_led_read(int fd, void *buffer, int count)
{
	struct devclass *dev;
	struct vext_data *priv;
	uint8_t mask;
	printk("Vext Read\n");
	if (count < 2) { // 1byte for value 1 byte for \0
		return 0;
	}
	dev = devclass_by_fd(fd);
	priv = (struct vext_data *)devclass_get_priv(dev);
	mask = 1 << devclass_fd_to_id(fd);

	((char *)buffer)[0] = '0' + ((priv->led_status & mask) == mask);
	((char *)buffer)[1] = '\0';

	return 2;
}

static int vext_switch_read(int fd, void *buffer, int count)
{
	struct devclass *dev;
	struct vext_data *priv;
	struct input_event input;
	printk("Vext Switch Read\n");
	if (count < sizeof(input)) {
		return 0;
	}
	dev = devclass_by_fd(fd);
	priv = (struct vext_data *)devclass_get_priv(dev);

	wait_for_completion(&priv->sw_read_cplt);

	input.type = EV_KEY;
	input.code = KEYS[priv->key_index];
	input.value = 1;
	memcpy(buffer, (void *)&input, sizeof(input));
	return sizeof(input);
}

static struct file_operations vext_led_fops = { .write = vext_led_write,
						.read = vext_led_read };

static struct devclass vext_led_dev = {
	.class = "vextled",
	.id_start = 0,
	.id_end = 4,
	.type = VFS_TYPE_DEV_CHAR,
	.fops = &vext_led_fops,
};

static struct file_operations vext_switch_fops = { .read = vext_switch_read };

static struct devclass vext_switch_dev = {
	.class = "vextswitch",
	.type = VFS_TYPE_DEV_CHAR,
	.fops = &vext_switch_fops,
};

static uint32_t vext_get_register_offset(int fdt_offset, const char *node_name)
{
	uint32_t value;
	fdt_offset = fdt_find_node_by_name(__fdt_addr, fdt_offset, node_name);
	BUG_ON(fdt_offset < 0);
	fdt_property_read_u32(__fdt_addr, fdt_offset, "reg", &value);
	return value;
}

static void common_init(dev_t *dev, struct vext_data *priv)
{
	init_completion(&priv->sw_read_cplt);
	devclass_register(dev, &vext_led_dev);
	devclass_register(dev, &vext_switch_dev);

	devclass_set_priv(&vext_led_dev, priv);
	devclass_set_priv(&vext_switch_dev, priv);
}

int virt_vext_init(dev_t *dev, int fdt_offset)
{
	/*
	 * Le kernel itére sur le device tree et
	 * vérifie la chaine compatible et le status
	 * si le status == "ok" et qu'il trouve un driver avec la même chaine compatbile
	 * la fonction init correspondante est appellé.
	 * La fonction init et la chaine compatible sont les deux paramètres passés 
	 * aux macros `REGISTER_DRIVER_POSTCORE` et `REGISTER_DRIVER_CORE` qui permettent 
	 * au kernel d'être au courant de l'existence du driver
	 */
	struct vext_data *priv;
	const struct fdt_property *prop;
	int prop_len;
	irq_def_t irq;

	printk("Vext Init\n");
	priv = (struct vext_data *)malloc(sizeof(*priv));
	BUG_ON(!priv);

	prop = fdt_get_property(__fdt_addr, fdt_offset, "reg", &prop_len);
	BUG_ON(!prop);
	BUG_ON(prop_len != 2 * sizeof(uint32_t));
	priv->base_addr =
		(void *)io_map(fdt32_to_cpu(((const fdt32_t *)prop->data)[0]),
			       fdt32_to_cpu(((const fdt32_t *)prop->data)[1]));

	BUG_ON(!priv->base_addr);

	priv->is_virt32 = 1;

	priv->led_offset = vext_get_register_offset(fdt_offset, "led");
	priv->sw_offset = vext_get_register_offset(fdt_offset, "switch");
	priv->irqctrl_offset = vext_get_register_offset(fdt_offset, "irqctrl");

	fdt_interrupt_node(fdt_offset, &irq);

	irq_bind(irq.irqnr, irq_handler, irq_differed_handler, priv);

	common_init(dev, priv);

	iowrite8(priv->base_addr + priv->irqctrl_offset, 0x80);

	return 0;
}

int rpi4_vext_init(dev_t *dev, int fdt_offset)
{
	struct vext_data *priv;
	printk("Vext Init\n");
	priv = (struct vext_data *)malloc(sizeof(*priv));
	BUG_ON(!priv);

	priv->is_virt32 = 0;
	common_init(dev, priv);
	rpisense_joystick_handler_register(priv, joystick_handler);

	return 0;
}
void vext_post_init(void)
{
	//FIXME: This only works because there's a 'dummy' implementation of the rpisense for virt32
	rpisense_matrix_off();
}

REGISTER_DRIVER_POSTCORE("pos,vext", virt_vext_init);
REGISTER_DRIVER_POSTCORE("rpi4,vext", rpi4_vext_init);
REGISTER_POST_IRQ_INIT(vext_post_init)
