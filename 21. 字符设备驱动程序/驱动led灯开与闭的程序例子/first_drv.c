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
static struct class  *firstdrv_class;		// ��
static struct class_device  *firstdrv_class_dev;	// �������ٽ�һ���豸

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;

static int first_drv_open(struct inode *inode, struct file *file)
{
	//printk(" first_drv_open\n");
/******����GPF4 ,GPF5,GPF7Ϊ���******/
*gpfcon &=  ~((0x3 << (4 * 2)) | (0x3 << (5 * 2)) | (0x3 << (6 * 2)));	// ����ϰ�ߣ�����0
*gpfcon |=  ((0x1 << (4 * 2)) | (0x1 << (5 * 2))  | (0x1 << (6 * 2)));		// ��1 ������Ϊ�����ģʽ
	return 0;
}

static int first_drv_write(struct file *file, const __user * buf ,size_t count , loff_t * ppos) // buf : �����Ӧ�ò㴫������buf
{
	int val;
	copy_from_user(&val,  buf, count);	// ���ַ�ʽ��Ӧ�ò�ȡֵ, ������val�ĵ�ַ����
     //  copy_to_user(void __user * to, const void * from, unsigned long n)   ���ں˿ռ临�����ݵ��û��ռ�

	if (val == 1)
	{
		//���
		*gpfdat &= ~((1 << 4) | (1 << 5) |(1 << 6) );
	}
	else
	{
		//���
		*gpfdat |= ((1 << 4) | (1 << 5) |(1 << 6) );
	}
	//printk("  first_drv_write\n");
	return 0;
}

static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =   first_drv_open,     
    .write  =   first_drv_write,
};


int major;
// ע�᣺ major �����豸��  first_dev ������(��������� )    file_operations : �ṹ �������
// �ṹ�����ں�
static  int first_drv_init(void)
{
major = register_chrdev(0, "first_drv",  &first_drv_fops);

firstdrv_class = class_create(THIS_MODULE,"firstdrv"); // ����һ����ʵ��
if (IS_ERR(firstdrv_class))
	return PTR_ERR(firstdrv_class);

// ���´���һ���豸
firstdrv_class_dev = class_device_create(firstdrv_class, NULL, MKDEV(major,0), NULL, "xyz") ;
// firstdrv ������»ᴴ��xyz ����豸/*/dev/xyz ����豸�ڵ�
if (unlikely(IS_ERR(firstdrv_class_dev))) 
	return PTR_ERR(firstdrv_class_dev);

gpfcon = (volatile unsigned long *)ioremap(0x56000050 , 16); // 0x56000060 : ��ʼ��ַ16 ��16���ֽڴ�С
gpfdat = gpfcon + 1;	// ��ַ + 1

return 0;
}

// ж������
 static  void first_drv_exit(void)
{
unregister_chrdev(major, "first_drv");

class_device_unregister(firstdrv_class_dev);	//ɾ���豸
class_destroy(firstdrv_class);				// ɾ����
iounmap(gpfcon);						// ɾ��ӳ��
}



// module_init ������һ���ṹ�壬����ṹ��������һ������ָ�룬
// ָ�� first_drv_init ���������ں���
module_init(first_drv_init);

module_exit(first_drv_exit);

MODULE_LICENSE("GPL");

