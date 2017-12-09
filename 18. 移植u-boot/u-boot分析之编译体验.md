## u-boot分析之编译体验 (2017.12.08)
* S3C2410/S3C2440的CPU为ARM920T
* SOC : system on chip
* u-boot : Universal Boot Loader 通用Bootloader
## u-boot的目录结构说明
![u-boot的目录结构说明](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E7%9A%84%E7%9B%AE%E5%BD%95%E7%BB%93%E6%9E%84%E8%AF%B4%E6%98%8E.JPG)
## U-Boot顶层目录的层次结构
![U-Boot顶层目录的层次结构](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/U-Boot%E9%A1%B6%E5%B1%82%E7%9B%AE%E5%BD%95%E7%9A%84%E5%B1%82%E6%AC%A1%E7%BB%93%E6%9E%84.JPG)
## bootloader与内核的交互方式
![bootloader与内核的交互方式](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/bootloader%E4%B8%8E%E5%86%85%E6%A0%B8%E7%9A%84%E4%BA%A4%E4%BA%92%E6%96%B9%E5%BC%8F.jpg)

## windows和linux系统启动过程的对比
![windows和linux系统启动过程的对比](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/windows%E5%92%8Clinux%E7%B3%BB%E7%BB%9F%E5%90%AF%E5%8A%A8%E8%BF%87%E7%A8%8B%E7%9A%84%E5%AF%B9%E6%AF%94.JPG)
1. `打补丁文件解释(.patch)`
```
diff -urN u-boot-1.1.6/common/cmd_load.c u-boot-1.1.6_jz2440/common/cmd_load.c
--- u-boot-1.1.6/common/cmd_load.c	2006-11-02 22:15:01.000000000 +0800
+++ u-boot-1.1.6_jz2440/common/cmd_load.c	2010-11-26 12:54:38.142063808 +0800
@@ -34,6 +34,8 @@           //原来 ：从34行开始到之后的6行 --- 现在 ：从34行开始到之后的8行
 DECLARE_GLOBAL_DATA_PTR;                           // 34--old  // 34--new
                                                    // 35--old  // 35--new
 #if (CONFIG_COMMANDS & CFG_CMD_LOADB)              // 36--old  // 36--new
+/* support xmodem, www.100ask.net */                           // 37--new
+static ulong load_serial_xmodem (ulong offset);                // 38--new
 static ulong load_serial_ymodem (ulong offset);    // 37--old  // 39--new
 #endif                                             // 38--old  // 41--new
                                                    // 40--old  // 42--new
@@ -53,355 +55,355 @@
```
## u-boot编译过程
![u-boot编译过程](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E7%BC%96%E8%AF%91%E8%BF%87%E7%A8%8B.JPG)
## u-boot 为 bootloader的其中一种，其终极目的是启动内核
## 成功启动内核条件1
![u-boot要成功启动内核需要实现的功能列表](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E8%A6%81%E6%88%90%E5%8A%9F%E5%90%AF%E5%8A%A8%E5%86%85%E6%A0%B8%E9%9C%80%E8%A6%81%E5%AE%9E%E7%8E%B0%E7%9A%84%E5%8A%9F%E8%83%BD%E5%88%97%E8%A1%A8.JPG)
## 成功启动内核条件2
![u-boot要成功启动内核需要实现的功能列表](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/u-boot%E8%A6%81%E6%88%90%E5%8A%9F%E5%90%AF%E5%8A%A8%E5%86%85%E6%A0%B8%E9%9C%80%E8%A6%81%E5%AE%9E%E7%8E%B0%E7%9A%84%E5%8A%9F%E8%83%BD%E5%88%97%E8%A1%A82.JPG)

##  `分析u-boot的目录结构情况以及怎样链接的最快捷的方法是---分析 Makefile`
1. 以 100ask24x0_config为例说明
*  make 100ask24x0_config : 配置
2. 打开 Makefile , 搜索 100ask24x0_config，得到以下情况
```
100ask24x0_config	:	unconfig
	@$(MKCONFIG) $(@:_config=) arm arm920t 100ask24x0 NULL s3c24x0
```
3. 搜索 MKCONFIG，得到
* MKCONFIG	:= $(SRCTREE)/mkconfig    
//SRCTREE ： source tree : 源文件目录下/源码树下
4. 查看一下
```
galen@HD66:/work/system/u-boot-1.1.6$ ls mkconfig 
mkconfig        //确实存在该文件
```
5. $(@:_config=)
* @ --- 表示 100ask24x0_config
* _config=  //为空 , 表示把 100ask24x0_config 中的 _config 去掉
* $(@:_config=) 等价于 100ask24x0
6. @$(MKCONFIG) $(@:_config=) arm arm920t 100ask24x0 NULL s3c24x0 等价于
* mkconfig  100ask24x0 arm arm920t 100ask24x0 NULL s3c24x0
7. `mkconfig源码分析`---`配置过程`
```
APPEND=no	# Default: Create new config file
BOARD_NAME=""	# Name to print in make output

while [ $# -gt 0 ] ; do             // $# : 表示传入参数的个数 -gt : 大于  ---   参数个数大于0就执行
	case "$1" in                    // $1 : 表示第二个参数
	--) shift ; break ;;
	-a) shift ; APPEND=yes ;;
	-n) shift ; BOARD_NAME="${1%%_config}" ; shift ;;
	*)  break ;;
	esac
done

[ "${BOARD_NAME}" ] || BOARD_NAME="$1"      // 如果 BOARD_NAME 定义了，就不执行  BOARD_NAME="$1"
                                            // 如果 BOARD_NAME 没定义(或为空)，就执行  BOARD_NAME="$1
                                            // 该句等于 ：BOARD_NAME=100ask24x0
[ $# -lt 4 ] && exit 1                      // 小于4会退出
[ $# -gt 6 ] && exit 1                      // 大于6会退出

echo "Configuring for ${BOARD_NAME} board..." //程序能执行到此处，就会打印该信息到标准输出里(tty)

#
# Create link to architecture specific headers
#
if [ "$SRCTREE" != "$OBJTREE" ] ; then   // Makefile中查找到 SRCTREE		:= $(CURDIR)
// OBJTREE		:= $(if $(BUILD_DIR),$(BUILD_DIR),$(CURDIR)) 如果定义了 BUILD_DIR，则 OBJTREE=$(BUILD_DIR)
// 否则 OBJTREE=$(CURDIR) 
// Makefile中查看  BUILD_DIR := $(O)   没定义
	mkdir -p ${OBJTREE}/include
	mkdir -p ${OBJTREE}/include2
	cd ${OBJTREE}/include2
	rm -f asm
	ln -s ${SRCTREE}/include/asm-$2 asm
	LNPREFIX="../../include2/asm/"
	cd ../include
	rm -rf asm-$2
	rm -f asm
	mkdir asm-$2
	ln -s asm-$2 asm
else
	cd ./include
	rm -f asm
	ln -s asm-$2 asm      // 软链接  实际为：ln -s asm-arm asm  即asm指向asm-arm 
                          // galen@HD66:/work/system/u-boot-1.1.6/include$ ls -l asm
// lrwxrwxrwx 1 galen galen 7 Dec  8 05:40 asm -> asm-arm 这样做为了方便生成文件 #include <asm/type.h>代替
// 临时生成的各种架构的头文件
//galen@HD66:/work/system/u-boot-1.1.6/include$ ls asm
//arch          arch-arm925t     arch-imx     arch-omap     arch-s3c44b0  bitops.h     global_data.h  


fi

rm -f asm-$2/arch       // rm -f asm-arm/arch

if [ -z "$6" -o "$6" = "NULL" ] ; then     // -z : 字符串为0则为真 -o : 或者  第七个参数为空或者为NULL的话就执行 then 的操作
	ln -s ${LNPREFIX}arch-$3 asm-$2/arch   
else
	ln -s ${LNPREFIX}arch-$6 asm-$2/arch  // LNPREFIX 没定义，为空 源码为：ln -s arch-s3c24x0 asm-arm/arch
// galen@HD66:/work/system/u-boot-1.1.6/include$ ls -l asm-arm/arch
// lrwxrwxrwx 1 galen galen 12 Dec  8 05:40 asm-arm/arch -> arch-s3c24x0
fi

if [ "$2" = "arm" ] ; then
	rm -f asm-$2/proc
	ln -s ${LNPREFIX}proc-armv asm-$2/proc		//建立链接文件 ln -s proc-armv asm-arm/proc
fi

#
# Create include file for Make 				    //生成一个配置文件
#
echo "ARCH   = $2" >  config.mk				    // 新建(或者是覆盖一个已有的config.md文件)一个文件 config.mk
echo "CPU    = $3" >> config.mk					// 追加 CPU    = $3 到 config.mk文件中
echo "BOARD  = $4" >> config.mk					// 追加 BOARD  = $4 到 config.mk文件中

/*
config.mk 里面的内容为：
ARCH   = arm 
CPU    = arm920t
BOARD  = 100ask24x0
*/

[ "$5" ] && [ "$5" != "NULL" ] && echo "VENDOR = $5" >> config.mk  // 如果第6个参数存在，并且该参数不等于																	   // NULL,再追加内容 VENDOR =NULL

[ "$6" ] && [ "$6" != "NULL" ] && echo "SOC    = $6" >> config.mk  // 如果第6个参数存在，并且该参数不等于
																   // NULL,再追加内容 SOC =s3c24x0
/* 
galen@HD66:/work/system/u-boot-1.1.6/include$ cat config.mk 
ARCH   = arm
CPU    = arm920t
BOARD  = 100ask24x0
SOC    = s3c24x0
*/
#
# Create board specific header file				// 创建单板相关的头文件
#
if [ "$APPEND" = "yes" ]	# Append to existing config file   //  Makefile中查看 APPEND=no
then
	echo >> config.h
else
	> config.h		# Create new config file		// > : 表示新建文件
fi
echo "/* Automatically generated - do not edit */" >>config.h
echo "#include <configs/$1.h>" >>config.h			//  #include <configs/100ask24x0.h>

/*
galen@HD66:/work/system/u-boot-1.1.6/include$ cat config.h
/* Automatically generated - do not edit */
#include <configs/100ask24x0.h>

*/

exit 0
```
## $0 $1 $2 ...
```
 mkconfig  100ask24x0 arm arm920t 100ask24x0 NULL s3c24x0
 $0             $1     $2    $3        $4     $5     $6
```
## 总结
* 配置命令 make 100ask24x0_config 实际的作用是 ./mkconfig  100ask24x0 arm arm920t 100ask24x0 NULL s3c24x0
8. `分析编译过程 --- make`
```
#########################################################################
# U-Boot objects....order is important (i.e. start must be first)

OBJS  = cpu/$(CPU)/start.o    // echo "CPU    = $3" >> config.mk    $3 : arm920t
ifeq ($(CPU),i386)
OBJS += cpu/$(CPU)/start16.o
OBJS += cpu/$(CPU)/reset.o
endif
```

```
LIBS  = lib_generic/libgeneric.a
LIBS += board/$(BOARDDIR)/lib$(BOARD).a  // $(BOARDDIR) = 100ask24x0 
										 // 所以这里为：LIBS += board/100ask24x0/lib100ask24x0.a
LIBS += cpu/$(CPU)/lib$(CPU).a			 // LIBS += cpu/arm920t/libarm920t.a

LIBS += net/libnet.a			// LIBS的作用 ： 把net目录下的所有文件编译好了之后打包成libnet.a这样的库文件
```
* make  // 如果我们不指定make的目标的话，它默认是生成Makefile中的第一个目标

```
ALL = $(obj)u-boot.srec $(obj)u-boot.bin $(obj)System.map $(U_BOOT_NAND)

all:		$(ALL)						// all命令依赖于 $(ALL)

$(obj)u-boot.hex:	$(obj)u-boot
		$(OBJCOPY) ${OBJCFLAGS} -O ihex $< $@

$(obj)u-boot.srec:	$(obj)u-boot		
		$(OBJCOPY) ${OBJCFLAGS} -O srec $< $@

$(obj)u-boot.bin:	$(obj)u-boot		// u-boot.bin(二进制可执行文件) 依赖于 u-boot(elf可执行文件)
		$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(obj)u-boot.img:	$(obj)u-boot.bin
		./tools/mkimage -A $(ARCH) -T firmware -C none \
		-a $(TEXT_BASE) -e 0 \
		-n $(shell sed -n -e 's/.*U_BOOT_VERSION//p' $(VERSION_FILE) | \
			sed -e 's/"[	 ]*$$/ for $(BOARD) board"/') \
		-d $< $@

$(obj)u-boot.dis:	$(obj)u-boot
		$(OBJDUMP) -d $< > $@

// (obj)u-boot的生成这部分可以通过执行make来查看具体的链接内容

/*
UNDEF_SYM=`arm-linux-objdump -x lib_generic/libgeneric.a board/100ask24x0/lib100ask24x0.a cpu/arm920t/libarm920t.a cpu/arm920t/s3c24x0/libs3c24x0.a lib_arm/libarm.a fs/cramfs/libcramfs.a fs/fat/libfat.a fs/fdos/libfdos.a fs/jffs2/libjffs2.a fs/reiserfs/libreiserfs.a fs/ext2/libext2fs.a net/libnet.a disk/libdisk.a rtc/librtc.a dtt/libdtt.a drivers/libdrivers.a drivers/nand/libnand.a drivers/nand_legacy/libnand_legacy.a drivers/usb/libusb.a drivers/sk98lin/libsk98lin.a common/libcommon.a |sed  -n -e 's/.*\(__u_boot_cmd_.*\)/-u\1/p'|sort|uniq`;\
//进入目录
cd /work/system/u-boot-1.1.6 &&
//链接脚本 ： u-boot.lds
arm-linux-ld -Bstatic -T /work/system/u-boot-1.1.6/board/100ask24x0/u-boot.lds -Ttext 0x33F80000  

$UNDEF_SYM cpu/arm920t/start.o \
			--start-group 
// 原材料
			lib_generic/libgeneric.a board/100ask24x0/lib100ask24x0.a cpu/arm920t/libarm920t.a cpu/arm920t/s3c24x0/libs3c24x0.a lib_arm/libarm.a fs/cramfs/libcramfs.a fs/fat/libfat.a fs/fdos/libfdos.a fs/jffs2/libjffs2.a fs/reiserfs/libreiserfs.a fs/ext2/libext2fs.a net/libnet.a disk/libdisk.a rtc/librtc.a dtt/libdtt.a drivers/libdrivers.a drivers/nand/libnand.a drivers/nand_legacy/libnand_legacy.a drivers/usb/libusb.a drivers/sk98lin/libsk98lin.a common/libcommon.a --end-group -L /work/tools/gcc-3.4.5-glibc-2.3.6/lib/gcc/arm-linux/3.4.5 -lgcc \
			-Map u-boot.map -o u-boot

*/

$(obj)u-boot:		depend version $(SUBDIRS) $(OBJS) $(LIBS) $(LDSCRIPT)  // SUBDIRS	= tools \
		UNDEF_SYM=`$(OBJDUMP) -x $(LIBS) |sed  -n -e 's/.*\(__u_boot_cmd_.*\)/-u\1/p'|sort|uniq`;\
		cd $(LNDIR) && $(LD) $(LDFLAGS) $$UNDEF_SYM $(__OBJS) \  
// LNDIR : 链接目录 LD ：链接是否存在 LDFLAGS ：链接参数  __OBJS ： 所有的 .o文件
			--start-group $(__LIBS) --end-group $(PLATFORM_LIBS) \	// __LIBS :所有的库
			-Map u-boot.map -o u-boot

$(OBJS):
	echo $(OBJS)	
		$(MAKE) -C cpu/$(CPU) $(if $(REMOTE_BUILD),$@,$(notdir $@))

$(LIBS):
		$(MAKE) -C $(dir $(subst $(obj),,$@)
```
## 链接脚本说明
```
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
/*OUTPUT_FORMAT("elf32-arm", "elf32-arm", "elf32-arm")*/
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;								// 当前地址为 0x00000000
// 因为链接命令中
// arm-linux-ld -Bstatic -T /work/system/u-boot-1.1.6/board/100ask24x0/u-boot.lds -Ttext 0x33F80000 
// 所以 后面的代码的地址从  0x00000000 + 0x33F80000 = 0x33F80000开始排放

	. = ALIGN(4);								// 表示起始运行地址为4字节对齐
	.text      :								// 定义了一个名为 “.text” 的代码段,内容为 { }的内容
	{
	  cpu/arm920t/start.o	(.text)				// 先排放 cpu/arm920t/start.o 的 代码段 (.text)
          board/100ask24x0/boot_init.o (.text)  // 后排放 board/100ask24x0/boot_init.o 的 代码段 (.text)
	  *(.text)									// 其他文件的所有代码段
	}

	. = ALIGN(4);								// 表示起始运行地址为4字节对齐
	.rodata : { *(.rodata) }					// 定义了一个名为 “.rodata”的段,内容为所有文件的只读数据段

	. = ALIGN(4);								// 表示起始运行地址为4字节对齐
	.data : { *(.data) }						// 定义了一个名为 “.data”的段,内容为所有文件的数据段

	. = ALIGN(4);
	.got : { *(.got) }

	. = .;
	__u_boot_cmd_start = .;
	.u_boot_cmd : { *(.u_boot_cmd) }
	__u_boot_cmd_end = .;

	. = ALIGN(4);
	__bss_start = .;
	.bss : { *(.bss) }
	_end = .;
}
```

```
galen@HD66:/work/system/u-boot-1.1.6$ grep "33F80000" *  -nR
board/smdk2410/config.mk:25:TEXT_BASE = 0x33F80000
board/100ask24x0/config.mk:25:TEXT_BASE = 0x33F80000
board/mpl/vcma9/config.mk:24:TEXT_BASE = 0x33F80000
board/sbc2410x/config.mk:23:TEXT_BASE = 0x33F80000
u-boot.srec:2:S31533F80000170000EA14F09FE514F09FE514F09FE526
u-boot.srec:12195:S31533FAF9F0E4F9FA33F8000000626F6F746172677371
u-boot.srec:12400:S70533F80000CF
```
```
config.mk:189:LDFLAGS += -Bstatic -T $(LDSCRIPT) -Ttext $(TEXT_BASE) $(PLATFORM_LDFLAGS) //定义了LDFLAGS
//这句跟 
//arm-linux-ld -Bstatic -T /work/system/u-boot-1.1.6/board/100ask24x0/u-boot.lds -Ttext 0x33F80000 
// 吻合起来
board/100ask24x0/config.mk:25:TEXT_BASE = 0x33F80000  // TEXT_BASE 的定义位置
```
## Makefile分析步骤
![Makefile分析步骤](https://github.com/GalenDeng/Embedded-Linux/blob/master/18.%20%E7%A7%BB%E6%A4%8Du-boot/Makefile%E5%88%86%E6%9E%90%E6%AD%A5%E9%AA%A4.JPG)