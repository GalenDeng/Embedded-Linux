## busybox -- 命令行的收集 (2017.12.14)
1. galen@HD66:/work/busybox-test$ tar jxf busybox-1.7.0.tar.bz2 
2. cd busybox-1.7.0/ ; ls
3. `一般可以从 INSTALL or README 文件来查看编译的步骤`
4. `编译`
```
  make menuconfig     # This creates a file called ".config"  
  // 这个是图形界面，如果在这个图形界面中search不到cross的字样(交叉编译工具),只能去Makefile中查找
  make                # This creates the "busybox" executable
  make install        # or make CONFIG_PREFIX=/path/from/root install  // /path/from/root 安装目录
```
5. `vim Makefile`
* /^CROSS
* CROSS_COMPILE   ?=            // 这个就是交叉编译工具
* 我们看一下能不能在配置文件里面定义 -- 这里找不到
* 我们只能直接定义 CROSS_COMPILE   ?= arm-linux-  或者 在编译的时候执行 make CROSS_COMPILE=arm-linux-
* ：wq!
6. `make menuconfig 进行性能微调`
* [*] Tab completion    // 命令补全   位置 Busybox Settings ---> Busybox Library Tuning --->
* [] Build BusyBox a static binary (no shared libs)     // 不选这个，不然使用glibc时,编译BusyBox会出错     --- 应使用动态连接的BusyBox     位置： Build Options --->
* 还有其他的配置 --- 详见:《嵌入式Linux应用开发》的 P348 - P349
6. `make`
* 这个过程中可能会出现一些错误的信息，如果这些命令你不用的，可以执行`make menuconfig`，在弹出的
* 菜单栏里面去掉这些配置
7. `因为我们现在使用的是交叉编译工具，如果我们此时执行 make install , busybox将会安装在我们的主机上面，这样是错误的`
* galen@HD66:/work/busybox-test/busybox-1.7.0$ mkdir -p /work/nfs_root/first-fs
* -p : 可以建立嵌套的目录文件啊，如果上一级的目录没有创建，会自动创建出来
* galen@HD66:/work/busybox-test/busybox-1.7.0$ make CONFIG_PREFIX=/work/nfs_root/first-fs install
* galen@HD66:/work/busybox-test/busybox-1.7.0$ cd /work/nfs_root/first-fs/
* galen@HD66:/work/nfs_root/first-fs$ ls -lt
```
total 12
drwxrwxr-x 4 galen galen 4096 Dec 13 15:39 usr
drwxrwxr-x 2 galen galen 4096 Dec 13 15:39 bin
lrwxrwxrwx 1 galen galen   11 Dec 13 15:39 linuxrc -> bin/busybox
drwxrwxr-x 2 galen galen 4096 Dec 13 15:39 sbin
```
```
galen@HD66:/work/nfs_root/first-fs$ ls -l bin/ls
lrwxrwxrwx 1 galen galen 7 Dec 13 15:39 bin/ls -> busybox
galen@HD66:/work/nfs_root/first-fs$ ls -l linuxrc 
lrwxrwxrwx 1 galen galen 11 Dec 13 15:39 linuxrc -> bin/busybox
```
8. `直到这里，我们完成了 init ===> busybox 的功能`

