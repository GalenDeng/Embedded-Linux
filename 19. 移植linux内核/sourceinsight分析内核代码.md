## sourceinsight分析内核代码 (2017.12.12)
* `内核运行的最终目的` ： `运行应用程序(根文件系统)`
* `处理传入参数` :  `/arch/arm/kernel/head.S`
* `/arch/arm/kernel/head.S` 压缩为 `/arch/arm/boot/compressed`,要启动内核，需要添加`自解压代码`，即
*  `启动内核 = 自解压 + 压缩的内核` ，入口在 `自解压代码中`
*  `自解压代码作用`
```
解压压缩的内核，启动内核
```
## linux的启动过程包括两个阶段
*  `架构/开发板相关的引导过程` : `通常使用汇编语言编写`
*  `后续的通用启动过程` :  `start kernel` `主要使用C语言编写` `仍然有部分架构/开发板相关的代码` --- `setup-arch函数`
##  /arch/arm/kernel/head.S 分析
```
	.section ".text.head", "ax"
	.type	stext, %function
ENTRY(stext)
	msr	cpsr_c, #PSR_F_BIT | PSR_I_BIT | SVC_MODE @ ensure svc mode
						@ and irqs disabled
	mrc	p15, 0, r9, c0, c0		@ get processor id    //读取寄存器,获取处理器ID,看是否能支持该处理器
	bl	__lookup_processor_type		@ r5=procinfo r9=cpuid
	movs	r10, r5				@ invalid processor (r5=0)?
	beq	__error_p			@ yes, error 'p'          //不能支持的处理器跳到这里处理
	bl	__lookup_machine_type		@ r5=machinfo  //机器ID 相当于 u-boot的 bd->bi_arch_number的传入 r1
	movs	r8, r5				@ invalid machine (r5=0)?
	beq	__error_a			@ yes, error 'a'
	bl	__create_page_tables                         
// 创建列表 因为 vmlinux.lds 的 . = (0xc0000000) + 0x00008000; 为虚拟地址，不是真正存在的内存，
//所以要建列表,启动mmu

	ldr	r13, __switch_data		@ address to jump to after      //使能mmu
						@ mmu has been enabled
	adr	lr, __enable_mmu		@ return (PIC) address  

	.type	__switch_data, %object
__switch_data:
	.long	__mmap_switched
	.long	__data_loc			@ r4
	.long	__data_start			@ r5
	.long	__bss_start			@ r6
	.long	_end				@ r7
	.long	processor_id			@ r4
	.long	__machine_arch_type		@ r5
	.long	cr_alignment			@ r6
	.long	init_thread_union + THREAD_START_SP @ sp

/*
 * The following fragment of code is executed with the MMU on in MMU mode,
 * and uses absolute addresses; this is not position independent.
 *
 *  r0  = cp#15 control register
 *  r1  = machine ID
 *  r9  = processor ID
 */
	.type	__mmap_switched, %function          // 跳到 start kernel,start kernel 为第一个 内核C函数 
                                                // start kernel 来处理启动参数
__mmap_switched:
	adr	r3, __switch_data + 4

	b	start_kernel
     
```
```
* start kernel

	lock_kernel();
	tick_init();
	boot_cpu_init();
	page_address_init();
	printk(KERN_NOTICE);
	printk(linux_banner);				// 输出一些内核信息
	setup_arch(&command_line);			// 处理uboot传进来的启动参数
	setup_command_line(command_line);   // 处理uboot传进来的启动参数
	unwind_setup();
	setup_per_cpu_areas();
	smp_prepare_boot_cpu();	/* arch-specific boot-cpu hooks */
```
* `如`
```
Starting kernel ...

Uncompressing Linux...................................................................................................................... done, booting the kernel.
Linux version 2.6.22.6 (book@book-desktop) (gcc version 3.4.5) #1 Sat May 11 15:09:41 CST 2013
CPU: ARM920T [41129200] revision 0 (ARMv4T), cr=c0007177
Machine: SMDK2440
Memory policy: ECC disabled, Data cache writeback
CPU S3C2440A (id 0x32440001)
```
* 内核要处理的参数
![内核要处理的参数](https://github.com/GalenDeng/Embedded-Linux/blob/master/19.%20%E7%A7%BB%E6%A4%8Dlinux%E5%86%85%E6%A0%B8/linux%E5%86%85%E6%A0%B8%E5%90%AF%E5%8A%A8%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/%E5%86%85%E6%A0%B8%E8%A6%81%E5%A4%84%E7%90%86%E7%9A%84%E5%8F%82%E6%95%B0.JPG)
* parse : 解析
* `start kernel后要挂载根文件系统`，这样才能使用应用程序
* `内核第二阶段的启动流程`
```
* arch/arm/kernel/head.S
start kernel
	setup_arch(&command_line);			// 解析u-boot传入的启动参数
	setup_command_line(command_line);   // 解析u-boot传入的启动参数
	rest_init()
		kernel_init 
			prepare_namespace()
				mount_root() 			//挂载根文件系统
			init_post() 				//作用：打开/dev/console,执行应用程序
```
## 内核第二阶段具体的启动流程
![内核第二阶段具体的启动流程](https://github.com/GalenDeng/Embedded-Linux/blob/master/19.%20%E7%A7%BB%E6%A4%8Dlinux%E5%86%85%E6%A0%B8/linux%E5%86%85%E6%A0%B8%E5%90%AF%E5%8A%A8%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/%E5%86%85%E6%A0%B8%E7%AC%AC%E4%BA%8C%E9%98%B6%E6%AE%B5%E5%90%AF%E5%8A%A8%E6%B5%81%E7%A8%8B.JPG)
```
static void noinline __init_refok rest_init(void)
	__releases(kernel_lock)
{
	int pid;

	kernel_thread(kernel_init, NULL, CLONE_FS | CLONE_SIGHAND);	// 创建内核的线程 这里调用 kernel_init的函数
	numa_default_policy();
	pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
	kthreadd_task = find_task_by_pid(pid);
	unlock_kernel();

	/*
	 * The boot idle thread must execute schedule()
	 * at least one to get things moving:
	 */
	preempt_enable_no_resched();
	schedule();
	preempt_disable();

	/* Call into cpu_idle with preempt disabled */
	cpu_idle();
} 
```
1. `@ 为注释`
```
	.type	__lookup_machine_type, %function
__lookup_machine_type:
	adr	r3, 3b              @ r3 = 3b 的地址   实际存在的地址 -- 物理地址
/* 
3:	.long	.               // 3b的地址
	.long	__arch_info_begin
	.long	__arch_info_end
*/

	ldmia	r3, {r4, r5, r6}        @ r4 = "." 这是3b的虚拟地址，取决于编译器编译到这里的地址是啥, 
                                    @ r5 = __arch_info_begin , r6 = __arch_info_end
// __arch_info_begin  __arch_info_end 在内核里面找不到定义，只能在链接脚本里面找
// vmlinux.lds   
/*
  __arch_info_begin = .;
   *(.arch.info.init)           // 架构相关的初始化信息存放在这里  地址从  (0xc0000000) + 0x00008000 开始
  __arch_info_end = .;
*/
	sub	r3, r3, r4			@ get offset between virt&phys
	add	r5, r5, r3			@ convert virt addresses to
	add	r6, r6, r3			@ physical address space
1:	ldr	r3, [r5, #MACHINFO_TYPE]	@ get machine type
	teq	r3, r1				@ matches loader number?
	beq	2f				@ found
	add	r5, r5, #SIZEOF_MACHINE_DESC	@ next machine_desc
	cmp	r5, r6
	blo	1b
	mov	r5, #0				@ unknown machine
2:	mov	pc, lr
```
2. `grep .arch.info.init`
* galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ grep ".arch.info.init" * -nR
```
galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ grep ".arch.info.init" * -nR
arch/arm/kernel/vmlinux.lds.S:39:			*(.arch.info.init)
arch/arm/kernel/vmlinux.lds:306:   *(.arch.info.init)

Binary file arch/arm/mach-s3c2443/mach-smdk2443.o matches
Binary file arch/arm/mach-s3c2443/built-in.o matches
Binary file arch/arm/mach-s3c2440/built-in.o matches
Binary file arch/arm/mach-s3c2440/mach-smdk2440.o matches
Binary file arch/arm/mach-s3c2412/built-in.o matches
Binary file arch/arm/mach-s3c2412/mach-vstms.o matches
Binary file arch/arm/mach-s3c2412/mach-smdk2413.o matches
Binary file arch/arm/mach-s3c2410/built-in.o matches
Binary file arch/arm/mach-s3c2410/mach-smdk2410.o matches
Binary file arch/arm/mach-s3c2410/mach-qt2410.o matches

include/asm-arm/mach/arch.h:53: __attribute__((__section__(".arch.info.init"))) = {	\
include/asm/mach/arch.h:53: __attribute__((__section__(".arch.info.init"))) = {	\
galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ 
```
* `猜测支持 mach-s3c2443  mach-s3c2440 mach-s3c2412 mach-s3c2410 这些单板`
```
* include/asm-arm/mach/arch.h:53
#define MACHINE_START(_type,_name)			\
static const struct machine_desc __mach_desc_##_type	\
 __used							\
 __attribute__((__section__(".arch.info.init"))) = {	\
	.nr		= MACH_TYPE_##_type,		\
	.name		= _name,

#define MACHINE_END				\
};
```
* sourceinsight中查找 MACHINE_START 这个宏谁在使用
```
* Mach-smdk2440.c(arch/arm/mach-s3c2440)
MACHINE_START(S3C2440, "SMDK2440") // 估计是设置结构体  __attribute__((__section__(".arch.info.init")))
	/* Maintainer: Ben Dooks <ben@fluff.org> */
	.phys_io	= S3C2410_PA_UART,
	.io_pg_offst	= (((u32)S3C24XX_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C2410_SDRAM_PA + 0x100,

	.init_irq	= s3c24xx_init_irq,
	.map_io		= smdk2440_map_io,
	.init_machine	= smdk2440_machine_init,
	.timer		= &s3c24xx_timer,
MACHINE_END
```
* `所以： 这里的意思为：定义了一个结构体，类型为machine_desc，结构体的变量名称为：__mach_desc_S3C2440`
```
* include/asm-arm/mach/arch.h:53
#define MACHINE_START(_type,_name)			\
static const struct machine_desc __mach_desc_##_type	\    
//  __mach_desc_##_type =  __mach_desc_S3C2440  ##_type中的##仅仅为连词符号

 __used							\
 __attribute__((__section__(".arch.info.init"))) = {	\   
// 这里的 machine_desc 结构体被强制作为一个 .arch.info.init的段，并放在  __arch_info_begin = .; 和 //__arch_info_end = .;之间
	.nr		= MACH_TYPE_##_type,		\  
//  .nr		= MACH_TYPE_S3C2440
	.name		= _name,
//  .name		= "SMDK2440"
//然后把以下的东西写进来
	.phys_io	= S3C2410_PA_UART,
	.io_pg_offst	= (((u32)S3C24XX_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C2410_SDRAM_PA + 0x100,

	.init_irq	= s3c24xx_init_irq,
	.map_io		= smdk2440_map_io,
	.init_machine	= smdk2440_machine_init,
	.timer		= &s3c24xx_timer,
// #define MACHINE_END			\
// };      
};
```
* `struct machine_desc`
```
struct machine_desc {
	/*
	 * Note! The first four elements are used
	 * by assembler code in head-armv.S
	 */
	unsigned int		nr;		/* architecture number	*/
	unsigned int		phys_io;	/* start of physical io	*/
	unsigned int		io_pg_offst;	/* byte offset for io 
						 * page tabe entry	*/

	const char		*name;		/* architecture name	*/
	unsigned long		boot_params;	/* tagged list		*/

	unsigned int		video_start;	/* start of video RAM	*/
	unsigned int		video_end;	/* end of video RAM	*/

	unsigned int		reserve_lp0 :1;	/* never has lp0	*/
	unsigned int		reserve_lp1 :1;	/* never has lp1	*/
	unsigned int		reserve_lp2 :1;	/* never has lp2	*/
	unsigned int		soft_reboot :1;	/* soft reboot		*/
	void			(*fixup)(struct machine_desc *,
					 struct tag *, char **,
					 struct meminfo *);
	void			(*map_io)(void);/* IO mapping function	*/
	void			(*init_irq)(void);
	struct sys_timer	*timer;		/* system tick timer	*/
	void			(*init_machine)(void);
};
```
## ARM的linux内核vmlinux的启动过程(包括架构/开发板相关的引导过程 + 后续的通用启动过程)
![ARM的linux内核vmlinux的启动过程](https://github.com/GalenDeng/Embedded-Linux/blob/master/19.%20%E7%A7%BB%E6%A4%8Dlinux%E5%86%85%E6%A0%B8/linux%E5%86%85%E6%A0%B8%E5%90%AF%E5%8A%A8%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/ARM%E7%9A%84linux%E5%86%85%E6%A0%B8%E5%90%AF%E5%8A%A8%E8%BF%87%E7%A8%8B.JPG)
* zImage : 压缩格式的内核 --- 首先进行自解压得到vmlinux，然后执行vmlinux开始“正常的”的启动流程
 ## `搜索分区信息`
* galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ grep "\"bootloader\"" * -nR
```
arch/arm/mach-omap1/board-osk.c:55:	      .name		= "bootloader",
arch/arm/mach-sa1100/assabet.c:110:		.name		= "bootloader",
arch/arm/mach-sa1100/assabet.c:131:		.name		= "bootloader",
arch/arm/mach-sa1100/collie.c:184:		.name		= "bootloader",
arch/arm/plat-s3c24xx/common-smdk.c:120:        .name   = "bootloader",	// 应该是在这个文件里面修改的
arch/arm/mach-davinci/board-evm.c:42:		.name		= "bootloader",
arch/arm/mach-omap2/board-h4.c:83:	      .name		= "bootloader",
drivers/mtd/maps/sa1100-flash.c:72:		.name		= "bootloader",
drivers/mtd/maps/pq2fads.c:53:		.name		= "bootloader",
drivers/mtd/maps/wr_sbc82xx_flash.c:39:		.name =		"bootloader",
drivers/mtd/maps/wr_sbc82xx_flash.c:47:		.name =		"bootloader",
```
* 查看 `arch/arm/plat-s3c24xx/common-smdk.c`
```
* 可见分区信息被写死了
static struct mtd_partition smdk_default_nand_part[] = {
	[0] = {
        .name   = "bootloader",
        .size   = 0x00040000,
		.offset	= 0,
	},
	[1] = {
        .name   = "params",
        .offset = MTDPART_OFS_APPEND,  	// OFS_APPEND 表示紧接上一个分区的意思
        .size   = 0x00020000,
	},
	[2] = {
        .name   = "kernel",
        .offset = MTDPART_OFS_APPEND,
        .size   = 0x00200000,
	},
	[3] = {
        .name   = "root",
        .offset = MTDPART_OFS_APPEND,
        .size   = MTDPART_SIZ_FULL,
	}
};
```