## 移植linux内核 (2017.12.11)
## 内核移植编译步骤
![内核移植编译步骤](https://github.com/GalenDeng/Embedded-Linux/blob/master/19.%20%E7%A7%BB%E6%A4%8Dlinux%E5%86%85%E6%A0%B8/linux%E5%86%85%E6%A0%B8%E7%A7%BB%E6%A4%8D%E7%BC%96%E8%AF%91%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/%E5%86%85%E6%A0%B8%E7%A7%BB%E6%A4%8D%E7%BC%96%E8%AF%91%E6%AD%A5%E9%AA%A4.JPG)
## 使用默认配置，在上面修改内容，实现linux的移植的具体方式
1. `find命令`
* galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ find -name "*defconfig" 
2. 
```
galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ cd ./arch/arm/configs/
galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6/arch/arm/configs$ ls neponset_defconfig      
at91sam9263ek_defconfig  csb637_defconfig      integrator_defconfig  ks8695_defconfig      netwinder_defconfig     realview-smp_defconfig
at91sam9rlek_defconfig   ebsa110_defconfig     iop13xx_defconfig     lart_defconfig        netx_defconfig          rpc_defconfig
ateb9200_defconfig       edb7211_defconfig     iop32x_defconfig      lpd270_defconfig      ns9xxx_defconfig        s3c2410_defconfig
badge4_defconfig         ep93xx_defconfig      iop33x_defconfig      lpd7a400_defconfig    omap_h2_1610_defconfig  shannon_defconfig
carmeva_defconfig        footbridge_defconfig  ixp2000_defconfig     lpd7a404_defconfig    onearm_defconfig        shark_defconfig
```
3. `默认配置 -- make make s3c2410_defconfig 实现把 配置保存到 .config中`
* galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ make s3c2410_defconfig

4. `make menuconfig修改菜单项 -- 读取 .config`
* galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ make menuconfig 
```
提示信息：
#
# configuration written to .config
#
```
## 使用厂家提供的配置文件来实现linux的移植的具体方法
* cp config_厂家 .comnfig
* make menuconfig
* `config_ok 为 JZ2440提供的厂家的.config`
```
galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ ls config_ok 
config_ok
```
* galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ cp config_ok  .config
* galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ make uImage     //编译 uImage = header + 真正的内核 (厂家做好的Image在 /bin/uImage)
```
  UIMAGE  arch/arm/boot/uImage          // 生成的uImage的路径位置
Image Name:   Linux-2.6.22.6            // 名字
Created:      Sun Dec 10 20:14:49 2017
Image Type:   ARM Linux Kernel Image (uncompressed)     // 类型
Data Size:    1848696 Bytes = 1805.37 kB = 1.76 MB
Load Address: 0x30008000                                // 真正的内核地址
Entry Point:  0x30008000                                // 入口地址
  Image arch/arm/boot/uImage is ready
```
* 单板进入菜单界面，按 K
* linux 下 sudo dnw ./arch/arm/boot/uImage
* 烧写成功
* 用sourceinsight 查看 cmd.menu(common)源码
```
            case 'k':
            {
                strcpy(cmd_buf, "usbslave 1 0x30000000; nand erase kernel; nand write.jffs2 0x30000000 kernel $(filesize)");
                run_command(cmd_buf, 0);
                break;
            }

* usbslave 1 0x3000000  // usb命令 把下载的文件放在  0x3000000 这个地址上
* nand erase kernel     // 擦除kernel
* nand write.jffs2 0x30000000 kernel $(filesize)")  //把 0x30000000的内容写在kernel上 
* $(filesize)           // 这个一个宏 ，代表 usb 下载的文件的大小  
```
* u-boot菜单界面， 输入 b 启动

## 各个符号的意义
* <M> ：作为模块(设备驱动程序)
* [*] : 编译进内核
* [ ] : 不编译进内核
* <?> : 会弹出帮助信息
* ![各个符号的意义](https://github.com/GalenDeng/Embedded-Linux/blob/master/19.%20%E7%A7%BB%E6%A4%8Dlinux%E5%86%85%E6%A0%B8/linux%E5%86%85%E6%A0%B8%E7%A7%BB%E6%A4%8D%E7%BC%96%E8%AF%91%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/%E5%90%84%E4%B8%AA%E7%AC%A6%E5%8F%B7%E7%9A%84%E6%84%8F%E4%B9%89.JPG)

## 删除开发板的文件系统的方法
* 进入到 U-BOOT 的菜单界面 , 输入 q , 进入到 控制界面
* nand erase root   // root 就是 文件系统
* OpenJTAG> boot    // 启动内核
* 这时因为没有文件系统，所以起不来
```
UDA1341 audio driver initialized
ALSA device list:
  No soundcards found.
TCP cubic registered
NET: Registered protocol family 1
drivers/rtc/hctosys.c: unable to open rtc device (rtc0)
VFS: Mounted root (jffs2 filesystem).
Freeing init memory: 140K
Warning: unable to open an initial console.
Failed to execute /linuxrc.  Attempting defaults...
Kernel panic - not syncing: No init found.  Try passing init= option to kernel.
```
## 内核配置的具体过程
![内核配置的具体过程](https://github.com/GalenDeng/Embedded-Linux/blob/master/19.%20%E7%A7%BB%E6%A4%8Dlinux%E5%86%85%E6%A0%B8/linux%E5%86%85%E6%A0%B8%E7%A7%BB%E6%A4%8D%E7%BC%96%E8%AF%91%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/%E5%86%85%E6%A0%B8%E9%85%8D%E7%BD%AE%E7%9A%84%E5%85%B7%E4%BD%93%E8%BF%87%E7%A8%8B.JPG)
## .config 分析 -- 以 DM9000 为例
```
CONFIG_DM9000=y // 为 y 表示会编译进内核，支持DM9000,会动态加载
```
## 搜素出 含有  CONFIG_DM9000 的 文件 ，进行相关操作
* galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ grep "CONFIG_DM9000"  * -nwR
* -w : w就是匹配整个单词而不是字符串
```
[oracle@s12071 ~]$ echo -e "hello\n hell" 
hello
hell
[oracle@s12071 ~]$ echo -e "hello\n hell" | grep -w "hell"
hell
[oracle@s12071 ~]$ echo -e "hello\n hell" | grep "hell"
hello
hell
```
```
* 类一 ： C源码 --- 这里的 CONFIG_DM9000 肯定是 宏 ，而宏肯定在 .c 或 .h 文件里面定义的 所以在
          include/linux/autoconf.h中定义
arch/arm/plat-s3c24xx/common-smdk.c:46:#if defined(CONFIG_DM9000) || defined(CONFIG_DM9000_MODULE)
arch/arm/plat-s3c24xx/common-smdk.c:162:#if defined(CONFIG_DM9000) || defined(CONFIG_DM9000_MODULE)
arch/arm/plat-s3c24xx/common-smdk.c:200:#endif /* CONFIG_DM9000 */
arch/arm/plat-s3c24xx/common-smdk.c:250:#if defined(CONFIG_DM9000) || defined(CONFIG_DM9000_MODULE)
* 类二 ：子目录的Makefile 
drivers/net/Makefile:197:obj-$(CONFIG_DM9000) += dm9dev9000c.o
drivers/net/Makefile:198:#obj-$(CONFIG_DM9000) += dm9000.o
drivers/net/Makefile:199:#obj-$(CONFIG_DM9000) += dm9ks.o
* 类三 ： include下
include/config/auto.conf:144:CONFIG_DM9000=y
include/linux/autoconf.h:145:#define CONFIG_DM9000 1
// autoconf.h 从字面上的意义来看应该是自动生成的，那么它的来源应该是 .config

* 注意 arch/arm/configs/ 这样开头的不用管，都是默认的
```
* galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ vi include/linux/autoconf.h 
* #define CONFIG_DM9000 1

## `CONFIG_DM9000=y or CONFIG_DM9000=m 的差异体现在 Makefile 中`
```
CONFIG_DM9000=y
CONFIG_DM9000=m
* galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ vi drivers/net/Makefile 
* #obj-$(CONFIG_DM9000) += dm9000.o // CONFIG_DM9000为y,dm9000.c被编译
进内核,CONFIG_DM9000为m,dm9000.c被编译成可加载的模块 dm9000的定义在 include/config/auto.conf
不过最终也是来自于 .config
* galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ vi include/config/auto.conf
* CONFIG_DM9000=y

在内核子目录的Makefile中 
obj-y += xxx.o // 表示xxx.c or xxx.S最终会编译进内核里面
obj-m += xxx.o // xxx.c or xxx.S => xxx.ko   xxx.c最终被编译成可加载的模块(xxx.ko)[驱动程序]
```
## 所以 make uImage 实现了以下的功能
1. `配置生成 .config`
2. `.config  生成 include/linux/autoconf.h 供源代码使用`
3. `.config  生成 include/linux/autoconf.h 被顶层的Makefile包含，供子目录的Makefile使用`
## 分析 Makefile
* 通过第一个文件 , 链接脚本 -- 顺藤摸瓜
* linux中一个Makefile文件只负责生成当前目录下的目标文件，子目录下的目标文件由子目录的Makefile生成
## 多个源文件的模块(module)，除在obj-m中增加一个.o文件外，还需要
## 定义一个<module name>-objs变量来告诉Makefile这个.o文件由哪些文件组成
```
#driver/isdn/i41/Makefile
# Each configuration option enables a list of files.
obj-$(CONFIG_ISDN_I4L)          += isdn.o
obj-$(CONFIG_ISDN_PPP_BSDCOMP)  += isdn_bsdcomp.o
# Multipart objects.
isdn-objs                         := isdn_net.o isdn_tty.o isdn_v110.o isdn_common.o
```
* fs/Makefile
```
obj-$(CONFIG_JFFS2_FS)    += jffs2/    // 当 CONFIG_JFFS2_FS被定义为y或m时，在编译时将会进入jffs2/目录进行编译
```
## linux 内核版本查看
* /linux-2.6.22.6$ vim Makefile  // 2.6.22.6
```
VERSION = 2
PATCHLEVEL = 6    // 以上我主版本
SUBLEVEL = 22
EXTRAVERSION = .6 // 后两个为次版本
```
* EXTRAVERSION = -rc6 // 表示测试版本
## 打补丁注意事项
```
补丁文件 patch-2.6.xx.1 patch-2.6.xx.2  patch-2.6.xx.3,他们都是基于内核2.6xx生成的补丁文件，
使用时可以在内核2.6xx上直接打补丁 patch-2.6.xx.3,并不需要先打上补丁patch-2.6.xx.1、patch-2.6.xx.2,
相应的,在打补丁 patch-2.6.xx.3前，要先去除patch-2.6.xx.2
```
## kernel官网各标记符意义
![kernel官网各标记符意义](https://github.com/GalenDeng/Embedded-Linux/blob/master/19.%20%E7%A7%BB%E6%A4%8Dlinux%E5%86%85%E6%A0%B8/linux%E5%86%85%E6%A0%B8%E7%A7%BB%E6%A4%8D%E7%BC%96%E8%AF%91%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/kernel%E5%AE%98%E7%BD%91%E5%90%84%E6%A0%87%E8%AE%B0%E7%AC%A6%E6%84%8F%E4%B9%89.JPG)
## linux内核子目录结构
![linux内核子目录结构](https://github.com/GalenDeng/Embedded-Linux/blob/master/19.%20%E7%A7%BB%E6%A4%8Dlinux%E5%86%85%E6%A0%B8/linux%E5%86%85%E6%A0%B8%E7%A7%BB%E6%A4%8D%E7%BC%96%E8%AF%91%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/linux%E5%86%85%E6%A0%B8%E5%AD%90%E7%9B%AE%E5%BD%95%E7%BB%93%E6%9E%84.JPG)
## linux内核源码层次结构
![linux内核源码层次结构](https://github.com/GalenDeng/Embedded-Linux/blob/master/19.%20%E7%A7%BB%E6%A4%8Dlinux%E5%86%85%E6%A0%B8/linux%E5%86%85%E6%A0%B8%E7%A7%BB%E6%A4%8D%E7%BC%96%E8%AF%91%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/linux%E5%86%85%E6%A0%B8%E6%BA%90%E7%A0%81%E5%B1%82%E6%AC%A1%E7%BB%93%E6%9E%84.JPG)
## 内核的Makefile的作用、用法讲解文件
* /linux-2.6.22.6$ cat ./Documentation/kbuild/makefiles.txt 
## linux内核Makefile文件分类
![linux内核Makefile文件分类](https://github.com/GalenDeng/Embedded-Linux/blob/master/19.%20%E7%A7%BB%E6%A4%8Dlinux%E5%86%85%E6%A0%B8/linux%E5%86%85%E6%A0%B8%E7%A7%BB%E6%A4%8D%E7%BC%96%E8%AF%91%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/linux%E5%86%85%E6%A0%B8Makefile%E6%96%87%E4%BB%B6%E5%88%86%E7%B1%BB.JPG)
## 怎样编译文件
![怎样编译文件](https://github.com/GalenDeng/Embedded-Linux/blob/master/19.%20%E7%A7%BB%E6%A4%8Dlinux%E5%86%85%E6%A0%B8/linux%E5%86%85%E6%A0%B8%E7%A7%BB%E6%A4%8D%E7%BC%96%E8%AF%91%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/%E6%80%8E%E6%A0%B7%E7%BC%96%E8%AF%91%E6%96%87%E4%BB%B6.JPG)
## 怎样连接文件及顺序特点(简介patsubst字符串处理函数的用法)
![怎样连接文件及顺序特点](https://github.com/GalenDeng/Embedded-Linux/blob/master/19.%20%E7%A7%BB%E6%A4%8Dlinux%E5%86%85%E6%A0%B8/linux%E5%86%85%E6%A0%B8%E7%A7%BB%E6%A4%8D%E7%BC%96%E8%AF%91%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/%E6%80%8E%E6%A0%B7%E8%BF%9E%E6%8E%A5%E6%96%87%E4%BB%B6%E5%8F%8A%E9%A1%BA%E5%BA%8F%E7%89%B9%E7%82%B9.JPG)
## linux内核的Makefile总结
![linux内核的Makefile总结](https://github.com/GalenDeng/Embedded-Linux/blob/master/19.%20%E7%A7%BB%E6%A4%8Dlinux%E5%86%85%E6%A0%B8/linux%E5%86%85%E6%A0%B8%E7%A7%BB%E6%A4%8D%E7%BC%96%E8%AF%91%E5%9B%BE%E7%89%87%E7%AC%94%E8%AE%B0/linux%E5%86%85%E6%A0%B8%E7%9A%84Makefile%E6%80%BB%E7%BB%93.JPG)
* vmlinux -- 内核映像文件
* 分析的时候，顶层的Makefile
```
顶层Makefile包括： galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$  vim Makefile
include $(srctree)/arch/$(ARCH)/Makefile
all: vmlinux        // 顶层的第一个目标就是生成一个vmlinux的内核映像文件
vmlinux: $(vmlinux-lds) $(vmlinux-init) $(vmlinux-main) $(kallsyms.o) FORCE
            链接脚本         初始化           核心代码   

vmlinux-init := $(head-y) $(init-y)   //(head-y 在顶层没定义 ，则一定在 arch(架构)里面定义的 
// /arch/arm/Makefile head-y		:= arch/arm/kernel/head$(MMUEXT).o arch/arm/kernel/init_task.o 
// $(MMUEXT) 为空的话 head$(MMUEXT).o  => head.o
// init-y		:= init/ ; 
// init-y		:= $(patsubst %/, %/built-in.o, $(init-y)) 意思是：init-y  =  init/built-in.o

vmlinux-main := $(core-y) $(libs-y) $(drivers-y) $(net-y)
// core-y		:= usr/
// core-y		+= kernel/ mm/ fs/ ipc/ security/ crypto/ block/
// core-y		:= $(patsubst %/, %/built-in.o, $(core-y))  意思是：
// core-y = usr/built-in.o  kernel/built-in.o mm/built-in.o fs/built-in.o ipc/built-in.o security/built-in.o crypto/built-in.o block/built-in.o

// libs-y		:= lib/
// libs-y1		:= $(patsubst %/, %/lib.a, $(libs-y))
// libs-y2		:= $(patsubst %/, %/built-in.o, $(libs-y))
// libs-y		:= $(libs-y1) $(libs-y2)   意思是：
// libs-y = lib/lib.a lib/built.o


// drivers-y	:= drivers/ sound/
// drivers-y	:= $(patsubst %/, %/built-in.o, $(drivers-y)) 意思是：
// drivers-y =   drivers/built-in.o  sound/built-in.o

// net-y		:= net/
// net-y		:= $(patsubst %/, %/built-in.o, $(net-y))  意思是：
// net-y = net/built-in.o

vmlinux-all  := $(vmlinux-init) $(vmlinux-main)
vmlinux-lds  := arch/$(ARCH)/kernel/vmlinux.lds
export KBUILD_VMLINUX_OBJS := $(vmlinux-all)         
```
* make uImage => /arch/arm/Makefile
```
./arch/arm/ Makefile
zImage Image xipImage bootpImage uImage: vmlinux    // uImage依赖于vmlinux
```
## 分析原材料怎么编译的，除了分析Makefile文件，还可以通过编译内核来查看
1. `原材料`
```
* head-y		:= arch/arm/kernel/head$(MMUEXT).o arch/arm/kernel/init_task.o 
* init-y  =  init/built-in.o
* core-y = usr/built-in.o  kernel/built-in.o mm/built-in.o fs/built-in.o ipc/built-in.o security/built-in.o crypto/built-in.o block/built-in.o
* libs-y = lib/lib.a lib/built.o
* drivers-y =   drivers/built-in.o  sound/built-in.o
* net-y = net/built-in.o
```
2. `编译方式-- 法一： 分析顶层Makefile`
* vmlinux: $(vmlinux-lds) $(vmlinux-init) $(vmlinux-main) $(kallsyms.o) FORCE
* `代码太庞大了，我们没有精力去做`
```
# vmlinux image - including updated kernel symbols
vmlinux: $(vmlinux-lds) $(vmlinux-init) $(vmlinux-main) $(kallsyms.o) FORCE
ifdef CONFIG_HEADERS_CHECK
	$(Q)$(MAKE) -f $(srctree)/Makefile headers_check
endif
	$(call if_changed_rule,vmlinux__)
	$(Q)$(MAKE) -f $(srctree)/scripts/Makefile.modpost $@
	$(Q)rm -f .old_version
  ...
```
3. `编译方式-- 法二：通过编译内核来分析`
* `删除内核映像文件 -- vmlinux`
```
* galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ rm vmlinux 
```
* `make`
```
* galen@HD66:/work/linux-2.6-transplant/linux-2.6.22.6$ make uImage V=1
* V=1   // 把命令更加详细的罗列出来
```
* `关注最后的命令`
```
arm-linux-ld -EL  -p --no-undefined -X -o vmlinux   // 链接  输出内核vmlinux

-T arch/arm/kernel/vmlinux.lds     // 链接脚本 决定链接的方式

arch/arm/kernel/head.o arch/arm/kernel/init_task.o 
//  第一个文件 ：arch/arm/kernel/head.S (应该是汇编)
//  链接脚本 :  arch/arm/kernel/vmlinux.lds
init/built-in.o 

--start-group  usr/built-in.o  arch/arm/kernel/built-in.o  arch/arm/mm/built-in.o  arch/arm/common/built-in.o  arch/arm/mach-s3c2410/built-in.o  arch/arm/mach-s3c2400/built-in.o  arch/arm/mach-s3c2412/built-in.o  arch/arm/mach-s3c2440/built-in.o  arch/arm/mach-s3c2442/built-in.o  arch/arm/mach-s3c2443/built-in.o  arch/arm/nwfpe/built-in.o  arch/arm/plat-s3c24xx/built-in.o  kernel/built-in.o  mm/built-in.o  fs/built-in.o  ipc/built-in.o  security/built-in.o  crypto/built-in.o  block/built-in.o  arch/arm/lib/lib.a  lib/lib.a  arch/arm/lib/built-in.o  lib/built-in.o  drivers/built-in.o  sound/built-in.o  net/built-in.o --end-group .tmp_kallsyms2.o
```
* `打开 vmlinux.lds`
```
SECTIONS
{
 . = (0xc0000000) + 0x00008000;       // 虚拟地址

 .text.head : {                       
  _stext = .;
  _sinittext = .;
  *(.text.head)   
//放所有文件的head.o 放的顺序为 vmlinux: $(vmlinux-lds) $(vmlinux-init) $  (vmlinux-main) $(kallsyms.o) FORCE 
// vmlinux-init := $(head-y) $(init-y)
// vmlinux-main := $(core-y) $(libs-y) $(drivers-y) $(net-y)
 }

 .init : { /* Init code and data		*/
......
```