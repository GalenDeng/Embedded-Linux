#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/uaccess.h>
static struct class  *firstdrv_class;		// 类
static struct class_device  *firstdrv_class_dev;	// 类里面再建一个设备

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;

static int first_drv_open(struct inode *inode, struct file *file)
{
	//printk(" first_drv_open\n");
/******配置GPF4 ,GPF5,GPF7为输出******/
*gpfcon &=  ~((0x3 << (4 * 2)) | (0x3 << (5 * 2)) | (0x3 << (6 * 2)));	// 良好习惯，先清0
*gpfcon |=  ((0x1 << (4 * 2)) | (0x1 << (5 * 2))  | (0x1 << (6 * 2)));		// 置1 ，设置为输出的模式
	return 0;
}

static int first_drv_write(struct file *file, const __user * buf ,size_t count , loff_t * ppos) // buf : 这个是应用层传进来的buf
{
	int val;
	copy_from_user(&val,  buf, count);	// 这种方式从应用层取值, 拷贝进val的地址里面
     //  copy_to_user(void __user * to, const void * from, unsigned long n)   从内核空间复制内容到用户空间

	if (val == 1)
	{
		//点灯
		*gpfdat &= ~((1 << 4) | (1 << 5) |(1 << 6) );
	}
	else
	{
		//灭灯
		*gpfdat |= ((1 << 4) | (1 << 5) |(1 << 6) );
	}
	//printk("  first_drv_write\n");
	return 0;
}

static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   first_drv_open,     
    .write  =   first_drv_write,
};


int major;
// 注册： major ：主设备号  first_dev ：名字(可以随便起 )    file_operations : 结构 ，把这个
// 结构告诉内核
static  int first_drv_init(void)
{
major = register_chrdev(0, "first_drv",  &first_drv_fops);

firstdrv_class = class_create(THIS_MODULE,"firstdrv"); // 创建一个类实例
if (IS_ERR(firstdrv_class))
	return PTR_ERR(firstdrv_class);

// 类下创建一个设备
firstdrv_class_dev = class_device_create(firstdrv_class, NULL, MKDEV(major,0), NULL, "xyz") ;
// firstdrv 这个类下会创建xyz 这个设备/*/dev/xyz 这个设备节点
if (unlikely(IS_ERR(firstdrv_class_dev))) 
	return PTR_ERR(firstdrv_class_dev);

gpfcon = (volatile unsigned long *)ioremap(0x56000050 , 16); // 0x56000060 : 开始地址16 ：16个字节大小
gpfdat = gpfcon + 1;	// 地址 + 1

return 0;
}

// 卸载驱动
 static  void first_drv_exit(void)
{
unregister_chrdev(major, "first_drv");

class_device_unregister(firstdrv_class_dev);	//删除设备
class_destroy(firstdrv_class);				// 删除类
iounmap(gpfcon);						// 删掉映射
}



// module_init 定义了一个结构体，这个结构体里面有一个函数指针，
// 指向 first_drv_init 这个驱动入口函数
module_init(first_drv_init);

module_exit(first_drv_exit);

MODULE_LICENSE("GPL");

