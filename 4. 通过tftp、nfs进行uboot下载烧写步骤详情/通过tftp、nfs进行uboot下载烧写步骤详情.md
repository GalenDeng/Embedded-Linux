## 通过tftp进行uboot的下载步骤详情 (2017.12.6)
1. `下载tftpboot软件，并安装`
2. 打开`tftpd32软件`，`选择要下载的文件的地址`,选择`本电脑的IP地址`
* ![tftp设置](https://github.com/GalenDeng/Embedded-Linux/blob/master/4.%E9%80%9A%E8%BF%87tftpboot%E4%B8%8B%E8%BD%BDuboot%E7%AD%89bin%E6%96%87%E4%BB%B6/tftpboot%E8%AE%BE%E7%BD%AE.JPG)
3. NOR flash启动单板,按空格进行u-boot的菜单栏，按 q 退出 menu
   进入到 openjtag 中, 设置 ipaddr serverip ,其中 serverip要和电脑的IP相同，而 ipaddr可以随便设置，但要保证其和serverip在同一个网段下,最后 `save`
* ![ip设置](https://github.com/GalenDeng/Embedded-Linux/blob/master/4.%E9%80%9A%E8%BF%87tftpboot%E4%B8%8B%E8%BD%BDuboot%E7%AD%89bin%E6%96%87%E4%BB%B6/%E8%AE%BE%E7%BD%AEIP%E5%B9%B6%E8%BF%9B%E8%A1%8C%E7%A8%8B%E5%BA%8F%E7%9A%84%E4%B8%8B%E8%BD%BD%E6%93%8D%E4%BD%9C.JPG)
* openjtag > print  //print可查看网络的设置情况
* ![查看网络设置](https://github.com/GalenDeng/Embedded-Linux/blob/master/4.%E9%80%9A%E8%BF%87tftpboot%E4%B8%8B%E8%BD%BDuboot%E7%AD%89bin%E6%96%87%E4%BB%B6/u-boot%E7%9A%84%E8%8F%9C%E5%8D%95%E6%A0%8F%E6%93%8D%E4%BD%9C1.JPG)
4. 在u-boot菜单栏中 ping 一下 电脑的 IP 看是否 ping 通
5. `下载` 
* tftp 30000000 lcd.bin     // 30000000：为烧写地址 lcd.bin 为 二进制程序 这里不用指定目录,因为tftpd32已经指定了目录的地址
6. `显示分区`
```
OpenJTAG> mtdpart

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

## tftp烧写内核步骤
1. 打开tftpd32软件，设置本地IP、要下载的文件目录地址,准备进行网络下载
2. 进入到openJTAG > 中, 确保 tftp服务已打开 , 设置 ipaddr serverip , save
3. tftp 30000000 uImage  //把取到的uImage下载到开发板的0x30000000中
4. mtdpart查看分区信息
5. nand erase kernel //擦除kernel分区
6. nand wirte.jffs2 30000000 kernel   //之前我们下载的文件放在 0x30000000这个位置了，现在把它烧写在kernel里面 

## tftp烧写fs_qtopia(文件系统)步骤
1. tftp 30000000 fs_qtopia.yaffs2
2. nand erase root
3. nand write.yaffs 30000000 00260000 $(filesize)  
 //$(filesize) : 表示下载的文件的大小，我们可以直接查看下载的文件有多大，写具体的size上去

## NFS烧写内核步骤
1. nfs 30000000 192.168.99.140:/work/nfs_root/uImage
// 这里的nfs_root必须是被支持挂载的，可以在 /etc/export查看，如：
![被支持的挂载文件目录]()
*  /work/nfs_root *(rw,sync,no_root_squash)  // 在 /etc/exports 添加 然后重启 sudo /etc/init.d/nfs-kernel-server restart即可
2. nand erase kernel
3. nand write.yaffs2 30000000 kernel 

## NFS烧写文件系统步骤
1. nfs 30000000 192.168.99.140:/work/nfs_root/fs_qtopia.yaffs2
2. nand erase root
3. nand write.yaffs 30000000  260000 2f76b40        //  2f76b40 为下载的文件大小
