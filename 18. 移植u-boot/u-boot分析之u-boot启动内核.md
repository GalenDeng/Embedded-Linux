## u-boot分析之u-boot启动内核 (2017.12.10)
## 分区
![分区](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E5%90%AF%E5%8A%A8%E5%86%85%E6%A0%B8%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/%E5%88%86%E5%8C%BA.JPG)
1. `分区配置 -- 100ask24x0.h(include/configs)`
```
* 写死分区表
#define MTDIDS_DEFAULT "nand0=nandflash0"
#define MTDPARTS_DEFAULT "mtdparts=nandflash0:256k@0(bootloader)," \        //bootloader
                            "128k(params)," \                               // env 环境参数
                            "2m(kernel)," \                                 // kernel -- 内核
                            "-(root)"                                       // 根文件系统
```
* openJTAG > mtd    // 查看分区

* nand read.jffs2 0x30007FC0 kernel 等价于
nand read.jffs2 0x30007FC0 0x00200000  0x00060000
* 因为 ：
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
```
* `所以说分区的名字(kernel)不重要，重要的是它代表的起始地址和结束地址`
## u-boot启动内核原理分析
![u-boot启动内核原理分析](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E5%90%AF%E5%8A%A8%E5%86%85%E6%A0%B8%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/u-boot%E5%90%AF%E5%8A%A8%E5%86%85%E6%A0%B8%E5%8E%9F%E7%90%86%E5%88%86%E6%9E%90.JPG)

##  内核加载 : uImage = header + 真正的内核(0x30008000) 
* header : hex : 40 ; DEC : 64 
* 我们设置的 bootm 0x30007FC0
*  0x30007FC0 + header的size = 真正内核的地址 , 这样就能加快启动速率，免得重新设置
```
typedef struct image_header {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/      // 内核加载的真正地址
	uint32_t	ih_ep;		/* Entry Point Address		*/      //程序入口地址
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
} image_header_t;
```
```
	switch (hdr->ih_comp) {
	case IH_COMP_NONE:
		if(ntohl(hdr->ih_load) == data) {
			printf ("   XIP %s ... ", name);
		} else {
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
			size_t l = len;
			void *to = (void *)ntohl(hdr->ih_load);
			void *from = (void *)data;

			printf ("   Loading %s ... ", name);

			while (l > 0) {
				size_t tail = (l > CHUNKSZ) ? CHUNKSZ : l;
				WATCHDOG_RESET();
				memmove (to, from, tail);
				to += tail;
				from += tail;
				l -= tail;
			}
```
## do_bootm_linux的作用
![do_bootm_linux的作用](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E5%90%AF%E5%8A%A8%E5%86%85%E6%A0%B8%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/do_bootm_linux%E7%9A%84%E4%BD%9C%E7%94%A8.JPG)
* `启动内核 ： do_bootm_linux` 
``` 
* theKernel (0, bd->bi_arch_number, bd->bi_boot_params);    // Armlinux.c(lib_arm)
* theKernel = (void (*)(int, int, uint))ntohl(hdr->ih_ep);  // hdr->ih_ep 头部->入口地址
* bd->bi_arch_number  
// 机器ID 100ask24x0.c (board\100ask24x0):  gd->bd->bi_arch_number = MACH_TYPE_S3C2440;
```
## 启动内核分析各个TAG的作用
![启动内核分析各个TAG的作用](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E5%90%AF%E5%8A%A8%E5%86%85%E6%A0%B8%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/%E5%90%AF%E5%8A%A8%E5%86%85%E6%A0%B8%E5%88%86%E6%9E%90%E5%90%84%E4%B8%AATAG%E7%9A%84%E4%BD%9C%E7%94%A8.JPG)
* `TAG`
```
setup_start_tag (bd);
setup_memory_tags (bd);
setup_commandline_tag (bd, commandline);
setup_end_tag (bd);
```
```
static void setup_start_tag (bd_t *bd)
{
	params = (struct tag *) bd->bi_boot_params;

	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size (tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next (params);
}

100ask24x0.c (board\100ask24x0):    gd->bd->bi_boot_params = 0x30000100;
```
```
int dram_init (void)
{
    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

    return 0;
}
// 和视频有区别
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0xc0000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x02000000 /* 32 MB */

#define CFG_FLASH_BASE		0x00000000 /* Flash Bank #1 */
```
* `commandline`
```
#ifdef CONFIG_CMDLINE_TAG
	setup_commandline_tag (bd, commandline);
#endif

#ifdef CONFIG_CMDLINE_TAG               // 在 Armlinux(lib_arm)
	char *commandline = getenv ("bootargs");
#endif

* bootargs=noinitrd root=/dev/mtdblock3 init=/linuxrc console=ttySAC0 rootfstype=jffs2
root=/dev/mtdblock3 // 根文件系统 相当于 windows的 C 盘  这里位于第四个分区
init=/linuxrc       // 第一个应用程序
console=ttySAC0     // console : 内核的打印信息从哪里打出来  ttySAC0 : 串口0
```