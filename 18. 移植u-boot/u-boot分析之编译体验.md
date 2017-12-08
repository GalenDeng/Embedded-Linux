## u-boot分析之编译体验 (2017.12.08)
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
7. `mkconfig源码分析`
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
	ln -s ${LNPREFIX}arch-$6 asm-$2/arch
fi

if [ "$2" = "arm" ] ; then
	rm -f asm-$2/proc
	ln -s ${LNPREFIX}proc-armv asm-$2/proc
fi

#
# Create include file for Make
#
echo "ARCH   = $2" >  config.mk
echo "CPU    = $3" >> config.mk
echo "BOARD  = $4" >> config.mk

[ "$5" ] && [ "$5" != "NULL" ] && echo "VENDOR = $5" >> config.mk

[ "$6" ] && [ "$6" != "NULL" ] && echo "SOC    = $6" >> config.mk

#
# Create board specific header file
#
if [ "$APPEND" = "yes" ]	# Append to existing config file
then
	echo >> config.h
else
	> config.h		# Create new config file
fi
echo "/* Automatically generated - do not edit */" >>config.h
echo "#include <configs/$1.h>" >>config.h

exit 0
```
## $0 $1 $2 ...
```
 mkconfig  100ask24x0 arm arm920t 100ask24x0 NULL s3c24x0
 $0             $1     $2    $3        $4     $5     $6
```