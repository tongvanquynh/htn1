#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/delay.h>

#define DRIVER_NAME "tcs34725_driver"
#define DEVICE_NAME "tcs34725"
#define CLASS_NAME "tcs34725"

#define TCS34725_ENABLE      0x00
#define TCS34725_ATIME       0x01
#define TCS34725_CONTROL     0x0F
#define TCS34725_ID          0x12
#define TCS34725_CDATAL      0x14

#define TCS34725_CMD_BIT     0x80

#define TCS34725_ENABLE_PON  0x01
#define TCS34725_ENABLE_AEN  0x02

#define TCS34725_IOCTL_MAGIC      't'
#define TCS34725_IOCTL_READ_COLOR _IOR(TCS34725_IOCTL_MAGIC, 1, struct tcs34725_color)

struct tcs34725_color {
	u16 clear;
	u16 red;
	u16 green;
	u16 blue;
};
static struct i2c_client *tcs34725_client;
static struct class* tcs34725_class = NULL;
static struct device* tcs34725_device = NULL;
static int major_number;

static int tcs34725_init_sensor(void)
{
	int ret;

	// Power on
	ret = i2c_smbus_write_byte_data(tcs34725_client, TCS34725_CMD_BIT | TCS34725_ENABLE, TCS34725_ENABLE_PON);
	if (ret < 0) return ret;
	msleep(3);

	// Enable RGBC
	ret = i2c_smbus_write_byte_data(tcs34725_client, TCS34725_CMD_BIT | TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
	if (ret < 0) return ret;

	// Set integration time and gain (defaults)
	ret = i2c_smbus_write_byte_data(tcs34725_client, TCS34725_CMD_BIT | TCS34725_ATIME, 0xFF); // 2.4ms
	if (ret < 0) return ret;

	ret = i2c_smbus_write_byte_data(tcs34725_client, TCS34725_CMD_BIT | TCS34725_CONTROL, 0x00); // 1x gain
	if (ret < 0) return ret;

	msleep(50); // Wait for first integration cycle
	return 0;
}
static int tcs34725_read_color(struct tcs34725_color *color)
{
	u8 buf[8];
	int ret;

	ret = i2c_smbus_read_i2c_block_data(tcs34725_client, TCS34725_CMD_BIT | TCS34725_CDATAL, 8, buf);
	if (ret < 0)
		return ret;

	color->clear = (buf[1] << 8) | buf[0];
	color->red   = (buf[3] << 8) | buf[2];
	color->green = (buf[5] << 8) | buf[4];
	color->blue  = (buf[7] << 8) | buf[6];

	return 0;
}

static long tcs34725_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct tcs34725_color data;

	switch (cmd) {
		case TCS34725_IOCTL_READ_COLOR:
			if (tcs34725_read_color(&data) < 0)
				return -EIO;
			if (copy_to_user((void __user *)arg, &data, sizeof(data)))
				return -EFAULT;
			return 0;
		default:
			return -EINVAL;
	}
}

static int tcs34725_open(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "TCS34725 device opened\n");
	return 0;
}

static int tcs34725_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "TCS34725 device closed\n");
	return 0;
}

static struct file_operations fops = {
	.open = tcs34725_open,
	.unlocked_ioctl = tcs34725_ioctl,
	.release = tcs34725_release,
};

static int tcs34725_probe(struct i2c_client *client)
{
	tcs34725_client = client;

	if (tcs34725_init_sensor() < 0) {
		printk(KERN_ERR "Failed to initialize TCS34725 sensor\n");
		return -EIO;
	}

	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0)
		return major_number;

	tcs34725_class = class_create(CLASS_NAME);
	if (IS_ERR(tcs34725_class)) {
		unregister_chrdev(major_number, DEVICE_NAME);
		return PTR_ERR(tcs34725_class);
	}

	tcs34725_device = device_create(tcs34725_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(tcs34725_device)) {
		class_destroy(tcs34725_class);
		unregister_chrdev(major_number, DEVICE_NAME);
		return PTR_ERR(tcs34725_device);
	}

	printk(KERN_INFO "TCS34725 driver initialized\n");
	return 0;
}

static void tcs34725_remove(struct i2c_client *client)
{
	device_destroy(tcs34725_class, MKDEV(major_number, 0));
	class_unregister(tcs34725_class);
	class_destroy(tcs34725_class);
	unregister_chrdev(major_number, DEVICE_NAME);

	printk(KERN_INFO "TCS34725 driver removed\n");
}

static const struct of_device_id tcs34725_of_match[] = {
	{ .compatible = "taos,tcs34725", },
	{ }
};
MODULE_DEVICE_TABLE(of, tcs34725_of_match);

static struct i2c_driver tcs34725_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = of_match_ptr(tcs34725_of_match),
	},
	.probe = tcs34725_probe,
	.remove = tcs34725_remove,
};

static int __init tcs34725_init(void)
{
	printk(KERN_INFO "Loading TCS34725 driver...\n");
	return i2c_add_driver(&tcs34725_driver);
}

static void __exit tcs34725_exit(void)
{
	i2c_del_driver(&tcs34725_driver);
	printk(KERN_INFO "TCS34725 driver unloaded\n");
}

module_init(tcs34725_init);
module_exit(tcs34725_exit);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("TCS34725 I2C Color Sensor Driver with IOCTL Interface");
MODULE_LICENSE("GPL");
