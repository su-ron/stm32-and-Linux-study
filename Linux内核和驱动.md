# 从0开始学习Linux内核驱动

**参考文件：**

[呕心沥血整理的的嵌入式学习路线_一口Linux的博客-CSDN博客](https://blog.csdn.net/daocaokafei/article/details/108513929)

[从0学习Linux驱动 内核、目录 3_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV19v411h7g9/?spm_id_from=333.788&vd_source=e6a100138906f3892c6413488ca8e688)

一口Linux公众号回复ubuntu获取资料

## 0.C++文件操作的一些函数

**open**

```
int open(const char *pathname, int flags, mode_t mode);
```

参数说明：

- `pathname`：要打开或创建的文件路径名。

- ```
  flags
  ```

  ：打开文件的标志位，可以使用以下常量进行组合：

  - `O_RDONLY`：只读模式。
  - `O_WRONLY`：只写模式。
  - `O_RDWR`：读写模式。
  - `O_CREAT`：如果文件不存在则创建文件。
  - `O_APPEND`：追加模式，每次写入时都将数据追加到文件末尾。
  - `O_TRUNC`：截断文件，即清空文件内容。
  - 等等，还有其他标志位可供使用，可以根据需求进行组合。

- `mode`：当使用 `O_CREAT` 标志位创建文件时，指定文件的权限。可以使用如 `S_IRUSR | S_IWUSR` 的组合来设置读写权限，具体权限值可参考 `sys/stat.h` 头文件中的宏定义。

返回值是一个非负整数的文件描述符，表示打开的文件。如果出现错误，则返回-1，并设置相应的错误码。可以使用 `perror` 函数输出错误信息。



**read函数**

```
ssize_t read(int fd, void *buf, size_t count);
```

参数说明：

- `fd`：文件描述符，表示要读取的文件或套接字。
- `buf`：指向用于存储读取数据的缓冲区的指针。
- `count`：要读取的最大字节数。



**close函数**

```
int close(int fd);
```

参数说明：

- `fd`：要关闭的文件描述符。

返回值是整数类型，如果成功关闭文件，则返回0；否则返回-1，并设置相应的错误码。

## 1.模块化编程



<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230828211620174.png" alt="image-20230828211620174" style="zoom: 50%;" />

```c++
/*  
 *一口Linux
 *2021.6.21
 *version: 1.0.0
*/

#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PD");

static int hello_init(void)
{
	printk(KERN_SOH"hello_init \n");
	return 0;
}
static void hello_exit(void)
{
	printk("hello_exit \n");  //printk:内核的printf函数，用于输出信息
	return;
}

module_init(hello_init); //insmod  模块的入口：完成模块的加载
module_exit(hello_exit);//rmmod    模块的出口;完成模块的卸载

```



### 1.1**编译加载查看命令**

- make  编译
- lsmod 显示模块
- lsmod |grep hello  查看hello模块是否有被加载
- insmod/rmmod 安装/卸载模块
- dmesg  打印log信息
- dmesg -c 用于查看内核环缓冲区（kernel ring buffer）中的日志消息，并将缓冲区清除。该命令可以显示最近的内核日志消息，例如系统启动信息、硬件设备的插拔、驱动程序的加载和其他与内核相关的事件。



操作步骤：

![image-20230828213231233](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230828213231233.png)

**添加权限 chmod o+w 是重点**

![image-20230828213638009](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230828213638009.png)

此时编译完成，使用ls可以看到hello.ko文件已经生成

然后insmod hello.ko 安装hello模块，然后通过dmesg打印日志



## 2.makefile

makefile文件

```
ifneq ($(KERNELRELEASE),)
obj-m:=hello.o
else
KDIR :=/lib/modules/$(shell uname -r)/build
PWD  :=$(shell pwd)
all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -f *.ko *.o *.mod.o *.symvers *.cmd  *.mod.c *.order
endif
```



make -C $(KDIR) M=$(PWD) modules

![image-20230829110947339](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829110947339.png)

执行完这一步后，hello.c会生成目标文件hello.o ，内核的makefile顶层文件就会执行**链接操作**，然后生成hello.ko

![image-20230829111431843](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829111431843.png)

所以makefile是进入了三次

**论证过程**

```
ifneq ($(KERNELRELEASE),)
$(info "2nd")
obj-m:=hello.o
else
KDIR :=/lib/modules/$(shell uname -r)/build
PWD  :=$(shell pwd)
all:
    $(info "1st")
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -f *.ko *.o *.mod.o *.symvers *.cmd  *.mod.c *.order
endif
```

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829112909045.png" alt="image-20230829112909045" style="zoom:50%;" />



## 3.符号导出

### 3.1理论说明

本节内容代码在2_export文件夹中

**本节内容主要解决的问题是：Linux内核中模块B如何去访问模块A中的全局变量和函数**

![image-20230829113015516](C:/Users/su/AppData/Roaming/Typora/typora-user-images/image-20230829113015516.png)



![image-20230829113134280](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829113134280.png)



![image-20230829150117982](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829150117982.png)

模块B中extern关键字是说明该变量是**全局变量**

**步骤：**

![image-20230829150457749](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829150457749.png<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829150534627.png" alt="image-20230829150534627" style="zoom:50%;" />

**注意:**

1.加载的时候，必须加载a模块，再加载b模块

2.卸载：必须先卸载b模块，再卸载a模块



### 3.2 实验操作

步骤：

首先进入到a文件夹中make编译文件

![image-20230829152811856](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829152811856.png)

此时可以得到编译文件Module.symvers

如何把该文件复制到b文件夹中

```
cp Module.symvers ../b/
```

![image-20230829152855587](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829152855587.png)

然后清除日志信息

```
sudo dmesg -c
```

然后加载a模块

```
insmod helloa.ko
```

dmesg打印日志消息

再进入b文件夹make编译文件

然后加载b模块

```
insmod hellob.ko
```





## 4.模块传参

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829160453273.png" alt="image-20230829160453273" style="zoom:33%;" />

Linux内核支持的数据类型

![image-20230829160623171](C:/Users/su/AppData/Roaming/Typora/typora-user-images/image-20230829160623171.png)

实验代码

```c++
/*  
 *一口Linux
 *2021.6.21
 *version: 1.0.0
*/

#include <linux/init.h>
#include <linux/module.h>

static char *whom = "hello \n";
static int var = 1;

static int hello_init(void)
{
	printk("hello_init %s \n",whom);
	return 0;
}
static void hello_exit(void)
{
	printk("hello_exit  %d\n",var);
	return;
}
MODULE_LICENSE("GPL");
module_param(whom,charp,0644);
module_param_named(var_out,var,int,0644);

module_init(hello_init);
module_exit(hello_exit);

```

测试：

- **insmod hello.ko var=1  whom="yikoupeng"**



<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829161906766.png" alt="image-20230829161906766" style="zoom: 50%;" />



**sysfs:内核给一些重要的资源创建的目录或者文件，每个模块会在/sys/module下创建一个同名的文件夹**

可以发现实验中添加了hello模块后，/sys/module下创建一个同名的hello文件夹

![image-20230829162626552](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829162626552.png)

打开hello文件夹中的parameters文件夹

![image-20230829162059192](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829162059192.png)





## 5.字符设备号

参考资料：[从0学Linux驱动-字符设备号 8_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1AU4y1a7Hi/?spm_id_from=pageDriver&vd_source=e6a100138906f3892c6413488ca8e688)

### 5.1 理论说明

modinfo

一个在Linux系统中使用的命令，用于查看已加载内核模块的信息。它提供了关于指定内核模块的详细信息，包括模块的文件路径、版本号、作者、描述、参数和依赖项等。

![image-20230829163102096](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829163102096.png)





设备分类：

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829163533735.png" alt="image-20230829163533735" style="zoom: 67%;" />

设备号

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829163943353.png" alt="image-20230829163943353" style="zoom:50%;" />



构造设备号

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829164502078.png" alt="image-20230829164502078" style="zoom:50%;" />

**MKDEV就是让ma左移20位使其变成主设备号**

注册/注销设备号

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829165506126.png" alt="image-20230829165506126" style="zoom: 67%;" />







### 5.2 实验

测试代码：

```c+=
/*  
 *一口Linux
 *2021.6.21
 *version: 1.0.0
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>

static int major = 233;
static int minor = 0;

static dev_t devno;

static int hello_init(void)
{
	int result;
	
	printk("hello_init \n");
	devno = MKDEV(major,minor);
	
	result = register_chrdev_region(devno, 1, "test");
	if(result<0)
	{
		printk("register_chrdev_region fail \n");
		return result;
	}
	
	return 0;
}
static void hello_exit(void)
{
	printk("hello_exit \n");
	unregister_chrdev_region(devno,1);
	return;
}

module_init(hello_init);
module_exit(hello_exit);
//proc/devices
```

加载hello模块可以发现233设备号已经加载

![image-20230829170418814](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829170418814.png)

当rmmod hello.模块时，可以发现设备号233也已经卸载

![image-20230829170555576](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829170555576.png)



## 6.字符设备架构

Linux系统一切皆文件

`mknod /dev/hello c 237 0` 是在Linux系统中用于创建设备节点的命令。

该命令的作用是创建一个名为 `/dev/hello` 的字符设备节点。字符设备节点是一种特殊文件，用于与字符设备进行交互，比如串口设备、打印机等。

在这个命令中，`c` 表示创建一个字符设备节点，`237` 是设备的主设备号，`0` 是次设备号。主设备号和次设备号的值通常由设备驱动程序来确定，并且用于唯一标识每个设备。通过执行该命令，将在 `/dev` 目录下创建一个名为 `hello` 的字符设备节点，其主设备号为 237，次设备号为 0。

![image-20230829173622037](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829173622037.png)

有一个文件，就有一个inode，inode记录了文件的静态信息（不变的信息）

i_rdev 存放可变的信息

设备号存储在 i_rdev结构体



VFS存放的不变信息包括设备号和读写权限（VFS (Virtual File System) 是操作系统中的一个抽象层，用于统一管理不同文件系统和文件的访问。）

![image-20230829172626826](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230829172626826.png)

chrdevs是哈希表数据类型

中间237指的就是我们注册的字符设备号   





![image-20230830110135129](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230830110135129.png)

每一个char_device_struct结构体内部都会有一个cdev结构体

![image-20230830110649596](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230830110649596.png)



## 7.字符设备注册



<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230830095143407.png" alt="image-20230830095143407" style="zoom:33%;" />

本节代码

hello.c

```c++
/*  
 *一口Linux
 *2021.6.21
 *version: 1.0.0
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>

static int major = 237;
static int minor = 0;
static dev_t devno;
static struct cdev cdev;
static int hello_open (struct inode *inode, struct file *filep)
{
	printk("hello_open()\n");
	return 0;
}
static struct file_operations hello_ops = 
{
	.open = hello_open,
};
static int hello_init(void)
{
	int result;
	int error;	
	printk("hello_init \n");
	devno = MKDEV(major,minor);	
	result = register_chrdev_region(devno, 1, "test");
	if(result<0)
	{
		printk("register_chrdev_region fail \n");
		return result;
	}
	cdev_init(&cdev,&hello_ops);
	error = cdev_add(&cdev,devno,1);
	if(error < 0)
	{
		printk("cdev_add fail \n");
		unregister_chrdev_region(devno,1); 
		return error;
	}
	return 0;
}
static void hello_exit(void)
{
	printk("hello_exit \n");
	cdev_del(&cdev);
	unregister_chrdev_region(devno,1);
	return;
}
module_init(hello_init);
module_exit(hello_exit);
//proc/devices

```



123.c

```c++
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc,char**argv)
{
    int fd;
    
    fd=open("/dev/test",O_RDWR);

    if(fd<0)
    {
        perror("");
        return 0;
    }

}
```

注释：如果在Linux中查看open所需的头文件，那么使用命令

```
man 2 open
```



### 更简单的方式

注册函数

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230830144606860.png" alt="image-20230830144606860" style="zoom:50%;" />

注销函数

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230830144737602.png" alt="image-20230830144737602" style="zoom: 67%;" />





**字符注册的过程：**

![image-20230830151147767](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230830151147767.png)

实验代码

hello.c

```c++
/*  
 *一口Linux
 *2021.6.21
 *version: 1.0.0
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>

unsigned int major = 237;

int hello_open (struct inode *inode, struct file *file)
{
	printk("hello_open()\n");
	return 0;
}

int hello_release (struct inode *inode, struct file *file)
{
	printk("hello_release()\n");
	return 0;
}

struct file_operations fops ={
	.open = hello_open,
	.release = hello_release,
};

static int hello_init(void)
{
	int ret;
	
	printk("hello_init()\n");

    
	ret = register_chrdev(major, "peng", &fops);
	if(ret<0)
	{
		printk("register_chrdev fail\n");
		return ret;
	}
	return 0;	
}

static void hello_exit(void)
{
	printk("hello_exit()\n");
	unregister_chrdev(major, "peng");
	return ;
}

MODULE_LICENSE("GPL");
module_init(hello_init);
module_exit(hello_exit);

```

123.c

```c++
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc,char**argv)
{
    int fd;
    
    fd=open("/dev/test",O_RDWR);

    if(fd<0)
    {
        perror("");
        return 0;
    }
   
   sleep(10);
   close(fd);
}
```

**实验过程**

首先make编译，然后创建一个名为 `/dev/hello` 的字符设备节点

```
mknod /dev/test c 237 0
```

查看是否创建成功

```
ls /dev/test -l
```

然后查看设备号是否注册成功

```
cat /proc/devices
```

然后添加hello模块

```
insmod hello.ko
```

然后gcc编译123.c文件

```
gcc 123.c
./a.out
```

打开另一个终端

```
dmesg
```

查看日志





## 8.自动创建设备节点

理论说明：

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230830155501159.png" alt="image-20230830155501159" style="zoom:50%;" />

例如：我们可以查看鼠标的设备节点，可以看到它也是使用自动创建设备节点的方式的

![image-20230830160622242](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230830160622242.png)

自动创建设备节点需要依赖于两个函数

创建一个类

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230830160345252.png" alt="image-20230830160345252" style="zoom:50%;" />

注销类

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230830160954298.png" alt="image-20230830160954298" style="zoom:50%;" />

导出设备信息到用户空间

<img src="C:/Users/su/AppData/Roaming/Typora/typora-user-images/image-20230830161122401.png" alt="image-20230830161122401" style="zoom:67%;" />

注销设备信息

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230830161437081.png" alt="image-20230830161437081" style="zoom: 67%;" />





## 9.增加read、write接口

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230831093930326.png" alt="image-20230831093930326" style="zoom:50%;" />





<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230831094042173.png" alt="image-20230831094042173" style="zoom:50%;" />

read结构

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230831094402743.png" style="zoom:50%;" />

```c++
static inline unsigned long __must_check copy_to_user(void*to,const void __user *from,unsigned long n)
```

write结构

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230831094907612.png" alt="image-20230831094907612" style="zoom:50%;" />

```c++
static inline unsigned long __must_check copy_from_user(void*to,const void __user *from,unsigned long n)
```



关键代码

```c++
#define KMAX_LEN 32
char kbuf[KMAX_LEN+1] = "kernel";


//read(fd,buff,40);

static ssize_t hello_read (struct file *filep, char __user *buf, size_t size, loff_t *pos)
{
	int error;

	
	if(size > strlen(kbuf))
	{
		size = strlen(kbuf);
	}

	if(copy_to_user(buf,kbuf, size))
	{
		error = -EFAULT;
		return error;
	}

	return size;
}
//write(fd,buff,40);
static ssize_t hello_write (struct file *filep, const char __user *buf, size_t size, loff_t *pos)
{
	int error;

	if(size > KMAX_LEN)
	{
		size = KMAX_LEN;
	}
	memset(kbuf,0,sizeof(kbuf));
	if(copy_from_user(kbuf, buf, size))
	{
		error = -EFAULT;
		return error;
	}
	printk("%s\n",kbuf);
	return size;
}


static struct file_operations hello_ops = 
{
	.open = hello_open,
	.release = hello_release,
	.read = hello_read,
	.write = hello_write,
};
```



test.c

```c++
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
void main()
{
	int fd;
	char buf[64]={0};
	char buf2[64]="suronjian";
	int len;
	
	fd = open("/dev/test",O_RDWR);
	if(fd<0)
	{
		perror("open fail \n");
		return;
	}

	len=read(fd,buf,64);
    buf[len]='\0';
	printf("read:%s\n len=%d\n",buf,len);

	strcpy(buf,"su");
	len=write(fd,buf2,strlen(buf2));
	printf("len=%d\n",len);
	
	printf("open ok \n ");
}
```



## 10.字符设备ioctl接口

为什么要引入ioctrl？

<img src="C:/Users/su/AppData/Roaming/Typora/typora-user-images/image-20230831201516252.png" alt="image-20230831201516252" style="zoom:50%;" />

应用层系统调用

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230831201542829.png" alt="image-20230831201542829" style="zoom:50%;" />

在Linux内核的函数

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230831201642209.png" alt="image-20230831201642209" style="zoom:50%;" />

调用关系

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230831201803249.png" alt="image-20230831201803249" style="zoom:50%;" />



<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230831201909817.png" alt="image-20230831201909817" style="zoom:50%;" />

封装命令

<img src="C:/Users/su/AppData/Roaming/Typora/typora-user-images/image-20230831202053037.png" alt="image-20230831202053037" style="zoom:50%;" />

举例：

![image-20230831202429329](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230831202429329.png)





如何检查命令、地址的正确性？
<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230831202545718.png" alt="image-20230831202545718" style="zoom:50%;" />

实验代码：

hello.c

```c++
 static int knum=99;//内核设置knum=
/*
  ioctl(fd,DEV_FIFO_GETVALUE,&num);
*/
   long hello_ioctl(struct file* filep,unsigned int cmd,unsigned long arg)
   {
        void __user*argp=(void __user*)arg;
		int __user*p =argp;
    
		long err,ret;
      //检测有效性
			if(_IOC_TYPE(cmd)!=DEV_FIFO_TYPE){
		pr_err("cmd   %u,bad magic 0x%x/0x%x.\n",cmd,_IOC_TYPE(cmd),DEV_FIFO_TYPE);
		return-ENOTTY;
	}
	if(_IOC_DIR(cmd)&_IOC_READ)
		ret=!access_ok(VERIFY_WRITE,(void __user*)arg,_IOC_SIZE(cmd));
	else if( _IOC_DIR(cmd)&_IOC_WRITE )
		ret=!access_ok(VERIFY_READ,(void   __user*)arg,_IOC_SIZE(cmd));

	if(ret){
		pr_err("bad   access %ld.\n",ret);
		return-EFAULT;
	}

        switch(cmd)
        {
         case DEV_FIFO_CLEAN:
           printk("DEV_FIFO_CLEAN\n");
		   break;
		 case DEV_FIFO_GETVALUE:
		 	err=put_user(knum,p);
		   printk("DEV_FIFO_GETVALUE %d\n",knum);
		   break;
		 case DEV_FIFO_SETVALUE:
		   err=get_user(knum,p);
		   printk("DEV_FIFO_SETVALUE %d\n",knum);
		   break;
		 default:
		 	return -EINVAL;
        }
		return err;
   }
```

test.c

```c++
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include"beep.h"
void main()
{
	int fd;
	int len;
	int num;
	fd = open("/dev/test",O_RDWR);
	if(fd<0)
	{
		perror("open fail \n");
		return;
	}

    ioctl(fd,DEV_FIFO_CLEAN);

	ioctl(fd,DEV_FIFO_GETVALUE,&num);
	printf("num=%d\n",num);

	num=77;
	ioctl(fd,DEV_FIFO_SETVALUE,&num);
	printf("num=%d\n",num);
	
	printf("open ok \n ");
}
```

实验结果：

![image-20230901093842994](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230901093842994.png)



## 11.进程、文件描述符、file、inode、设备号关系

inode存放静态信息，file存放动态信息

**用户空间和内核空间地址问题**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230901094155375.png" alt="image-20230901094155375" style="zoom:50%;" />



**文件、inode**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230901094832453.png" alt="image-20230901094832453" style="zoom: 67%;" />

**VFS**（Virtual File System，虚拟文件系统）是操作系统中的一个抽象层，用于统一管理不同文件系统和文件的访问

**mknod /dev/com0 c 237 0**该命令的作用是创建一个名为 `/dev/com0` 的字符设备节点。字符设备节点是一种特殊文件，用于与字符设备进行交互



**进程与文件描述符**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230901095353561.png" alt="image-20230901095353561" style="zoom:67%;" />

查看文件被哪一些进程打开

```
lsof test   
```

![image-20230901101159104](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230901101159104.png)

可以看到test文件被run进程打开

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230901101714273.png" alt="image-20230901101714273" style="zoom:50%;" />



<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230901101814976.png" alt="image-20230901101814976" style="zoom: 67%;" />

struct files_struct中的成员struct file __rcu*fd_array[NR_OPEN_DEFAL]维护文件描述符fd



打开文件

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230901102621018.png" alt="image-20230901102621018" style="zoom:67%;" />

文件描述符fd存放对文件的操作，文件指向的inode存放设备号



## 12.如何支持多个次设备号？

参考程序：10-1

**此处问题在于open(inode,file),有inode信息，能够区分次设备号，而read(file)函数只存入file信息，如何区分次设备号？**

文件描述符fd0,fd1打开的文件是不一样的，但是它们使用的open函数是一样的，所以在open函数中提取出次设备号

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230901110252324.png" alt="image-20230901110252324" style="zoom:67%;" />



实现

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230901105319969.png" alt="image-20230901105319969" style="zoom:50%;" />



实验代码：

(关键部分)

```c++
#define MAX_COM_NUM 2

struct mydev{
	char *reg;
	int test;
};
struct mydev *pmydev[MAX_COM_NUM];
struct cdev cdev;


ssize_t dev_fifo_read (struct file *file, char __user *buf, size_t size, loff_t *pos)
{
	struct mydev *cd;

	cd = (struct mydev *)file->private_data;
	printk("read()       file->private_data         cd->test=%d\n",cd->test);

	if(copy_to_user(buf, &(cd->test), size)){
		return -EFAULT;
	}

	return size;
}

int dev_fifo_close (struct inode *inode, struct file *file)
{
	printk("dev_fifo_close()\n");
	return 0;
}
static int dev_fifo_open (struct inode *inode, struct file *file)
{
	struct mydev *cd;

	printk("minor = %d \n",MINOR(inode->i_rdev));
	cd = pmydev[MINOR(inode->i_rdev)];
	
	file->private_data = cd;
	return 0;
}
```

设备打开函数 `dev_fifo_open` 中，通过获取当前打开设备的次设备号来找到对应的 `mydev` 结构体，并将其存储在打开文件的私有数据区域 `file->private_data` 中，以便在后续操作中使用。

在设备读取函数 `dev_fifo_read` 中，则从打开文件的私有数据区域中获取 `mydev` 结构体指针，并将其中的测试整数复制到用户空间缓冲区中。



## 13.Linux中的并发机制

**并发：多个执行单元同时进行或多个执行单元串行执行，宏观并行执行**

**竞态：并发的执行单元对共享资源（硬件资源和软件上的全局变量）的访问而导致的竞态状态**

**临界资源：多个进程访问的资源**

**临界区：多个进程访问的代码段**

**并发场合**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230902183616102.png" alt="image-20230902183616102" style="zoom:50%;" />

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230902183708796.png" alt="image-20230902183708796" style="zoom:50%;" />

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230902183841559.png" alt="image-20230902183841559" style="zoom:50%;" />

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230902184041603.png" alt="image-20230902184041603" style="zoom:50%;" />



**Linux并发解决方案**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230902184234960.png" alt="image-20230902184234960" style="zoom:50%;" />



## 14.一个有问题的并发控制

出现的问题：

进程的切换有可能导致两个任务都成功打开同一个设备，这是有问题的

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230902192935327.png" alt="image-20230902192935327" style="zoom:50%;" />





## 15.Linux驱动-原子操作

参考代码：12atomic

**概念：**

- **原子操作是指不被打断的操作，即它是最小的执行单元。**
- **最简单的原子操作就是一条条的汇编指令（不包括一些伪指令，伪指令会被汇编器解释成多条汇编指令）**



定义：在linux中原子操作对应的数据结构为automic_t，定义如下：

```
typedef struct{
      int counter;
}atomic_t
```

常用操作：

<img src="C:/Users/su/AppData/Roaming/Typora/typora-user-images/image-20230902193237731.png" alt="image-20230902193237731" style="zoom:50%;" />

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230902193356852.png" alt="image-20230902193356852" style="zoom: 50%;" />



举例：

使用原子操作后，进程切换不会导致多个进程同时使用同一个设备的关键在于它是先**减一然后判断是否为0这两步操作同时进行。**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230902193534924.png" alt="image-20230902193534924" style="zoom:50%;" />

关键代码：

```c++
static atomic_t v =ATOMIC_INIT(1);

static int hello_open (struct inode *inode, struct file *filep)
{
	if(!atomic_dec_and_test(&v)) 
	{
		//busy
		atomic_inc(&v);
		return -EBUSY;
	}
	return 0;

	#if 0
	if(flage == 0)
	{
		return -EBUSY;
	}
	printk("hello_open()\n");
	flage = 0;
	#endif
	
}

static int hello_release (struct inode *inode, struct file *filep)
{
	atomic_inc(&v);
	printk("hello_release()\n");
	return 0;
}
```



## 16.内核互斥锁

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230902204401509.png" alt="image-20230902204401509" style="zoom: 33%;" />

**mutex的使用注意事项**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230902204608525.png" alt="image-20230902204608525" style="zoom:33%;" />

**初始化**

静态定义：

```c++
DEFINE_MUTEX(name)
```

动态初始化mutex

```c++
mutex_init(&mutex)
```

**互斥锁的操作**

为指定的mutex上锁，如果不可用则睡眠

```c++
mutex_lock(struct mutex*)
```



为指定的mutex解锁

```c++
mutex_unlock(struct mutex*)
```



尝试获取指定的mutex,如果成功则返回1，否则锁被获取，返回值是0

```
mutex_trylock(struct mutex*)
```



如果锁已被征用，则返回1；否则返回0

```c++
mutex_is_lock(struct mutex*)
```



**使用实例**

就是但我们不想临界区的代码被打断时，就是加一个互斥锁来进行保护

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230902205808091.png" alt="image-20230902205808091" style="zoom:50%;" />

关键代码：

```c++
struct mutex mutex;
int flage=1;//1 available   0:busy


static int hello_open (struct inode *inode, struct file *filep)
{
	printk("hello_open()\n");
	mutex_lock(&mutex);
    if(flage!=1)
    	{
    	mutex_unlock(&mutex);
        return -EBUSY;
    	}

	//占用设备
	flage=0;
	mutex_unlock(&mutex);
	return 0;
}

static int hello_release (struct inode *inode, struct file *filep)
{
	printk("hello_release()\n");
	//释放设备
    flage=1;
	return 0;
}
```

test.c

```c++
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
main()
{
	int fd;
	int len;
	char buf[64]={0};
	char buf2[64+1]="peng";
	
	
	fd = open("/dev/hellodev",O_RDWR);
	if(fd<0)
	{
		perror("open fail \n");
		return;
	}
    printf("before sleep\n");
    sleep(15);
	printf("after sleep\n");
	 
	close(fd);
}
```



**实验结果**

![image-20230904092818809](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904092818809.png)

可以看到第二个进程无法成功打开同一个设备



## 17.信号量

**概念：** 信号量又被称为信号灯

它被用来协调不同进程间的数据对象，而最主要的应用是共享内存方式的进程间通信。本质上，信号量是一个计数器，它用来记录对某个资源（如共享内存）的存取情况。



**Linux下的几种信号量**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904093709127.png" alt="image-20230904093709127" style="zoom:50%;" />

POSIX信号量主要使用于线程中，SYTEM V信号量主要使用于进程间



核心操作

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904094019500.png" alt="image-20230904094019500" style="zoom:50%;" />

当任务队列为满时，信号量值为0

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904094439134.png" alt="image-20230904094439134" style="zoom:50%;" />

当任务队列中为空时，那么信号量值就为三了

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904094500244.png" alt="image-20230904094500244" style="zoom:50%;" />



**初始化**

```c++
#include <linux/semaphore.h>

struct semaphore sem;
sema_init(&sem,1);
```



**P、V操作**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904100433155.png" alt="image-20230904100433155" style="zoom:50%;" />

```
extern int __must_check down(struct semaphore *sem)
```

阻塞状态不可以被打断

```c++
extern int __must_check down_interruptible(struct semaphore *sem)
```

阻塞状态可以被打断

`down_interruptible(&sem)` 是Linux内核中的一个函数调用，用于尝试获取一个信号量（Semaphore）。该函数会使当前进程在等待获取信号量时被挂起（阻塞），直到成功获取信号量或者被接收到一个中断信号（如 `Ctrl+C` 或 `kill -INT`）。

具体而言，`down_interruptible()` 函数的作用是：

1. 如果信号量 `sem` 的值大于零，则将其递减1，并继续执行。
2. 如果信号量 `sem` 的值为零，则当前进程被置于睡眠状态，直到有其他进程通过 `up()` 函数增加了信号量的值。如果在睡眠期间接收到中断信号，`down_interruptible()` 函数会返回 `-ERESTARTSYS` 错误码，表示进程因为中断而被唤醒。
3. 如果成功获取了信号量，函数返回0。



```c++
 extern int __must_check down_killable(struct semaphore *sem)
```

`down_killable(struct semaphore *sem)` 是Linux内核中的一个函数调用，用于尝试获取一个信号量（Semaphore）。与 `down_interruptible()` 函数类似，该函数也会使当前进程在等待获取信号量时被挂起（阻塞），直到成功获取信号量或者收到一个可终止的信号。

具体而言，`down_killable()` 函数的行为如下：

1. 如果信号量 `sem` 的值大于零，则将其递减1，并继续执行。
2. 如果信号量 `sem` 的值为零，则当前进程被置于睡眠状态，直到有其他进程通过 `up()` 函数增加了信号量的值。如果在睡眠期间接收到可终止信号（如 `SIGTERM` 或 `SIGINT`），`down_killable()` 函数会返回 `-EINTR` 错误码，表示进程因为接收到可终止信号而被中断。
3. 如果成功获取了信号量，函数返回0。

**与 `down_interruptible()` 不同，`down_killable()` 在等待信号量时只会对可终止的信号做出响应，而不是对所有中断信号都做出响应**。这意味着一些无法终止的信号（如 `SIGKILL`）不会导致 `down_killable()` 返回。这样可以在需要在保证资源的完整性的同时，允许某些特定的信号可以中断正在等待资源的进程。

需要注意的是，使用 `down_killable()` 函数时也需要小心处理多线程或多进程情况下的并发和竞态条件，以及在获取信号量时是否要考虑可终止信号的影响。



```c++
 extern int __must_check down_trylock(struct semaphore *sem)
```

`trylock(struct semaphore *sem)` 是Linux内核中的一个函数调用，用于尝试获取一个信号量（Semaphore）。与 `down_interruptible()` 和 `down_killable()` 函数不同，**该函数是非阻塞调用，只有在当前时刻信号量可用时才会成功获取**，否则将立即返回。

具体而言，`trylock()` 函数的行为如下：

1. 如果信号量 `sem` 的值大于零，则将其递减1，并返回成功获取信号量的结果（0）。
2. 如果信号量 `sem` 的值为零，则立即返回失败的结果（非0）。

需要注意的是，由于 `trylock()` 函数是一个非阻塞调用，因此可以在需要快速尝试获取信号量的场景中使用。例如，可以在执行某些关键操作之前先尝试获取信号量，以确保当前资源没有被其他进程占用。如果成功获取了信号量，则可以执行关键操作，否则等待下一次机会。

需要注意的是，使用 `trylock()` 函数时也需要小心处理多线程或多进程情况下的并发和竞态条件，以确保对共享资源的访问是线程安全的。



```c++
extern int __must_check down_timeout(struct semaphore *sem,long jiffies)
```

`down_timeout(struct semaphore *sem, long jiffies)` 是 Linux 内核中的一个函数调用，用于以超时方式尝试获取一个信号量（Semaphore）。该函数会尝试获取信号量，如果在指定的超时时间内未成功获取，则会返回一个错误码。

具体而言，`down_timeout()` 函数的行为如下：

1. 如果信号量 `sem` 的值大于零，则将其递减 1，并返回成功获取信号量的结果（0）。
2. 如果信号量 `sem` 的值为零，则当前进程被置于睡眠状态，等待其他进程通过 `up()` 函数增加了信号量的值。同时，`down_timeout()` 函数会将进程设置为超时睡眠状态并等待指定的时间（以 jiffies 为单位）。
3. 如果在超时时间内成功获取了信号量，则函数返回 0。
4. 如果超时时间到期仍未成功获取信号量，则函数返回 `-ETIME` 错误码，表示等待超时。

需要注意的是，jiffies 是 Linux 内核中用于计算时间的单位，表示从系统启动开始算起的节拍数。可以使用 `jiffies_to_msecs()` 等函数将 jiffies 转换为毫秒。

`down_timeout()` 函数常用于需要在一段时间内等待资源可用的场景，以避免永久阻塞并允许进行超时处理。通过指定合适的超时时间，可以控制等待信号量的时间，并在超时后执行其他逻辑。



举例：

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904100659543.png" alt="image-20230904100659543" style="zoom:50%;" />

实验代码：

实验代码基于8rw代码进行修改

hello.c的部分代码

```c++
static int hello_open (struct inode *inode, struct file *filep)
{
		
	printk("hello_open()\n");
	   if(down_interruptible(&sem))
	   	return -ERESTARTSYS;
	return 0;
}
static int hello_release (struct inode *inode, struct file *filep)
{
	printk("hello_release()\n");

	up(&sem);
	return 0;
}

static int hello_init(void)
{
   //省略代码
	sema_init(&sem,1); //初始化信号量为1
	
}
```



test.c

```c++
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
main()
{
	int fd;
	int len;
	char buf[64]={0};
	char buf2[64+1]="peng";
	
	
	fd = open("/dev/hellodev",O_RDWR);
	if(fd<0)
	{
		perror("open fail \n");
		return;
	}
    printf("before sleep\n");
    sleep(15);
	printf("after sleep\n");
	close(fd);
}
```

实验结果：

<img src="C:/Users/su/AppData/Roaming/Typora/typora-user-images/image-20230904103509444.png" alt="image-20230904103509444" style="zoom:50%;" />

可以看到第二个进程无法成功同时打开同一个设备

同时可以被打断



## 18.自旋锁spinlock

概念：

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904105425340.png" alt="image-20230904105425340" style="zoom:50%;" />

**自旋锁的优点**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904105617000.png" alt="image-20230904105617000" style="zoom:50%;" />

注意事项：

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904105838249.png" alt="image-20230904105838249" style="zoom:50%;" />

**定义**

动态的：

```c++
spinlock_t lock;
spin_lock_init(&lock);
```

静态的：

```c++
DEFINE_SPINLOCK(lock)
```



**锁申请/释放**

加锁

```c++
spin_lock(&lock);
```

解锁

```c++
spin_unlock(&lock)
```



使用实例：

<img src="C:/Users/su/AppData/Roaming/Typora/typora-user-images/image-20230904110345285.png" alt="image-20230904110345285" style="zoom:50%;" />



## 19.自旋锁-死锁

**死锁的情况**

<img src="C:/Users/su/AppData/Roaming/Typora/typora-user-images/image-20230904112428901.png" alt="image-20230904112428901" style="zoom:50%;" />

死锁举例：

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904112708840.png" alt="image-20230904112708840" style="zoom:50%;" />

**如何避免死锁**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904113630097.png" alt="image-20230904113630097" style="zoom:50%;" />



## 20.同步机制的总结

自旋锁和互斥锁使用场合

<img src="C:/Users/su/AppData/Roaming/Typora/typora-user-images/image-20230904143154178.png" alt="image-20230904143154178" style="zoom:50%;" />

例子：

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904143959910.png" alt="image-20230904143959910" style="zoom:50%;" />



<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904144028030.png" alt="image-20230904144028030" style="zoom:50%;" />



**信号量与互斥体**

优先使用互斥体

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904144225176.png" alt="image-20230904144225176" style="zoom:50%;" />



## 21.IO模型

参考资料



<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904144536618.png" alt="image-20230904144536618" style="zoom:50%;" />



### **IO的重要性**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904144705575.png" alt="image-20230904144705575" style="zoom:50%;" />



### Linux IO栈

![image-20230904145301235](https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904145301235.png)



### **五种IO模型**

- **阻塞式I/O**
- **非阻塞式I/O**
- **I/O服用（select,poll,epoll等）**
- **信号驱动式I/O**
- **异步I/O（POSIX的aio_系列函数）**



**阻塞IO**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904150349019.png" alt="image-20230904150349019" style="zoom:50%;" />



**非阻塞IO**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904150502212.png" alt="image-20230904150502212" style="zoom:50%;" />

**IO多路复用**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904150715272.png" alt="image-20230904150715272" style="zoom:50%;" />

**信号驱动IO**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904151148791.png" alt="image-20230904151148791" style="zoom:50%;" />

**异步IO**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904151420611.png" alt="image-20230904151420611" style="zoom:50%;" />



**异步IO和信号驱动IO的不同？**

<img src="https://cdn.jsdelivr.net/gh/su-ron/myBlogResource/freertos/picture/image-20230904151535149.png" alt="image-20230904151535149" style="zoom:50%;" />



## 22.等待队列



# 前期准备工作

### 1.数据结构

### 1.1 能掌握常见的算法

比如：冒泡排序、直接插入排序、快速排序、二叉树等。

冒泡排序：



直接插入排序：



快速排序：



二叉树：

二叉树的定义

```c++
struct TreeNode{
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int val):val(val),left(nullptr),right(nullptr){}
};
```



### 1.2 链表操作，创建、增删改查

单链表的定义

```c++
struct ListNode{
  int val;//节点存储的元素
  ListNode *next; //指向下一个节点的指针
  ListNode(int x):val(x),next(nullptr){} //节点的构造函数
};
```



链表的常见操作

```c++
class MyLinkedList{
    public:
    //定义链表节点结构体
   struct ListNode{
  int val;//节点存储的元素
  ListNode *next; //指向下一个节点的指针
  ListNode(int x):val(x),next(nullptr){} //节点的构造函数
};
   
    //初始化链表
    MyLinkedList(){
       //这里定义的头结点是一个虚拟头结点，而不是真正的头结点
        _dummyHead=new LinkedNode(0);
        _size =0;       
    }
    
    //获取第index个节点的数值，如果index是非法数值就返回-1
    //注意index是从0开始的，第0个节点就是头结点
    int get(int index){
        if(index>(_size-1)||index<0){
            return -1;
        }
       LinkedNode*cur =_dummyHead->next;
       while(index--){
           cur=cur->next;
       }
        return cur->val;
    }
    
   //在链表最前面插入一个节点，插入完成后，新插入的节点为链表的新的头节点
    void addAtHead(int val)
    {
        LinkedNode* newNode =new LinkedNode(val);
        newNode->next =_dummyHead->next;
        _dummyHead->next =newNode;
        _size++;
    }
    
    //在链表最后面添加一个节点
    void addAtTail(int val)
    {
        LinkedNode *newNode =new LinkNode(val);
        LinkedNode*cur =_dummyHead;
        while(cur->next !=nullptr){
            cur=cur->next;
        }
        cur->next=newNode;
        _size++;
    }
    
    //在第index个节点之前插入一个新节点
    //如果index为0，那么新插入的节点为链表新的头节点
    //如果index等于链表的长度，则说明新插入的节点为链表的尾节点
    //如果index大于链表的长度，则返回空
    void addAtIndex(int index,int val){
        if(index>_size){
            return;
        }
        LinkedNode*newNode =new LinkNode(val);
        LinkedNode*cur =_dummyHead;
        while(index--){
            cur=cur->next;
        }
        newNode->next =cur->next;
        cur->next =newNode;
        _size++;
    }
    
    //删除第index个节点，如果index大于或者等于链表的长度，则直接返回
    //注意index是从0开始的
    void deleteAtIndex(int index)
    {
        if(index >=_size)||index<0){
            return;
        }
       LinkedNode*cur =_dummyHead;
       while(index--)
       {
          cur=cur->next;
       }
       LinkedNode*tmp=cur->next;
        cur->next=cur->next->next;
        delete tmp;
        _size--;
    }
    
    //打印链表
    void printLinkedList(){
        LinkedNode*cur =_dummyHead;
        while(cur->next!=nullptr){
            cout<<cur->next->val<<" ";
            cur=cur->next;
        }
        cout<<endl;
    }
    private:
    int _size;
    LinkedNode* _dummyHead;
    
};
```



## 2.Linux的基本操作

参考资料：[Linux入门的基础知识点，有这篇就够了（持续更新）_8.来接v送 h+_一口Linux的博客-CSDN博客](https://blog.csdn.net/daocaokafei/article/details/117451388?spm=1001.2014.3001.5502)