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
* 所以说分区的名字(kernel)不重要，重要的是它代表的起始地址和结束地址
