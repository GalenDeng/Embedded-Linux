
/* NAND FLASH控制器 */
#define NFCONF (*((volatile unsigned long *)0x4E000000))
#define NFCONT (*((volatile unsigned long *)0x4E000004))
#define NFCMMD (*((volatile unsigned char *)0x4E000008))
#define NFADDR (*((volatile unsigned char *)0x4E00000C))
#define NFDATA (*((volatile unsigned char *)0x4E000010))
#define NFSTAT (*((volatile unsigned char *)0x4E000020))

/* GPIO */
#define GPHCON              (*(volatile unsigned long *)0x56000070)
#define GPHUP               (*(volatile unsigned long *)0x56000078)

/* UART registers*/
#define ULCON0              (*(volatile unsigned long *)0x50000000)
#define UCON0               (*(volatile unsigned long *)0x50000004)
#define UFCON0              (*(volatile unsigned long *)0x50000008)
#define UMCON0              (*(volatile unsigned long *)0x5000000c)
#define UTRSTAT0            (*(volatile unsigned long *)0x50000010)
#define UTXH0               (*(volatile unsigned char *)0x50000020)
#define URXH0               (*(volatile unsigned char *)0x50000024)
#define UBRDIV0             (*(volatile unsigned long *)0x50000028)

#define TXD0READY   (1<<2)


void nand_read(unsigned int addr,unsigned char *buf, unsigned int len);

int isBootFromNorFlash(void)
{
	volatile int *p= (volatile int *)0;
	int val;
	val = *p;
	*p = 0x12345678;
	if (*p == 0x12345678)
	{
		/* 写成功，是nand flash启动 */
		*p = val;	// 恢复原来的值
		return 0;
	}
	else
	{
		/* nor flash不能像内存一样写 */
		return 1;
	}
}

void copy_code_to_sdram(unsigned char *src,unsigned char *dest,unsigned int len)
{
	int i=0;
	/* 如果是NOR FLASH启动 */
	if(isBootFromNorFlash())
	{
		while(i < len)
		{
			dest[i] = src[i];
			i++;
		}
	} 
	else
	{
		nand_read((unsigned int)src,dest,len);
	}
}

// 清理bss段
void clear_bss(void)
{
	extern int __bss_start ,__bss_end;
	int *p = &__bss_start;

	for(;p < &__bss_end;p++)	//遍历
	{
		*p = 0;		//清零
	}
}

void nand_init(void)
{
	// 时序设置 根据数据手册
#define TACLS 0
#define TWRPH0 1
#define TWRPH1 0
	NFCONF = ((TACLS << 12)|(TWRPH0 << 8)|(TWRPH1 << 4));
	/* 使能NAND Flash控制器, 初始化ECC, 禁止片选 */
	NFCONT = ((1 << 4) | (1 << 1) | (1 << 0));
}

/* Enable chip select */
void nand_select(void)
{
	NFCONT &= ~(1 << 1);
}

/* Disable chip select */
void nand_deselect(void)
{
	NFCONT |= (1 << 1);
}

// 发送命令
void nand_cmd(unsigned char cmd)
{
	volatile int i;
	NFCMMD = cmd;
	for(i=0;i<10;i++);
}
// 发送地址
void nand_addr(unsigned int addr)
{
	volatile int i;
	unsigned int col , row; 	// 列地址 行地址
	col = addr % 2048;
	row = addr / 2048;

	NFADDR = col & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = row & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (row >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (row >> 16) & 0xff;
	for (i = 0; i < 10; i++);
}

// 状态检测
void nand_read_ready(void)
{
	while(!(NFSTAT & 0x01));	
}

// 这里定义返回值为unsigned char,是因为原理图中的传送数据只有8位I/O
unsigned char nand_data(void)	
{
	return NFDATA;
}

// 读取 Nand FLASH的内容
void nand_read(unsigned int addr,unsigned char *buf, unsigned int len)
{
	int col = addr % 2048;
	int i = 0;
	/* 选中片选 */
	nand_select();
	while(i < len) 
	{
	/* 2. 发出读命令 */
	nand_cmd(0x00);
	/* 3. 发出地址(分5步发出) */
	nand_addr(addr);
	/* 4. 发出读命令30h */
	nand_cmd(0x30);
	/* 5. 判断状态 */
	nand_read_ready();
	/* 6. 读数据 */
	for(;(col < 2048) && (i < len); col++)			// 第一次处于寄存器的中间位置
		{
			buf[i] = nand_data();
			i++;
			addr++;
		}	
		col = 0;	
	}
	/* 7. 取消选中 */
	nand_deselect();
}

#define PCLK            50000000    // init.c中的clock_init函数设置PCLK为50MHz
#define UART_CLK        PCLK        //  UART0的时钟源设为PCLK
#define UART_BAUD_RATE  115200      // 波特率
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)

/* 1. 初始化UART0 */
/* 115200 8N1,无流控 */
void uart0_init()
{
    GPHCON  |= 0xa0;    // GPH2,GPH3用作TXD0,RXD0
    GPHUP   = 0x0c;     // GPH2,GPH3内部上拉

	ULCON0 = 0x03;			// 8位,无校验，1个停止位
	UCON0 = 0x05;			// 查询方式，uart的中断源为PCLK
	UFCON0 = 0x00;			// 不使用FIFO
	UMCON0 = 0x00;			// 不使用流控
	UBRDIV0 = UART_BRD;		// 设置波特率为 115200
}

// 发送一个字节
void putc(unsigned char c)
{
	// UTRSTAT0 & TXD0READY = 1 表示发送缓冲区的数据已经全部发送出去
	while(!(UTRSTAT0 & TXD0READY));	
	/* 向UTXH0寄存器中写入数据，UART即自动将它发送出去 */	
	UTXH0 = c;
}

void puts(char *str)
{
	int i=0;
	while(str[i])
	{
		putc(str[i]);
		i++;
	}
}

void puthex(unsigned int val)		// 输出16进制的值
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
