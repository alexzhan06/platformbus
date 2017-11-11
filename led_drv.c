#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#define LED_MAJOR		0
static int LED_major = LED_MAJOR;
struct resource *IO_mem_resource;
static struct class *led_class;
unsigned long io_addr;
struct LED_dev
{
	struct cdev cdev;
};
struct LED_dev  *LED_devp;
 int LED_open(struct inode *inode, struct file *filp)
{
 	filp->private_data = LED_devp;
    printk("In the open process! turn off the LED!\n");
    iowrite32(0x1111 | ioread32(io_addr), io_addr);     //设置GPJ2CON寄存器,功能为输出
    iowrite32(0xFFAA & ioread32(io_addr + 8), (io_addr+8)); //设置GPJ2UP寄存器,使能内部上拉
    iowrite32(0xF0 & ioread32(io_addr+4), (io_addr+4)); //设置GPJ2DAT寄存器,LED对应引脚为高电平,关闭发光二极管
    return 0;
}

int LED_release(struct inode *inode, struct file *filp)
{
	 return 0;
}
static ssize_t LED_read(struct file *filp, char __user  *buf, size_t size, loff_t *ppos)
{
	 int ret = 0;
	struct LED_dev *dev = filp->private_data;
	return ret;
 }
static ssize_t LED_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	   unsigned int count = size;
	   int ret = 0;
	   struct LED_dev *dev = filp->private_data;
	   unsigned char userbuf;
	   if (copy_from_user(&userbuf, buf, sizeof(userbuf)))
		   ret = -EFAULT;
	   else
	   {
		   iowrite32(userbuf, (io_addr+4));
		   printk("write data from user to LED!\n");
	   }
	   return ret;
 }
static const struct file_operations LED_fops=
{
		.owner = THIS_MODULE,
		.read = LED_read,
		.write = LED_write,
		.open = LED_open,
		.release = LED_release,
};
static void LED_setup_cdev(struct LED_dev *dev, int index)
{
	 int err, devno=MKDEV(LED_major, index);
	 cdev_init(&dev->cdev, &LED_fops);
	 dev->cdev.owner = THIS_MODULE;
	 dev->cdev.ops = &LED_fops;
	 err = cdev_add(&dev->cdev, devno, 1);
	 if (err)
		 printk(KERN_NOTICE  "Error %d adding LED%d", err, index);
}
static int led_probe(struct platform_device *pdev)
{
	int result;
	struct resource *res;
	dev_t devno = MKDEV(LED_major, 0);
	if (LED_major)
		result = register_chrdev_region(devno, 1, "LED");
	else
	{
		result = alloc_chrdev_region(&devno, 0, 1, "LED");
		LED_major = MAJOR(devno);
	}
	if (result < 0)
		return result;
	LED_devp = kmalloc(sizeof(struct LED_dev), GFP_KERNEL);
	if (!LED_devp)
	{
		result = -ENOMEM;
		goto fail;
	}
	memset(LED_devp, 0, sizeof(struct LED_dev));
	LED_setup_cdev(LED_devp, 0);
	led_class = class_create(THIS_MODULE, "LED");
	device_create(led_class, NULL, MKDEV(LED_major,0),NULL, "LED");
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0); //获取设备资源
	if ((IO_mem_resource=request_mem_region(res->start, res->end - res->start + 1, "LED"))==NULL)
		goto fail;
	else
	{
		printk("In the init process!\n");
		io_addr = (unsigned long)ioremap(res->start, res->end - res->start+1);
		printk("io_addr: %lx \n", io_addr);
		return 0;
	}
fail: unregister_chrdev_region(devno,1);
	return result;
}
static int led_remove(struct platform_device *pdev)
{
	struct resource *res;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (IO_mem_resource!=NULL)
		release_mem_region(res->start, res->end - res->start + 1);
	cdev_del(&LED_devp->cdev);
	kfree(LED_devp);
	device_destroy(led_class, MKDEV(LED_major,0));
	class_destroy(led_class);
	unregister_chrdev_region(MKDEV(LED_major,0),1);
}
struct platform_driver led_drv = {  //驱动结构体
		.probe = led_probe,
		.remove = led_remove,
		.driver = {
				.name = "SMARTLED",
		}
};
int LED_init(void)
{
	platform_driver_register(&led_drv);
}
void LED_exit(void)
{
	platform_driver_unregister(&led_drv);
}
MODULE_AUTHOR("AK-47");
MODULE_LICENSE("Dual BSD/GPL");
module_param(LED_major, int, S_IRUGO);
module_init(LED_init);
module_exit(LED_exit);
