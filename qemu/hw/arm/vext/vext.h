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

#ifndef VEXT_H
#define VEXT_H

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "json/cjson.h"

/* Debug output configuration #define or #undef */
#define VEXT_EMUL_DEBUG 0

#if VEXT_EMUL_DEBUG
#define DBG(fmt, ...)                                      \
	do {                                               \
		printf("(qemu)vext: " fmt, ##__VA_ARGS__); \
	} while (0)
#else
#define DBG(fmt, ...) \
	do {          \
	} while (0)
#endif

#endif /* VEXT_H */
