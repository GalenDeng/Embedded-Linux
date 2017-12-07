## 通过nfs进行下载烧写--配置NFS网络文件系统的挂载卸载--制作镜像文件--虚拟机和开发板的交叉编译操作(通过MFS共享目录) (2017.12.07)
1. `确保nfs服务已开启`
2. `进入到u-boot的菜单栏，按 q 进入到 openJTAG的模式下：`
*  nfs 30000000 192.168.99.140:/work/nfs_root/tmp/fs.yaffs2 
*  30000000 : 烧写地址  192.168.99.140 ：文件所在的服务器IP  /work/nfs_root/tmp/fs.yaffs2 : 文件地址位置   // nfs的操作和tftpboot的操作一样
```
OpenJTAG> nfs 30000000 192.168.99.140:/work/nfs_root/tmp/fs.yaffs2
dm9000 i/o: 0x20000000, id: 0x90000a46 
DM9000: running in 16 bit mode
MAC: 08:00:3e:26:0a:5b
could not establish link
File transfer via NFS from server 192.168.99.140; our IP address is 192.168.99.135
Filename '/work/nfs_root/tmp/fs.yaffs2'.
Load address: 0x30000000
Loading: #################################################################
         ###########################################################
done
Bytes transferred = 8952768 (889bc0 hex)
```
3. `擦除`
* nand erase root 
4. `烧写`
* OpenJTAG> nand write.yaffs 30000000 260000 $(filesize)

## 配置通过 nfs(网络文件系统)挂载
1. `仅用flash上的根文件系统启动后，手动MOUNT NFS` --- `挂载共享`
* mount -t nfs -o nolock,vers=2 192.168.99.140:/work/nfs_root   /mnt  
```
result: 
# mount -t nfs -o nolock,vers=2 192.168.99.140:/work/nfs_root   /mnt  
# ls -lt
# cd /mnt/
# ls -lt
drwxrwxr-x    3 1000     1000         4096 Dec  6  2017 tmp
-rw-r--r--    1 1000     1000     16446798 Dec 24  2010 fs_qtopia.tar.bz2
-rw-r--r--    1 1000     1000     27072948 Apr 15  2008 fs_xwindow.tar.bz2
-rw-r--r--    1 1000     1000      2748536 Apr 15  2008 fs_mini.tar.bz2
-rw-r--r--    1 1000     1000      2832504 Apr 15  2008 fs_mini_mdev.tar.bz2
```
//把服务器的文件挂载到本地的 /mnt 目录下
* -t :  指定文件系统的类型
* -o : .-o options 主要用来描述设备或档案的挂接方式。常用的参数有：
　　loop：用来把一个文件当成硬盘分区挂接上系统
　　ro：采用只读方式挂接设备
　　rw：采用读写方式挂接设备
　　iocharset：指定访问文件系统所用字符集

```
nfs mount 默认选项包括文件锁，依赖于portmap提供的动态端口分配功能。
解决方法：kill 文件锁（lockd）或者mount -o nolock
```
2. `卸载`
```
# umount /mnt       // /mnt 为挂载点
# ls -lt /mnt
#
```
3. `nfs的挂载特征`
```
在开发板命令行输入 mount -o nolock -t nfs 192.168.1.200:/home/kmart/test   /mnt/nfs
这样就可以在开发板上访问宿主机192.168.1.200下的test目录啦！
```
4. `相关内核文件路径` 
* // F:\嵌入式\u-boot_and_core\linux-2.6.22.6_jz2440\linux-2.6.22.6\Documentation\nfsroot
* ip=<client-ip>:<server-ip>:<gw-ip>:<netmask>:<hostname>:<device>:<autoconf>
* ip=192.168.99.135:192.168.99.140:192.168.99.4:255.255.255.0::eth0:off
5. `完整配置` --- `配置完后reset一下,会以NFS形式启动`
* set bootargs noinitrd root=/dev/nfs nfsroot=192.168.99.140:/work/nfs_root/tmp/fs_mini_mdev ip=192.168.99.135:192.168.99.140:192.168.99.4:255.255.255.0::eth0:off
init=/linuxrc console=ttySAC0 rootfstype=jffs2

## 制作镜像文件
* mkyaffs2image /work/nfs_root/fs_qtopia /tmp/fs.yaffs2
* /work/nfs_root/fs_qtopia : 目录名   /tmp/fs.yaffs2 ： 生成的镜像文件名
* mkyaffs2image 这个二进制可执行文件要放在 /usr/bin /bin等目录下，shell才能辨别出来

## 编译文件，通过nfs的挂载实现交叉编译处理 (/work/nfs_root为nfs实现的共享目录)

## 虚拟机环境下: 192.168.99.140
1. cd /work/nfs_root/drivers_and_test/first_drv
2. make
3. arm-linux-gcc -o firstdrvtest firstdrvtest.c 
3. .ko是2.6内核使用的动态连接文件的后缀名，也就是模块文件

## 开发板环境下 (192.168.99.135)
1. `挂载` --- `
* mount -t nfs -o nolock,vers=2,rsize=1024,wsize=1024 192.168.99.140:/work/nfs_root /mnt
2. `若出现以下的问题，请添加参数`
* nfs：server is not responding, still trying
* 添加参数后命令为： mount -t nfs -o nolock,vers=2,rsize=1024,wsize=1024 192.168.99.140:/work/nfs_root /mnt
* `rsize=1024,wsize=1024` : `具有较高的传送速率的NFS主机网卡和较低速率的目标机网卡之间不匹配，要解决此问题需要在挂载文件系统时添加额外的参数`
3. `insmod  first_drv.ko`     
```
当前位置：首页 » 硬件·内核·Shell·监测 » insmod insmod命令内核与模块管理 insmod命令用于将给定的模块加载到内核中。
Linux有许多功能是通过模块的方式，在需要时才载入kernel。如此可使kernel较为精简，进而提高效率，以及保有较大的弹性。
这类可载入的模块，通常是设备驱动程序
```
4. `执行程序`
* # ./firstdrvtest on       //打开灯
* # ./firstdrvtest off      //关灯