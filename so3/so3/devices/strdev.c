#include <vfs.h>
#include <device/driver.h>
#include <heap.h>
#include <completion.h>

typedef struct {
	void *base;
	irq_def_t irq_def;
} strdev_t;

completion_t sync;

static int strdev_read(int fd, void *buffer, int count) {

	wait_for_completion(&sync);

	/*... */

	return count;
};

static irq_return_t strdev_isr(int irq, void *dev_id)
{
	/* ... */

	complete(&sync);

	return IRQ_BOTTOM;
}


static int strdev_write(int fd, const void *buffer, int count) {

	printk("## got the message: %s\n", buffer);

	return count;
};

struct file_operations strdev_fops = {
	.write = strdev_write,
	.read = strdev_read
};

struct devclass strdev_dev = {
	.class = "strdev",
	.type = VFS_TYPE_DEV_CHAR,
	.fops = &strdev_fops,
};

int strdev_init(dev_t *dev, int fdt_offset) {
	strdev_t *strdev;

	strdev = malloc(sizeof(strdev_t));
	BUG_ON(!strdev);

	/* Register the mydev driver so it can be accessed from user space. */
	devclass_register(dev, &strdev_dev);

	fdt_interrupt_node(fdt_offset, &strdev->irq_def);

	return 0;
}


REGISTER_DRIVER_POSTCORE("arm,strdev", strdev_init);
