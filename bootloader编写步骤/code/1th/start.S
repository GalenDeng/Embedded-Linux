// 从数据手册查看计算公式 2 * (92 +8)*12/((1+2)*2^2) =200
#define S3C2440_MPLL_200MHZ     ((0x5c<<12)|(0x01<<4)|(0x02))
#define MEM_CTL_BASE			0x48000000
.text
.global  _start
_start:

/* 1. 关看门狗*/
	ldr r0, =0x53000000
	mov r1, #0
	str r1, [r0]
/* 2. 设置时钟*/  // CLKDIVN 分频配置
	ldr r0,=0x4C000014
	mov r1,#0x03;	// FCLK:HCLK:PCLK=1:2:4, HDIVN=1,PDIVN=1
	str r1,[r0]

/* 如果HDIVN非0，CPU的总线模式应该从“fast bus mode”变为“asynchronous bus mode” */
// 数据手册有给出例子
	mrc p15,0,r1,c1,c0,0
	orr	r1, r1, #0xc0000000			/* 设置为“asynchronous bus mode”*/
	mcr p15,0,r1,c1,c0,0

/* MPLLCON = S3C2440_MPLL_200MHZ */   // FCLK = 200MHZ HCLK = 100MHZ PCLK = 50MHZ
	ldr r0, =0x4C000004 			
	ldr r1, =S3C2440_MPLL_200MHZ
	str r1,[r0]

/* 3. SDRAM初始化 */
ldr r0, = MEM_CTL_BASE	// 0X48000000 BWSCON 存储控制器
adr r1, sdram_config	// sdram_config的当前地址
add r3, r0, #(13*4)

1:
	ldr r2, [r1], #4
	str r2, [r0], #4
	cmp r0, r3
	bne 1b


/* 4. 重定位： 把bootloader本身的代码从flash复制到SDRAM中 */
// 因为打算用 C语言编写代码, 所以要设置栈
	ldr sp, = 0x34000000
// 因为涉及到从nand flash中复制代码，所以要初始化 nand flash
	bl nand_init	

	mov r0, #0
	ldr r1, =_start   		// 程序开始的位置
	ldr r2, =__bss_start	// __bss_start - _start = 代码的大小len
	sub r2, r2, r1

	bl copy_code_to_sdram	// 复制代码到sdram
	bl clear_bss			// 清零 (bss：没被初始化的全局变量和初始化为0的全局变量)

/* 5. 执行main */
	ldr lr, =halt	// lr存放跳转的返回值
	ldr pc, =main	// 主函数

halt:			// 伪指令
	bl halt		// 防止程序跑偏，设置死循环

sdram_config:	// 伪指令 : 设置存储管理器
		  // 这项为 寄存器的设置值
	.long 0x22011110	 //BWSCON			// Bus width & wait status control register 
	.long 0x00000700	 //BANKCON0			// 存储控制器0  片选信号
	.long 0x00000700	 //BANKCON1			// 存储控制器1
	.long 0x00000700	 //BANKCON2
	.long 0x00000700	 //BANKCON3  
	.long 0x00000700	 //BANKCON4
	.long 0x00000700	 //BANKCON5
	.long 0x00018005	 //BANKCON6			// sdram  RAS to CAS delay ：4clock
	.long 0x00018005	 //BANKCON7
	.long 0x008C04F4	 // REFRESH			// SDRAM型号:K4S561632N 刷新频率  Refresh period = (2^11 -refresh_count+1)/HCLK period: 64ms
	.long 0x000000B1	 //BANKSIZE			
	.long 0x00000030	 //MRSRB6			// Mode register set register bank6
	.long 0x00000030	 //MRSRB7			// Mode register set register bank7
	





