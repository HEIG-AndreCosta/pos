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

#ifndef VEXT_CLCD_H
#define VEXT_CLCD_H


/* Control commands */
#define CMD_BUSY_ADDR   0x0100
#define CMD_DDRAM_ADDR  0x0080
#define CMD_CGRAM_ADDR  0x0040
#define CMD_FCT_SET     0x0020
#define CMD_SHIFT       0x0010
#define CMD_ON_CTRL     0x0008
#define CMD_ENTRY_MODE  0x0004
#define CMD_HOME        0x0002
#define CMD_CLEAR       0x0001

#define CMD_BUSY_ADDR_BIT   8
#define CMD_DDRAM_ADDR_BIT  7
#define CMD_CGRAM_ADDR_BIT  6
#define CMD_FCT_SET_BIT     5
#define CMD_SHIFT_BIT       4
#define CMD_ON_CTRL_BIT     3
#define CMD_ENTRY_MODE_BIT  2
#define CMD_HOME_BIT        1
#define CMD_CLEAR_BIT       0
#define CMD_NO_BIT         -1

/* Data commands */
#define CMD_WR_RAM      0x0200
#define CMD_RD_RAM      0x0300


#define START_MASK      0x0400
#define RS_MASK         0x0200
#define RW_MASK         0x0100

#define DDRAM_SPACE     0
#define CGRAM_SPACE     1
#define DDRAM_ADDR_MASK 0x7F
#define CGRAM_ADDR_MASK 0x3F

#define CLCD_IF_READY 0x0100

#define ON_CTRL_DISP_MASK       0x04
#define ON_CTRL_CURS_MASK       0x02
#define ON_CTRL_BLINK_MASK      0x01

#define WIDTH 20
#define HEIGHT 4

typedef struct LCDState {

  int initialized;
  int cur_addr_space;         /* address in DDRAM (0), address in CGRAM (1) */

  /* Internal state of LCD device */
  uint16_t shadow_status;
  int display_on;
  int cursor_on;
  int blink_on;

  uint8_t ac;                 /* Current RAM address (DDRAM or CGRAM) */

  uint8_t ddram[104];            /* Display data RAM stored linearly ! */
  uint8_t cgram[64];             /* Custom graphics RAM */

} LCDState_t;

int vext_clcd_init(void);
uint16_t vext_clcd_status_process(void);
int vext_clcd_cmd_process(uint16_t cmd_word);

#endif /* VEXT_CLCD_H */
