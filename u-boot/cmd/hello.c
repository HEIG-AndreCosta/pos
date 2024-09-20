// SPDX-License-Identifier: GPL-2.0+
/*
 * André Costa 2024
 */

#include <common.h>
#include <command.h>
#include <timestamp.h>
#include <version.h>
#include <version_string.h>
#include <linux/compiler.h>
#ifdef CONFIG_SYS_COREBOOT
#include <asm/cb_sysinfo.h>
#endif

const char *hello_str = "Hello My Cutie Pie!";

static int do_hello(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	printf("%s\n", hello_str);
	return 0;
}

U_BOOT_CMD(hello, 1, 1, do_hello, "Print a warm and welcome message", "");
