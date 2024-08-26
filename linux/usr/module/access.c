/*******************************************************************
 * access.c
 *
 * Copyright (c) 2020 HEIG-VD, REDS Institute
 *******************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>

#include <asm/io.h>

static int access_init(void) {

	printk("access: small driver for accessing I/O ...\n");

	return 0;
}

static void access_exit(void) {

	printk("access: bye bye!\n");
}

module_init(access_init);
module_exit(access_exit);

MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");

