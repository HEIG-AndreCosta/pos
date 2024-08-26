/*******************************************************************
 * access.c
 *
 * Copyright (c) 2020 HEIG-VD, REDS Institute
 *******************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/device.h>
#include <linux/fs.h>

#include <asm/io.h>

#include <linux/kobject.h>
#include <linux/kdev_t.h>

static struct kobject *kobj;
static struct kset *kset;

char __str[80];

struct obj {
    struct kobject kobj;
    int attr_1;
};

ssize_t	attr_show(struct kobject *kobj, struct kobj_attribute *attr, char *str) {

	printk("Current str: %s\n", __str);

	return strlen(__str)+1;
}


ssize_t	attr_store(struct kobject *kobj, struct kobj_attribute *attr, const char *str, size_t len) {

	printk("Got str: %s\n", str);

	strcpy(__str, str);

	return len;
}

static struct kobj_attribute vext_attr = __ATTR(vext, 0664, attr_show, attr_store);


static int access_init(void) {
	int ret;
	struct device *dev;
	struct class *c1;
	dev_t vext_dev;

	printk("access: small driver for accessing I/O ...\n");
#if 0
	vext_dev = MKDEV(126, 0);

	register_chrdev_region(vext_dev, 1, "vext");
#endif
#if 0
	c1 = class_create(THIS_MODULE, "vextclass");

    dev = device_create(c1, NULL, vext_dev, NULL, "vext");

    kobject_uevent(&dev->kobj, KOBJ_ADD);
#endif
	kobj = kobject_create_and_add("vext", kernel_kobj);

	sysfs_create_file(kobj, &vext_attr.attr);

	/* Send a uevent */
	kobject_uevent(&dev->kobj, KOBJ_ADD);

	return 0;
}

static void access_exit(void) {

	printk("access: bye bye!\n");
}

module_init(access_init);
module_exit(access_exit);

MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");

