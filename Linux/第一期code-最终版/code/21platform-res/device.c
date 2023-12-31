/*
*公众号 ：一口Linux
*2021.6.26
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>


void	hello_release(struct device *dev)
{
	printk("hello_release\n");
	return;
}


struct resource	res[]={
	[0] ={
		.start = 0x139d0000,
		.end  = 0x139d0000 + 0x3,
		.flags = IORESOURCE_MEM,
	},

	[1] ={
		.start = 199,
		.end  = 199,
		.flags = IORESOURCE_IRQ,
	},	
};


struct platform_device hello_device ={
	.name = "yikoulinux",
	.id = -1,
	.dev.release = hello_release,
	//hardware TBD
	.num_resources = ARRAY_SIZE(res),
	.resource = res,
};

static int hello_init(void)
{
	printk("hello_init\n");
	return platform_device_register(&hello_device);
}

static void hello_exit(void)
{
	printk("hello_exit\n");
	platform_device_unregister(&hello_device);
	return;
}
MODULE_LICENSE("GPL");
module_init(hello_init);
module_exit(hello_exit);
