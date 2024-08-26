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

/* Logic based on proposed Qemu patch:
 * http://lists.gnu.org/archive/html/qemu-devel/2009-07/msg01179.html */

#include "vext.h"
#include "vext_clcd.h"
#include "vext_emul.h"

#include "json/cjson.h"

LCDState_t state;

int qemu_fls(int i);

int qemu_fls(int i)
{
    return 32 - clz32(i);
}


/*
 * vext_clcd_cmd_process
 *
 * Emulate the LCD4x20 management.
 * This function is called by write access to the LCD register.
 */
int vext_clcd_cmd_process(uint16_t cmd_word) {
	cJSON *root;

	int control;
	int high_bit_cmd;
	int res = 0;

	if ((cmd_word & START_MASK) == 0)
		return 0;

	control = !(cmd_word & RS_MASK); /* RS bit/pin indicates control(0) or data(1) */

	if (control) {
		DBG("%s: control\n", __FUNCTION__);

		high_bit_cmd = qemu_fls(cmd_word & ~START_MASK) - 1;

		switch (high_bit_cmd) {
		case CMD_BUSY_ADDR_BIT:
			DBG("CMD_BUSY_ADDR_BIT\n");
			state.shadow_status = CLCD_IF_READY | state.ac;
			break;
		case CMD_DDRAM_ADDR_BIT:
			DBG("CMD_DDRAM_ADDR_BIT\n");
			state.ac = cmd_word & DDRAM_ADDR_MASK;
			state.cur_addr_space = DDRAM_SPACE;
			break;
		case CMD_CGRAM_ADDR_BIT:
			DBG("CMD_CGRAM_ADDR_BIT\n");
			state.ac = cmd_word & CGRAM_ADDR_MASK;
			state.cur_addr_space = CGRAM_SPACE;
			break;
		case CMD_ON_CTRL_BIT:
			DBG("CMD_ON_CTRL_BIT\n");
			/* Cursor and blinking not implemented */
			state.display_on = ((cmd_word & ON_CTRL_DISP_MASK) != 0);
			state.cursor_on = ((cmd_word & ON_CTRL_CURS_MASK) != 0);
			state.blink_on = ((cmd_word & ON_CTRL_BLINK_MASK) != 0);
			break;
		case CMD_HOME_BIT:
			DBG("CMD_HOME_BIT\n");
			state.ac = 0;
			/* Unimplemented: cursor handling */
			/* vext_clcd_update_display(); update display only when cursor handling is implemented */
			break;
		case CMD_CLEAR_BIT:
			DBG("CMD_CLEAR_BIT\n");
			state.ac = 0;
			memset(state.ddram, ' ', sizeof(state.ddram));

			root = cJSON_CreateObject();

			cJSON_AddStringToObject(root, "perif", DEV_LCD);
			cJSON_AddStringToObject(root, "action", UPDATE);
			cJSON_AddStringToObject(root, "ddram", state.ddram);

			vext_cmd_post(root);
			/* Unimplemented: cursor handling */
			break;
		case CMD_FCT_SET_BIT:
			DBG("CMD_FCT_SET_BIT\n");
			/* Unimplemented */
			break;
		case CMD_SHIFT_BIT:
			DBG("CMD_SHIFT_BIT\n");
			/* Unimplemented */
			break;
		case CMD_ENTRY_MODE_BIT:
			DBG("CMD_ENTRY_MODE_BIT\n");
			/* Unimplemented */
			break;
		case CMD_NO_BIT:
			printf("%s/%d: invalid LCD command\n", __FUNCTION__,
					__LINE__);
			res = -1;
			break;
		default:
			printf("%s/%d: unknown LCD command 0x%08X\n",
					__FUNCTION__, __LINE__, cmd_word);
			res = -1;
			break;
		}
	} else /* Data operation */
	if (state.cur_addr_space == DDRAM_SPACE) {
		DBG("%s: data\n", __FUNCTION__);

		state.ddram[state.ac] = (uint8_t) (cmd_word & 0x00FF);
		state.ac = (state.ac + 1) % 104;

		root = cJSON_CreateObject();

		cJSON_AddStringToObject(root, "device", DEV_LCD);
		cJSON_AddStringToObject(root, "command", UPDATE);
		cJSON_AddStringToObject(root, "ddram", state.ddram);

		vext_cmd_post(root);
	} else {
		state.cgram[state.ac] = (uint8_t) (cmd_word & 0x00FF);
		state.ac = (state.ac + 1) % 64;
	}

	return res;
}

/*
 * vext_clcd_status_process
 *
 * Emulate the FPGA contents related to the LCD4x20 management.
 * This function is called by write access to the LCD register.
 */
uint16_t vext_clcd_status_process(void) {
	return state.shadow_status;
}

int vext_clcd_init(void) {
	static int dev_initialized = 0;

	/* One single instance is allowed */
	if (dev_initialized)
		return -1;

	state.ac = 0;
	state.display_on = 0;
	state.cursor_on = 0;
	state.blink_on = 0;

	/* Fill Display RAM with space characters */
	memset(state.ddram, ' ', sizeof(state.ddram));
	memset(state.cgram, 0, sizeof(state.cgram));

	state.cur_addr_space = DDRAM_SPACE;
	state.shadow_status = CLCD_IF_READY;

	/* Internal initialization status of LCD HW */
	state.initialized = 1;

	/* Qemu device initialized */
	dev_initialized = 1;

	return 0;
}

