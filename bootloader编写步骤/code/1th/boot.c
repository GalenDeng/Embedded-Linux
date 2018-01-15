#include "setup.h"
extern void uart0_init(void);
extern void nand_read(unsigned int addr,unsigned char *buf, unsigned int len);
extern void puts(char *str);
extern void puthex(unsigned int val);

static struct tag *params;
void	setup_start_tag(void)
{
	// 放置设置参数的地方 0x30000100
	params = (struct tag *)0x30000100;
	// tag_header内容: tag size
	params->hdr.tag = ATAG_CORE;	//  tag = 0x54410001 => 参数的开始
	params->hdr.size = tag_size (tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next (params);		// 设置标记的末尾位置，即指向当前标记的末尾
}
void	setup_memory_tags(void)
{
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size (tag_mem32);

		params->u.mem.start = 0x30000000;
		params->u.mem.size =  0x04000000;

		params = tag_next (params);
}

int strlen(char *str)
{
	int i = 0;
	while (str[i])
	{
		i++;
	}
	return i;
}

void strcpy(char *dest, char *src)
{
	while ((*dest++ = *src++) != '\0');
}

void	setup_commandline_tag(char *cmdline)
{
	int len = strlen(cmdline) + 1;
	
	params->hdr.tag  = ATAG_CMDLINE;
	params->hdr.size = (sizeof (struct tag_header) + len + 3) >> 2; // 加3比加4好

	strcpy (params->u.cmdline.cmdline, cmdline);		// 拷贝进tag列表中

	params = tag_next (params);
}
void	setup_end_tag(void)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}


int  main(void)
{
	void (*theKernel)(int zero, int arch, unsigned int params);
	volatile unsigned int *p = (volatile unsigned int *)0x30008000;
	/* 0. 帮内核设置串口: 内核启动的开始部分会从串口打印一些信息，但是内核一开始没有初始化串口 */
	uart0_init();

	/* 1. 从NAND FLASH里把内核读入内存 */
	puts("Copy kernel from nand\n\r");
	nand_read(0x60000 + 64,(unsigned char *)0x30008000,0x200000);	//src , dest , len
	puthex(0x1234ABCD);
	puts("\n\r");
	puthex(*p);
	puts("\n\r");
	/* 2. 设置参数 --- 通过tag的方式*/ 
	puts("Set boot params\n\r");
	setup_start_tag();
	setup_memory_tags();
	setup_commandline_tag("noinitrd root=/dev/nfs nfsroot=192.168.99.140:/work/nfs_root/app,rsize=1024,wsize=1024 ip=192.168.99.135:192.168.99.140:192.168.99.4:255.255.255.0::eth0:off init=/linuxrc console=ttySAC0");
	setup_end_tag();
	/* 3. 跳转执行 */
	puts("Boot kernel\n\r");
	theKernel = (void (*)(int, int, unsigned int))0x30008000; // 跳转到0x30008000
	theKernel(0,362,0x30000100);	
	/* 等同于 mov pc, #0x30008000 */
	/* mov r0, #0 */
	/* ldr r1, =362 */
	/* ldr r2, =0x30000100 */
	puts("Error!\n\r");
	/* 如果一切正常，不会执行到这里 */
	return -1;
}
