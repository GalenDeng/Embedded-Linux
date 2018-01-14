## bootloader编写步骤 (2018.1.4)
* 最简单的bootloader的编写步骤
1. 初始化硬件、关看门狗、设置SDRAM、初始化Nand Flash
2. 如果bootloader比较大，要把它重定位到SDRAM
2. 把内核从Nand flash读到SDRAM
3. 设置“要传给内核的参数”
4. 跳转执行内核

1. nand flash
* OOB : out of bank : 这个结构的出现是因为nand flash的缺陷:位反转
```
OOB描述：
* OOB中的第0个字节
* 这一页中的第2048字节
```
* 避免位反转方法：生成ECC校验码，把ECC写进OOB中
* nand flash 读写:
```
* 写：写页数据，生成ECC校验码，把ECC写进OOB中
* 读：读页数据，根据该页数据算出ECC校验码，把算出的ECC码和读OOB中的ECC码比较，
  相同即表示读数据没有出错，不相等的话通过某种算法来修正数据
```
2. `查看板子的kernel存放位置 --- 通过 MTD 命令查询`
```
##### 100ask Bootloader for OpenJTAG #####
[n] Download u-boot to Nand Flash
[o] Download u-boot to Nor Flash
[k] Download Linux kernel uImage
[j] Download root_jffs2 image
[y] Download root_yaffs image
[d] Download to SDRAM & Run
[z] Download zImage into RAM
[g] Boot linux from RAM
[f] Format the Nand Flash
[s] Set the boot parameters
[b] Boot the system
[r] Reboot u-boot
[q] Quit from menu
Enter your selection: q
OpenJTAG> mtd

device nand0 <nandflash0>, # parts = 4
 #: name                        size            offset          mask_flags
 0: bootloader          0x00040000      0x00000000      0
 1: params              0x00020000      0x00040000      0
 2: kernel              0x00200000      0x00060000      0
 3: root                0x0fda0000      0x00260000      0

active partition: nand0,0 - (bootloader) 0x00040000 @ 0x00000000

defaults:
mtdids  : nand0=nandflash0
mtdparts: mtdparts=nandflash0:256k@0(bootloader),128k(params),2m(kernel),-(root)
OpenJTAG> 
```
3. `uImage`
* uImage = 64字节的头部 + 真正的 Image
0x00060000    64        0x00060000 + 64
4. `加载地址`
```
## Booting image at 30007fc0 ...
   Image Name:   Linux-2.6.22.6
   Created:      2013-05-11   7:09:50 UTC
   Image Type:   ARM Linux Kernel Image (uncompressed)
   Data Size:    1848668 Bytes =  1.8 MB
   Load Address: 30008000               // 加载地址
   Entry Point:  30008000               // 入口点
   Verifying Checksum ... OK
   XIP Kernel Image ... OK

Starting kernel ...
```
* nand_read(0x00060000 + 64,0x30008000,0x200000);	//src , dest , len
5. `搜索bootm`
```
#ifdef CONFIG_CMDLINE_TAG     // 可看出这个宏为 1 才会有 bootargs
	char *commandline = getenv ("bootargs");
#endif
```
6. `跳转执行核心代码`
* void (*theKernel)(int zero, int arch, uint params); // 函数指针
* theKernel();	/* 等同于 mov pc, #0x30008000 */
7. `设置参数 --- 通过 TAG 的方式 --- include/asm-arm/setup.h`
```
	setup_start_tag ();
	setup_memory_tags ();
	setup_commandline_tag ();
	setup_end_tag ();
```
* 通过 project -> add and remove project file 来 复制 setup.h的路径，然后在我
* 的电脑的网址粘贴该网址来打开文件的绝对路径
8. `修改setup.h`
```
* 删除以下代码
#define __tag __attribute__((unused, __section__(".taglist")))
#define __tagtable(tag, fn) \
static struct tagtable __tagtable_##fn __tag = { tag, fn }
```
9. 
```
#define tag_size(type)	((sizeof(struct tag_header) + sizeof(struct type)) >> 2)
```
```
params->hdr.size = tag_size (tag_core);

struct tag_header {
	u32 size;       // SIZE = 4 (4字节)
	u32 tag;        // SIZE = 4 (4字节)
};

struct tag_core {
	u32 flags;		/* bit 0 = read-only */   // SIZE = 4 (4字节)
	u32 pagesize;                           // SIZE = 4 (4字节)
	u32 rootdev;                            // SIZE = 4 (4字节)
};

*  >> 2 等同于 除以整数 2^2 =4 ,其中 ^2 的2表示 >>2的2
* 所以 ((sizeof(struct tag_header) + sizeof(struct type)) >> 2) = 20 /4 = 5
```
10. 
* CONFIG_NR_DRAM_BANKS    // 表示有多少块内存,就可以作多少个内存标记
```
* 这里只有一块内存
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
* 所以以下代码的某部分可省略
#ifdef CONFIG_SETUP_MEMORY_TAGS   // 可省略
static void setup_memory_tags (bd_t *bd)
{
	int i;  // 可省略

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {    // 可省略
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size (tag_mem32);

		params->u.mem.start = bd->bi_dram[i].start;
		params->u.mem.size = bd->bi_dram[i].size;

		params = tag_next (params);
	}   // 可省略
}
#endif /* CONFIG_SETUP_MEMORY_TAGS */   // 可省略
```
```
* 因为
int dram_init (void)
{
    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

    return 0;
}

#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x30000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000 /* 64 MB */

* 所以
void	setup_memory_tags(void)
{
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size (tag_mem32);

		params->u.mem.start = 0x30000000;
		params->u.mem.size =  0x04000000;

		params = tag_next (params);
}
```
11. `len的向4取整 --- 即1个字节的时候当作4来算`
```
* 做法
int strlen(char *str)
{
	int i = 0;
	while (str[i])
	{
		i++;
	}
	return i;
}

void	setup_commandline_tag(char *cmdline)
{
	int len = strlen(cmdline) + 1;	// 即加上结束字符 
	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size =
		(sizeof (struct tag_header) + len + 3) >> 2;	// 加3比加4好
```
* 加3取整的理解
```
(len+3) >> 2  // 即除以4
100 => 25  *4   100
101 => 26  *4   104
102 => 26  *4   104
103 => 26  *4   104
104 => 26  *4   104
105 => 27  *4   108
```
## len向四取整的理解
![len向四取整的理解]()https://github.com/GalenDeng/Embedded-Linux/blob/master/bootloader%E7%BC%96%E5%86%99%E6%AD%A5%E9%AA%A4/bootloader%E7%BC%96%E5%86%99%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/len%E5%90%91%E5%9B%9B%E5%8F%96%E6%95%B4%E7%9A%84%E7%90%86%E8%A7%A3.JPG
12. `bootargs`
```
bootargs=noinitrd root=/dev/nfs nfsroot=192.168.99.140:/work/nfs_root/app,rsize=1024,wsize=1024 ip=192.168.99.135:192.168.99.140:192.168.99.4:255.255.255.0::eth0:off init=/linuxrc console=ttySAC0
```
* 所以要设置
```
setup_commandline_tag ("noinitrd root=/dev/nfs nfsroot=192.168.99.140:/work/nfs_root/app,rsize=1024,wsize=1024 ip=192.168.99.135:192.168.99.140:192.168.99.4:255.255.255.0::eth0:off init=/linuxrc console=ttySAC0");
```
13. `kernel`
```
* void (*theKernel)(int zero, int arch, unsigned int params);
int zero => 0   /* mov r0, #0 */   => 第一个参数
int arch => 机器ID(即单板属于哪一个ID,让uboot知道)

* theKernel (0, bd->bi_arch_number, bd->bi_boot_params);
* gd->bd->bi_arch_number = MACH_TYPE_S3C2440;
* #define MACH_TYPE_S3C2440              362

int arch => 362   /* ldr r1, =362 */
```
```
	/* 3. 跳转执行 */
	theKernel = (void (*)(int, int, unsigned int))0x30008000; // 跳转到0x30008000
	theKernel(0,362,0x30000100);	
	/* 等同于 mov pc, #0x30008000 */
	/* mov r0, #0 */
	/* ldr r1, =362 */
	/* ldr r2, =0x30000100 */
```
## 设置参数图解
![设置参数列表分析](https://github.com/GalenDeng/Embedded-Linux/blob/master/bootloader%E7%BC%96%E5%86%99%E6%AD%A5%E9%AA%A4/bootloader%E7%BC%96%E5%86%99%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/%E8%AE%BE%E7%BD%AE%E5%8F%82%E6%95%B0%E5%88%97%E8%A1%A8%E5%88%86%E6%9E%90.png)