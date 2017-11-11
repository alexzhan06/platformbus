#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
static struct resource led_resource[] = {/*声明发光二极管占用的端口资源 */
 [0] = {
		 .start = 0xE0200280,
		 .end = 0xE0200280 + 8 - 1,
		 .flags = IORESOURCE_MEM,
 	 },
 [1] = {
		 .start = 0,
		 .end = 0,
		 .flags = IORESOURCE_IRQ,
 	 }
};
int led_release(struct inode *inode, struct file *filp)
{
	 return 0;
}
static struct platform_device led_dev = {
		.name =  "SMARTLED", //设备名称, 与驱动程序中驱动名称保持一致
		.id =  -1,
		.num_resources = ARRAY_SIZE(led_resource),   //资源数量
		.resource = led_resource,  //设备资源
		.dev = {
				.release = led_release,
		},
};
static int led_dev_init(void)
{
	platform_device_register(&led_dev);  //注册设备
	return 0;
}
static void led_dev_exit(void)
{
  platform_device_unregister(&led_dev);  //注销设备
}
MODULE_LICENSE("GPL");
module_init(led_dev_init);
module_exit(led_dev_exit);


