#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/io.h>        //含有iomap函数iounmap函数
#include <asm/uaccess.h>//含有copy_from_user函数
#include <linux/device.h>//含有类相关的处理函数


volatile unsigned long *gp25con = NULL;
volatile unsigned long *gp25dat = NULL;

static struct class *led_class;
static struct class_device	*led_class_dev;

int major;

static int led_open(struct inode *inode, struct file *file)
{
    *gp25con &= ~(0x3FF);
    *gp25con |= (0 << 0);
    *gp25con |= (0 << 2);
    *gp25con |= (3 << 6);
    *gp25con |= (1 << 9);

	printk("led_open\n");
	return 0;
}

static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;

	//printk("led_write\n");

	copy_from_user(&val, buf, count); //	copy_to_user();

	if (val == 1)
	{
		// 点灯
        *gp25dat &= ~(0x3<< 0);//bit0-1 清零
        *gp25dat |= (0x2 << 0);//点灯
        printk("gpio_25 led1 on \n");

	}
	else
	{
		// 灭灯
        *gp25dat &= ~(0x3<< 0);//bit0-1 清零
        printk("gpio_25 led1 off\n");
	}

	return 0;
}

static struct file_operations led_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   led_open,
	.write	=	led_write,
};

static int led_init(void)
{
	major = register_chrdev(0, "myled", &led_fops); // 注册, 告诉内核

	led_class = class_create(THIS_MODULE, "myled");

	//led_class_dev = class_device_create(led_class, NULL, MKDEV(major, 0), NULL, "led1"); /* /dev/led1 */
	led_class_dev = device_create(led_class, NULL, MKDEV(major, 0), NULL, "led1"); /* /dev/led1 */

	gp25con = (volatile unsigned long *)ioremap((0x01000000+0x1000*25), 16);
	gp25dat = gp25con + 1;

	return 0;
}

static void led_exit(void)
{
	unregister_chrdev(major, "myled"); // 卸载

	//class_device_unregister(led_class_dev);
	device_unregister(led_class_dev);
	class_destroy(led_class);
	iounmap(gp25con);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");//不加的话加载会有错误提醒
MODULE_AUTHOR("puck.shen");//作者
MODULE_VERSION("v01");//版本
MODULE_DESCRIPTION("MyLED Driver Test on MDM9x07");//简单的描述

