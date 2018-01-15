## bootloader编写步骤 (2018.1.4)
* `最简单的bootloader的编写步骤`
1. 初始化硬件、关看门狗、设置SDRAM、初始化Nand Flash
2. 如果bootloader比较大，要把它重定位到SDRAM
2. 把内核从Nand flash读到SDRAM
3. 设置“要传给内核的参数”
4. 跳转执行内核

* `改进 --- 提高内核启动的速度`
1. 改变时钟: 200MHZ => 400MHZ , 记住这个时候还要改变分频系数 // 实践结果表示:提高频率对提高内核的启动速度影响不大
2. 启动ICACHE (如果要启动DCACHE的话，需要MMU,设置虚拟地址的映射)

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
![len向四取整的理解](https://github.com/GalenDeng/Embedded-Linux/blob/master/bootloader%E7%BC%96%E5%86%99%E6%AD%A5%E9%AA%A4/bootloader%E7%BC%96%E5%86%99%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/len%E5%90%91%E5%9B%9B%E5%8F%96%E6%95%B4%E7%9A%84%E7%90%86%E8%A7%A3.JPG)
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
14. `设置串口`
```
* 启动过程中需要有打印信息提示用户启动过程中的状态，所以我们需要初始化一个串口
```
* /* 0. 帮内核设置串口: 内核启动的开始部分会从串口打印一些信息,但是内核一开始没有初始化串口 */
*	uart0_init();
15.`\r \n \n\r` 
```
\n是换行，英文是New line，表示使光标到行首
\r是回车，英文是Carriage return，表示使光标下移一格
\r\n表示回车换行
```
* puts("Copy kernel from nand\n\r");
16. `arm-linux-ar`
* AR 		= arm-linux-ar    // AR : 把所有的东西整合成一个库
17. `make出现的问题解决`
```
* 1. 要进行外部声明
oot.c:70: warning: implicit declaration of function `uart0_init'
boot.c:73: warning: implicit declaration of function `puts'
boot.c:74: warning: implicit declaration of function `nand_read'
* 2. 删除该行代码，因为没用
boot.c:41: warning: unused variable `tmp'
* 3. 和系统库的strcpy和puts冲突，在Makefile中的 CPPFLAGS 添加 -nostdlib -nostdinc -fno-builtin
* 即 CPPFLAGS   	:= -nostdinc -nostdlib -fno-builtin
boot.c:40: warning: conflicting types for built-in function 'strcpy'
init.c:201: warning: conflicting types for built-in function 'puts'
* 4.出现以下的问题,用 touch boot.c(文件名)更新文件的系统时间即可
galen@HD66:/work/nfs_root/bootloader-write-myself/1th$ make
make: Warning: File `boot.c' has modification time 9.4e+04 s in the future
arm-linux-gcc -nostdinc -nostdlib -fno-builtin -Wall -O2 -c -o boot.o boot.c
arm-linux-ld -Tboot.lds -o boot.elf start.o init.o boot.o
arm-linux-objcopy -O binary -S boot.elf boot.bin
arm-linux-objdump -D -m arm boot.elf > boot.dis
make: warning:  Clock skew detected.  Your build may be incomplete.
galen@HD66:/work/nfs_root/bootloader-write-myself/1th$ touch boot.c
```
18. 
```
* 33f80034:	e28f103c 	add	r1, pc, #60	; 0x3c   
// 有 PC 就必须在原来的值下加上 8 , 这里是 pc = 34 +8 = 3c , 所以 3c + 3c = 78; 即跳到 33f80078
// 即是
33f80078 <sdram_config>:
33f80078:	22011110 	andcs	r1, r1, #4	; 0x4
```
19. `start.S boot.lds boot.dis的联系 `
```
* boot.dis
* 33f80058:	e59f1058 	ldr	r1, [pc, #88]	; 33f800b8 <sdram_config+0x40>

* 33f800b8:	33f80000 	mvnccs	r0, #0	; 0x0   // 33f80000

* boot.lds
SECTIONS {
	. = 0x33f80000;
	.text : { *(.text) }

	. = ALIGN(4);	
	.rodata : { *(.rodata) }

	. = ALIGN(4);
	.data : { *(.data) }

	. = ALIGN(4);
	__bss_start = . ;
	.bss : { *(.bss) *(COMMON) }
	__bss_end = . ;
}
```
20. `通过 boot.dis反汇编文件和 start.S来查看代码的大小`
```
33f80050:	eb000031 	bl	33f8011c <nand_init>
33f80054:	e3a00000 	mov	r0, #0	; 0x0
33f80058:	e59f1058 	ldr	r1, [pc, #88]	; 33f800b8 <sdram_config+0x40>    // 33f800b8
33f8005c:	e59f2058 	ldr	r2, [pc, #88]	; 33f800bc <sdram_config+0x44>    // 33f800bc
33f80060:	e0422001 	sub	r2, r2, r1
33f80064:	eb0000cc 	bl	33f8039c <copy_code_to_sdram>

33f800b8:	33f80000 	mvnccs	r0, #0	; 0x0
33f800bc:	33f80790 	mvnccs	r0, #37748736	; 0x2400000

* 所以代码大小为: 33f80790 - 33f80000 = 790(hex) = 1936
* 而从linux系统中也能看到其代码大小大致相同 (1933) --- ( 使用 . = ALIGN(4)取整 把 1933 变为 1936)
galen@HD66:/work/nfs_root/bootloader-write-myself/1th$ ls -lt boot.bin 
-rwxrwxr-x 1 galen galen 1933 Jan 13 13:57 boot.bin
```
21. `mtd ; nand dump 60000`
```
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
OpenJTAG> nand dump 60000
Page 00060000 dump:
        27 05 19 56 47 76 ea 15  51 8d ee be 00 1c 35 5c
        30 00 80 00 30 00 80 00  e3 00 1c fb 05 02 02 00
        4c 69 6e 75 78 2d 32 2e  36 2e 32 32 2e 36 00 00
        00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00      //  前面的4行表示头部 --- 64字节

        // 真正的内核 0x30008000
        00 00 a0 e1 00 00 a0 e1  00 00 a0 e1 00 00 a0 e1
        00 00 a0 e1 00 00 a0 e1  00 00 a0 e1 00 00 a0 e1
        02 00 00 ea 18 28 6f 01  00 00 00 00 5c 35 1c 00
        01 70 a0 e1 02 80 a0 e1  00 20 0f e1 03 00 12 e3
        01 00 00 1a 17 00 a0 e3  56 34 12 ef 00 20 0f e1
        c0 20 82 e3 02 f0 21 e1  00 00 00 00 00 00 00 00
        d0 00 8f e2 7e 30 90 e8  01 00 50 e0 0a 00 00 0a
        00 50 85 e0 00 60 86 e0  00 c0 8c e0 00 20 82 e0

OOB:                                                         // ECC校验
        ff ff ff ff ff ff ff ff
        ff ff ff ff ff ff ff ff
        ff ff ff ff ff ff ff ff
        ff ff ff ff ff ff ff ff
        ff ff ff ff ff ff ff ff
        95 9a 97 96 a5 6b 0f 3f
        03 f3 cc 33 03 0f f3 5a
        9a 5b 96 59 ab ff fc cf
OpenJTAG> 
```
22. `16进制显示`
```
void puthex(unsigned int val)
{
	/* 0x1234abcd */
	int i;
	int j;
	
	puts("0x");

	for (i = 0; i < 8; i++)
	{
		j = (val >> ((7-i)*4)) & 0xf;
		if ((j >= 0) && (j <= 9))
			putc('0' + j);
		else
			putc('A' + j - 0xa);
		
	}
	
}
```
23. `判断内核是否被 nand_read正常读取的方法`
```
        // 真正的内核 0x30008000  
        00 00 a0 e1 00 00 a0 e1  00 00 a0 e1 00 00 a0 e1	// 为 e1a00000
而从启动的内核来看：
Copy kernel from nand
0x1234ABCD
0xE1A00000	// 和 nand dump 60000 命令抓取到的正常内核启动内容 e1a00000 一样， 即 nand_read()读取成功
而从启动的内核来看：
Set boot params
Boot kernel
Uncompressing Linux......
```
24. `ICACHE加速内核启动速度 --- cpu/arm920t`
```
1. uboot源码中搜索 ICACHE =>  icache_enable
*  icache_enable
void icache_enable (void)
{
	ulong reg;

	reg = read_p15_c1 ();		/* get control reg. */
	cp_delay ();
	write_p15_c1 (reg | C1_IC);
}

* read_p15_c1 ()
/* read co-processor 15, register #1 (control register) */
static unsigned long read_p15_c1 (void)
{
	unsigned long value;

	__asm__ __volatile__(
		"mrc	p15, 0, %0, c1, c0, 0   @ read control reg\n"
		: "=r" (value)
		:
		: "memory");

#ifdef MMU_DEBUG
	printf ("p15/c1 is = %08lx\n", value);
#endif
	return value;
}

* 取 mrc	p15, 0, %0, c1, c0, 0   @ read control reg
// mrc : 
* MRC指令将协处理器的寄存器中数值传送到ARM处理器的寄存器中。如果协处理器不能成功地执行该操作，将产生未定义的指令异常中断
* MCR指令将ARM处理器的寄存器中的数据传送到协处理器的寄存器中。如果协处理器不能成功地执行该操作，将产生未定义的指令异常中断
* 协处理器可以通过一组专门的、提供load-store类型接口的ARM指令来访问。例如协处理器15（CP15），ARM处理器使用协处理器15的寄存器来控制cache、TCM和存储器管理
// mrc ； mov register compilation(编辑/编译/汇编)

* 修改 mrc	p15, 0, r0, c1, c0, 0   @ read control reg	// 从协处理器中读取值到r0中
* 由 write_p15_c1 (reg | C1_IC); 且 #define C1_IC		(1<<12)		/* icache off/on */ 所以得出
  orr 	r0, r0, #(1 << 12)		// r0 = r0 | (1 << 12)
* 又因为 
/* write to co-processor 15, register #1 (control register) */
static void write_p15_c1 (unsigned long value)
{
#ifdef MMU_DEBUG
	printf ("write %08lx to p15/c1\n", value);
#endif
	__asm__ __volatile__(
		"mcr	p15, 0, %0, c1, c0, 0   @ write it back\n"
		:
		: "r" (value)
		: "memory");

	read_p15_c1 ();
}
* 所以要把值写回到协处理器里面,即
* mcr	p15, 0, r0, c1, c0, 0   @ write it back
* ICACHE启动总结；

/* 启动ICACHE */  ----  12s 变为 2s 启动  => ICACHE的威力
 mrc	p15, 0, r0, c1, c0, 0   @ read control reg
 orr 	r0, r0, #(1 << 12)
 mcr	p15, 0, r0, c1, c0, 0   @ write it back
```
25. `orr`
```
2，orr
ORR指令的格式为：
ORR{条件}{S}  目的寄存器，操作数1，操作数2
ORR指令用于在两个操作数上进行逻辑戒运算，并把结果放置到目的寄存器中。操作数1应该是一
个寄存器，操作数2可以是一个寄存器，被移位的寄存器，或一个立即数。该指令常用于设置操
作数1的某些位。
指令示例：
ORR R0，R0，＃3          ；  该指令设置R0的0、1位，其余位保持不变。

orr r0,r0,#0xd3
    0xd3=1101 0111
    将r0与0xd3作算数或运算，然后将结果返还给r0,即把r0的bit[7:6]和bit[4]和bit[2:0]置为1
```
26. `ICACHE理解`
* 如下图，如果没有ICACHE的话,在内核启动的过程中,运行地址一直在CPU和SDRAM之间来回切换，在SDRAM取指令，在CPU中执行;
* 有了ICACHE(指令缓存)后,我们在第一次取SDRAM的指令的时候会把一部分的指令代码缓存到ICACHE中，下次CPU需要指令的时候
* 现在ICACHE中看有没有相对应的指令，有的话就直接使用,没有的时候才从SDRAM取指令(这个时候也会取一部分的指令代码再次
* 放到ICACHE中,方便使用,提高效率)
## ICACHE理解图示
![ICACHE理解图示](https://github.com/GalenDeng/Embedded-Linux/blob/master/bootloader%E7%BC%96%E5%86%99%E6%AD%A5%E9%AA%A4/bootloader%E7%BC%96%E5%86%99%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/ICACHE%E7%90%86%E8%A7%A3.JPG)