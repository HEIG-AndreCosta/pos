/*
 * Copyright (C) 2013-2014 Romain Bornet <romain.bornet@heig-vd.ch>
 * Copyright (C) 2016-2020 Daniel Rossier <daniel.rossier@heig-vd.ch>
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
#include "vext.h"
#include "exec/cpu-common.h"
#include "exec/memory.h"
#include "hw/sysbus.h"
#include "qemu/typedefs.h"
#include "qom/object.h"
#include "vext_emul.h"
#include <stdint.h>

#define DEVICE_NAME  "vext"
#define PUSH_BUT_REG 0x12
#define IRQ_CTRL_REG 0x18
#define LED_REG	     0x3A
#define SWITCH_MASK  0x1F
typedef struct {
	SysBusDevice busdev;
	MemoryRegion iomem;

	/*
		7:0 LEDx RW 
			write '1' to turn the LEDx ON, write '0' to turn it OFF
	*/
	uint8_t leds;

	/*
		4:0 SWITCHx R 
			'1' switch pressed '0' switch not pressed
		7:5
			Reserved	
	*/
	uint8_t switch_state;

} vext_state_t;
void vext_process_switch(void *raw, cJSON *packet)
{
	vext_state_t *instance = (vext_state_t *)raw;
	char *device = cJSON_GetObjectItem(packet, "device")->valuestring;
	cJSON *status = cJSON_GetObjectItem(packet, "status");
	if (strcmp(device, "switch") == 0) {
		instance->switch_state = status->valueint & SWITCH_MASK;
		printf("Switch State: %#x\n", instance->switch_state);
	}
}
static uint64_t vext_read(void *raw, hwaddr offset, unsigned size)
{
	vext_state_t *instance = (vext_state_t *)raw;
	switch (offset) {
	case PUSH_BUT_REG:
		return instance->switch_state;
	case LED_REG:
		return instance->leds;
	}
	return 0;
}
static void update_frontend(vext_state_t *instance)
{
	cJSON *obj = cJSON_CreateObject(); /* Creation of the JSON object */
	cJSON_AddStringToObject(obj, "device", "led");
	cJSON_AddNumberToObject(obj, "value", instance->leds);
	vext_cmd_post(obj);
}
static void vext_write(void *raw, hwaddr offset, uint64_t value, unsigned size)
{
	vext_state_t *instance = (vext_state_t *)raw;

	switch (offset) {
	case 0x3a:
		instance->leds = value & 0xFF;
		break;
	default:
		return;
	}
	update_frontend(instance);
}
static const MemoryRegionOps vext_ops = {
	.read = vext_read,
	.write = vext_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
};
static void vext_init(Object *obj)
{
	DBG("Initializing vext\n");
	if (vext_emul_init(obj)) {
		DBG("Error Initializing Object\n");
		return;
	}
	DeviceState *dev = DEVICE(obj);
	SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
	vext_state_t *instance = OBJECT_CHECK(vext_state_t, dev, DEVICE_NAME);
	memory_region_init_io(&instance->iomem, obj, &vext_ops, instance,
			      DEVICE_NAME, 0x1000);
	sysbus_init_mmio(sbd, &instance->iomem);
}

static const TypeInfo vext_info = { .name = DEVICE_NAME,
				    .parent = TYPE_SYS_BUS_DEVICE,
				    .instance_size = sizeof(vext_state_t),
				    .instance_init = vext_init };

static void vext_register_types(void)
{
	type_register_static(&vext_info);
}

type_init(vext_register_types)
