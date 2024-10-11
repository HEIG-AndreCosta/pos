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
#include "hw/irq.h"
#include "hw/sysbus.h"
#include "qemu/typedefs.h"
#include "qom/object.h"
#include "vext_emul.h"
#include <stdint.h>

#define DEVICE_NAME	       "vext"
#define PUSH_BUT_REG	       0x12
#define IRQ_CTRL_REG	       0x18
#define LED_REG		       0x3A
#define TIMER_CTRL_REG	       0x40
#define SWITCH_MASK	       0x1F
#define IRQ_ENABLE_MASK	       0x80
#define IRQ_SOURCE_MASK	       0x60
#define IRQ_SOURCE_SHIFT       5
#define IRQ_SOURCE_BTN	       0
#define IRQ_SOURCE_LBA	       1
#define IRQ_SOURCE_TIMER       IRQ_SOURCE_LBA
#define IRQ_SOURCE_LBS	       2
#define IRQ_STATUS_MASK	       0x10
#define IRQ_BTN_MASK	       0x0E
#define IRQ_BTN_SHIFT	       0x01
#define IRQ_CLEAR_MASK	       0x01
#define IRQ_SOURCE_SWITCH      0x00
#define TIMER_IRQ_ENABLE_MASK  0x01
#define TIMER_IRQ_PENDING_MASK 0x02

typedef struct {
	SysBusDevice busdev;
	MemoryRegion iomem;
	qemu_irq irq;

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
	uint8_t push_btn;

	/*
		0 IRQ_CLEAR W
			Write '1' to reset to '0' the interrupt line and to enable a new
			interrupt generation 
		3:1 IRQ_BUTTON R
			number of the button that has generated the IRQ (0 to 7).
			This field is valid only when IRQ_status = '1' and
			IRQ_SOURCE="00"
		4 IRQ_STATUS R
			when '1' : an IRQ has been generated
			when '0': no event or IRQ has been cleared
		6:5 IRQ_SOURCE R
			when "00": the IRQ source is the standard_interface
			(pressure on a switch)
			when "01": the IRQ source is the lba_user_interface
			when "10": the IRQ source is the lbs_user_interface
			This field is valid only when IRQ status = '1'
		7 IRQ_ENABLE RW
			Enables the use of IRQ 
	*/

	uint8_t irq_ctrl;

	/*
		0 TIMER_ENABLE RW
			Write '1' to enable a one second timer
		1 ACK_TIMER W
			Write '1' to ack the timer irq
	 */
	uint8_t timer_ctrl;

} vext_state_t;

static uint8_t btn_mask_to_btn_number(uint8_t mask);
static void raise_irq(vext_state_t *instance, uint8_t reason);
static void lower_irq(vext_state_t *instance);
static void handle_btn_irq(vext_state_t *instance, uint8_t new_button_state);
static uint8_t irq_source(vext_state_t *instance);
static int irq_enabled(vext_state_t *instance);
static int irq_pending(vext_state_t *instance);

/*
 * Returns the number of the first button whose corresponding bit is set to '1'
 * Returns 0xFF if input == '0'
 */
uint8_t btn_mask_to_btn_number(uint8_t mask)
{
	for (int i = 0; i < 8; ++i) {
		if (mask & 1) {
			return i;
		}
		mask >>= 1;
	}
	return 0xFF;
}
// Raises an irq
// Source must either be 0 for btn, 1 for lba or 2 for lbs
void raise_irq(vext_state_t *instance, uint8_t source)
{
	DBG("Raising IRQ. Source: %#x\n", source);
	//Indicate the source
	instance->irq_ctrl &= ~IRQ_SOURCE_MASK;
	instance->irq_ctrl |= (source << IRQ_SOURCE_SHIFT);

	//Indicate we generated the irq
	instance->irq_ctrl |= IRQ_STATUS_MASK;

	qemu_irq_raise(instance->irq);
}
void lower_irq(vext_state_t *instance)
{
	DBG("Clearing IRQ\n");

	//Clearing this bit means the irq_status and irq_source are no longer valid
	// as per the documentation so no need to clear those aswell
	instance->irq_ctrl &= ~IRQ_STATUS_MASK;
	qemu_irq_lower(instance->irq);
}
uint8_t irq_source(vext_state_t *instance)
{
	return (instance->irq_ctrl & IRQ_SOURCE_MASK) >> IRQ_SOURCE_SHIFT;
}
int irq_enabled(vext_state_t *instance)
{
	return instance->irq_ctrl & IRQ_ENABLE_MASK;
}
int irq_pending(vext_state_t *instance)
{
	return instance->irq_ctrl & IRQ_STATUS_MASK;
}
void handle_btn_irq(vext_state_t *instance, uint8_t new_button_state)
{
	if (new_button_state == 0) {
		return;
	}
	//Compare with latest value to see which button got us here
	const uint8_t btn_pressed = instance->push_btn ^ new_button_state;
	const uint8_t btn_number = btn_mask_to_btn_number(btn_pressed);
	const uint8_t btn_irq_pending = irq_pending(instance) &&
					irq_source(instance) == IRQ_SOURCE_BTN;

	if (btn_number != 0xFF && irq_enabled(instance) && !btn_irq_pending) {
		DBG("Raising IRQ\n");

		//Indicate which button generated the irq
		instance->irq_ctrl = (instance->irq_ctrl & ~IRQ_BTN_MASK) |
				     (btn_number << IRQ_BTN_SHIFT);

		raise_irq(instance, IRQ_SOURCE_BTN);

		DBG("IRQ CTRL State: %#x\n", instance->irq_ctrl);
	}
}
void vext_process_switch(void *raw, cJSON *packet)
{
	vext_state_t *instance = (vext_state_t *)raw;
	char *device = cJSON_GetObjectItem(packet, "device")->valuestring;
	cJSON *status = cJSON_GetObjectItem(packet, "status");
	if (strcmp(device, "switch") == 0) {
		const uint8_t new_state = status->valueint & SWITCH_MASK;

		handle_btn_irq(instance, new_state);
		instance->push_btn = new_state;
		DBG("Switch State: %#x\n", instance->push_btn);
	}
}
static uint64_t vext_read(void *raw, hwaddr offset, unsigned size)
{
	vext_state_t *instance = (vext_state_t *)raw;
	switch (offset) {
	case PUSH_BUT_REG:
		return instance->push_btn;
	case IRQ_CTRL_REG:
		return instance->irq_ctrl;
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
	case LED_REG:
		instance->leds = value & 0xFF;
		break;
	case IRQ_CTRL_REG:
		if (value & IRQ_CLEAR_MASK) {
			const int timer_irq_pending = instance->timer_ctrl &
						      TIMER_IRQ_PENDING_MASK;
			if (irq_source(instance) == IRQ_SOURCE_BTN) {
				if (timer_irq_pending) {
					raise_irq(instance, IRQ_SOURCE_TIMER);
				} else {
					lower_irq(instance);
				}
			}
		}
		if (value & IRQ_ENABLE_MASK) {
			DBG("Enabling IRQ\n");
		}

		instance->irq_ctrl = (instance->irq_ctrl & ~IRQ_ENABLE_MASK) |
				     (value & IRQ_ENABLE_MASK);
		break;
	case TIMER_CTRL_REG:
		if (value & TIMER_IRQ_PENDING_MASK) {
			if (irq_source(instance) == IRQ_SOURCE_TIMER) {
				instance->timer_ctrl &= ~TIMER_IRQ_PENDING_MASK;
				lower_irq(instance);
			}
		}
		if (irq_enabled(instance) && value & TIMER_IRQ_ENABLE_MASK) {
			sleep(1);
			const int irq_pending = instance->irq_ctrl &
						IRQ_STATUS_MASK;

			instance->timer_ctrl |= TIMER_IRQ_PENDING_MASK;
			if (!irq_pending) {
				raise_irq(instance, IRQ_SOURCE_TIMER);
			}
		}
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
	instance->leds = 0;
	instance->irq_ctrl = 0;
	instance->push_btn = 0;
	sysbus_init_irq(sbd, &instance->irq);
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
