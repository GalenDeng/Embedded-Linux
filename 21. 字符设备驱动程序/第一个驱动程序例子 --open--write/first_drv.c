#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>


static int first_drv_open(struct inode *inode, struct file *file)
{
	printk(" first_drv_open\n");
	return 0;
}

static int first_drv_write(struct file *file, const __user * buf ,size_t count , loff_t * ppos)
{
	printk("  first_drv_write\n");
	return 0;
}

static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   first_drv_open,     
    .write  =   first_drv_write,
};

// 注册： major ：主设备号  first_dev ：名字(可以随便起 )    file_operations : 结构 ，把这个
// 结构告诉内核
int major;
  int first_drv_init(void)
{
major = register_chrdev(0, "first_drv",  &first_drv_fops);
return 0;
}

// 卸载驱动
  void first_drv_exit(void)
{
unregister_chrdev(major, "first_drv");
}



// module_init 定义了一个结构体，这个结构体里面有一个函数指针，
// 指向 first_drv_init 这个驱动入口函数
module_init(first_drv_init)

module_exit(first_drv_exit)
