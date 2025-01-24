#include "linux/mod_devicetable.h"
#include "linux/uaccess.h"
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/regmap.h>

#define DRIVER_NAME	      "lsm9ds1-accel-driver"

#define DEVICE_ID	      0x68

#define REG_WHO_AM_I	      0x0f
#define REG_CTRL1_M	      0x20

#define DATA_RATE_MASK	      0x18
#define DATA_RATE_SHIFT	      3

#define HIGH_PERFORMANCE_MODE 0x20

enum accel_scale {
	FULL_SCALE_2G = 0,
	FULL_SCALE_4G = 2,
	FULL_SCALE_8G = 3,
	FULL_SCALE_16G = 1,
};

struct accel {
	int16_t x;
	int16_t y;
	int16_t z;
	enum accel_scale scale;
};
struct accel_data {
	struct miscdevice miscdev;
	struct i2c_client *client;
};

static ssize_t accel_read(struct file *f, char __user *buf, size_t count,
			  loff_t *ppos)
{
	struct accel acc;
	uint8_t ret;
	struct accel_data *priv =
		container_of(f->private_data, struct accel_data, miscdev);

	i2c_smbus_read_i2c_block_data(priv->client, 0x28, sizeof(acc),
				      (uint8_t *)&acc);

	ret = i2c_smbus_read_byte_data(priv->client, REG_CTRL1_M);

	acc.scale = (ret >> 3) & 3;

	memcpy(buf, &acc, sizeof(acc));

	return sizeof(acc);
}

static ssize_t accel_write(struct file *f, const char __user *buf, size_t count,
			   loff_t *ppos)
{
	uint8_t ret;
	int err;
	enum accel_scale scale;
	struct accel_data *priv =
		container_of(f->private_data, struct accel_data, miscdev);

	if (count != sizeof(scale)) {
		return 0;
	}

	err = copy_from_user(&scale, buf, sizeof(scale));

	if (err < sizeof(scale)) {
		return 0;
	}

	ret = i2c_smbus_read_byte_data(priv->client, REG_CTRL1_M);

	ret &= ~DATA_RATE_MASK;
	ret |= scale << DATA_RATE_SHIFT;

	i2c_smbus_write_byte_data(priv->client, REG_CTRL1_M, ret);

	return count;
}

const static struct file_operations accel_fops = {
	.owner = THIS_MODULE,
	.read = accel_read,
	.write = accel_write,
};

static const struct regmap_config lsm9ds1_i2c_config = {
	.reg_bits = 8,
	.val_bits = 8,
};

static int accel_probe(struct i2c_client *client,
		       const struct i2c_device_id *id)
{
	uint8_t ret;
	/*struct regmap *regmap;*/
	struct accel_data *priv;
	pr_info("Probe!\n");
	priv = devm_kzalloc(&client->dev, sizeof(struct accel_data),
			    GFP_KERNEL);
	if (!priv) {
		pr_err("Failed to allocate memory\n");
		return -ENOMEM;
	}

	if (id->driver_data != DEVICE_ID) {
		pr_err("Invalid Device ID. Expected (%d), Got (%lu)\n",
		       DEVICE_ID, id->driver_data);
		return -EINVAL;
	}

	priv->miscdev = (struct miscdevice){ .minor = MISC_DYNAMIC_MINOR,
					     .name = DRIVER_NAME,
					     .fops = &accel_fops };
	priv->client = client;
	/*regmap = devm_regmap_init_i2c(client, &lsm9ds1_i2c_config);*/
	/**/
	/*if (IS_ERR(regmap)) {*/
	/*	dev_err(&client->dev, "Failed to register i2c regmap %d\n",*/
	/*		(int)PTR_ERR(regmap));*/
	/*	return PTR_ERR(regmap);*/
	/*}*/

	// Read the device ID
	ret = i2c_smbus_read_byte_data(client, REG_WHO_AM_I);

	if (ret != DEVICE_ID) {
		pr_err("Invalid WHO_AM_I register value. Expected (%d), Got (%hhu)\n",
		       DEVICE_ID, ret);
		kfree(priv);
		return -EINVAL;
	}

	// Setup high performance mode
	ret = i2c_smbus_write_byte_data(client, REG_CTRL1_M,
					HIGH_PERFORMANCE_MODE);
	if (ret != 0) {
		pr_err("Write Failed(%d)}\n", ret);
		kfree(priv);
		return -EINVAL;
	}

	pr_info("Now registering Misc Device\n");

	return misc_register(&priv->miscdev);
}

static int accel_remove(struct i2c_client *client)
{
	struct accel_data *priv = client->dev.driver_data;
	misc_deregister(&priv->miscdev);
	return 0;
}
static const struct of_device_id of_ids[] = {
	{ .compatible = "st,lsm9ds1-accel", .data = "lsm9ds1-accel" },
	{}
};

static const struct i2c_device_id accel_ids[] = { { "lsm9ds1-accel",
						    DEVICE_ID },
						  {} };

MODULE_DEVICE_TABLE(i2c, accel_ids);

static struct i2c_driver accel_driver = {
	.driver = { .name = DRIVER_NAME, .of_match_table = of_ids },
	.probe = accel_probe,
	.remove = accel_remove,
	.id_table = accel_ids,
};

module_i2c_driver(accel_driver);
MODULE_LICENSE("GPL v2");
