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

#ifndef VEXT_EMUL_H
#define VEXT_EMUL_H

#include "json/cjson.h"

#define TCP_PORT  4442

#define DEV_LED 	"led"
#define DEV_LCD 	"lcd"
#define DEV_SEVEN_SEG	"7seg"
#define DEV_SWITCH	"switch"

#define SET		"set"
#define UPDATE		"update"
#define CLEAR		"clear"

void *vext_cmd_post(cJSON *packet);
void vext_process_switch(void *opaque, cJSON *packet);

int vext_emul_init(Object *obj);
int vext_emul_exit(void);


#endif /* VEXT_EMUL_H */
