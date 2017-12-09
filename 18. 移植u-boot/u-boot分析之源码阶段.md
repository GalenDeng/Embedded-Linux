## u-boot分析之源码阶段 (2017.12.09)
* bss : 没有初始化的静态变量或全局变量 + 初始值为0的静态变量或全局变量
## u-boot内存使用情况
![u-boot内存使用情况](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E5%88%86%E6%9E%90%E4%B9%8B%E6%BA%90%E7%A0%81%E9%98%B6%E6%AE%B5%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/u-boot%E5%86%85%E5%AD%98%E4%BD%BF%E7%94%A8%E6%83%85%E5%86%B5.JPG)
## u-boot分析
![u-boot分析](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E5%88%86%E6%9E%90%E4%B9%8B%E6%BA%90%E7%A0%81%E9%98%B6%E6%AE%B5%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/u-boot%E6%BA%90%E7%A0%81%E5%88%86%E6%9E%90--%E6%9D%A1%E4%BB%B6.JPG)
## u-boot启动的第一阶段
![u-boot启动的第一阶段](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E5%88%86%E6%9E%90%E4%B9%8B%E6%BA%90%E7%A0%81%E9%98%B6%E6%AE%B5%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/u-boot%E5%90%AF%E5%8A%A8%E7%9A%84%E7%AC%AC%E4%B8%80%E9%98%B6%E6%AE%B5.JPG)
## u-boot启动的第二阶段
![u-boot启动的第二阶段](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E5%88%86%E6%9E%90%E4%B9%8B%E6%BA%90%E7%A0%81%E9%98%B6%E6%AE%B5%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/u-boot%E5%90%AF%E5%8A%A8%E7%9A%84%E7%AC%AC%E4%BA%8C%E9%98%B6%E6%AE%B5.JPG)
## u-boot的核心
![u-boot的核心](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E5%88%86%E6%9E%90%E4%B9%8B%E6%BA%90%E7%A0%81%E9%98%B6%E6%AE%B5%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/u-boot%E7%9A%84%E6%A0%B8%E5%BF%83.JPG)

1. `cpu/arm920t/start.S`
```
ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot*/   // 设置地址 若为 0x33f80000
sub	r0, r0, #CFG_MALLOC_LEN	/* malloc area*/                  // 减地址
```
![栈设置](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E5%88%86%E6%9E%90%E4%B9%8B%E6%BA%90%E7%A0%81%E9%98%B6%E6%AE%B5%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/sub%20%E4%B8%8E%20%E6%A0%88.JPG)
* 栈设置好了才能调用C函数
```
        /* configure UPLL */
        clk_power->UPLLCON = S3C2440_UPLL_48MHZ;        // USB的时钟

        /* configure MPLL */
        clk_power->MPLLCON = S3C2440_MPLL_400MHZ;       // 整个系统的时钟
```
```
#ifndef CONFIG_SKIP_RELOCATE_UBOOT                                 
 //作用：把代码从 flash 读到 SDRAM的链接地址里面   
relocate:				/* relocate U-Boot to RAM	    */
	adr	r0, _start		/* r0 <- current position of code   */
	ldr	r1, _TEXT_BASE	/* test if we run from flash or RAM */
	cmp     r0, r1      /* don't reloc during debug         */
	beq     clear_bss
	
	ldr	r2, _armboot_start
	ldr	r3, _bss_start
	sub	r2, r3, r2		/* r2 <- size of armboot*/
#if 1
	bl  CopyCode2Ram	/* r0: source, r1: dest, r2: size*/
#else
	add	r2, r0, r2		/* r2 <- source end address*/

copy_loop:
	ldmia	r0!, {r3-r10}	/* copy from source address [r0] */
	stmia	r1!, {r3-r10}	/* copy to   target address [r1] */
	cmp	r0, r2			    /* until source end addreee [r2]*/
	ble	copy_loop
#endif
```
```
clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment   */
	ldr	r1, _bss_end		/* stop here                   */
	mov 	r2, #0x00000000		/* clear                   */   // r0 - r1 段 清零
```
```
ldr	pc, _start_armboot
_start_armboot:	.word start_armboot    // 调用 C 函数
```

2. `启动内核`
* bootcmd=nand read.jffs2 0x30007FC0 kernel; bootm 0x30007FC0
// 在 nand flash 里面,把 kernel(分区) 读取到  0x30007FC0 这里 , 然后 boot 从  0x30007FC0 这里启动内核
## boot启动内核具体方式
![boot启动内核具体方式](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E5%88%86%E6%9E%90%E4%B9%8B%E6%BA%90%E7%A0%81%E9%98%B6%E6%AE%B5%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/boot%E5%90%AF%E5%8A%A8%E5%86%85%E6%A0%B8%E5%85%B7%E4%BD%93%E6%96%B9%E5%BC%8F.JPG)
