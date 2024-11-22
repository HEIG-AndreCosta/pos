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
#include "device/device.h"
#include "heap.h"
#include "memory.h"
#include <vfs.h>

#include <asm/io.h>
#include <device/driver.h>

#define LED_OFFSET 0x3A

struct vext_data {
	void *base_addr;
	uint8_t led_status;
};

static int vext_write(int fd, const void *buffer, int count)
{
	struct devclass *dev;
	struct vext_data *priv;
	uint8_t mask;
	printk("Vext Write\n");
	if (count < 2) {
		return 0;
	}
	dev = devclass_by_fd(fd);
	priv = (struct vext_data *)devclass_get_priv(dev);
	mask = 1 << devclass_fd_to_id(fd);

	if (((char *)buffer)[0] == '0') {
		priv->led_status &= ~mask;
	} else {
		priv->led_status |= mask;
	}

	iowrite8(priv->base_addr + LED_OFFSET, priv->led_status);
	return 2;
}

static int vext_read(int fd, void *buffer, int count)
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

static struct file_operations vext_fops = { .write = vext_write,
					    .read = vext_read };

static struct devclass vext_dev = {
	.class = "vextled",
	.id_start = 0,
	.id_end = 4,
	.type = VFS_TYPE_DEV_CHAR,
	.fops = &vext_fops,
};

int vext_init(dev_t *dev, int fdt_offset)
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

	printk("Vext Init\n");
	priv = (struct vext_data *)malloc(sizeof(*priv));
	BUG_ON(!priv);

	/* Register the mydev driver so it can be accessed from user space. */
	devclass_register(dev, &vext_dev);
	devclass_set_priv(&vext_dev, priv);

	prop = fdt_get_property(__fdt_addr, fdt_offset, "reg", &prop_len);
	BUG_ON(!prop);
	BUG_ON(prop_len != 2 * sizeof(unsigned long));
	priv->base_addr =
		(void *)io_map(fdt32_to_cpu(((const fdt32_t *)prop->data)[0]),
			       fdt32_to_cpu(((const fdt32_t *)prop->data)[1]));
	BUG_ON(!priv->base_addr);
	return 0;
}

REGISTER_DRIVER_POSTCORE("pos,vext", vext_init);
