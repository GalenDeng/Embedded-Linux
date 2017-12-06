##  u-boot、linux内核打补丁、编译以及sourceinsight的使用说明 (2017.12.06)
## u-boot打补丁
1. 
```
diff -urN u-boot-1.1.6/board/100ask24x0/100ask24x0.c u-boot-1.1.6_jz2440/board/100ask24x0/100ask24x0.c
--- u-boot-1.1.6/board/100ask24x0/100ask24x0.c	1970-01-01 07:00:00.000000000 +0700
+++ u-boot-1.1.6_jz2440/board/100ask24x0/100ask24x0.c	2010-11-26 12:54:37.034090906 +0800
@@ -0,0 +1,96 @@
+/*
+ * (C) Copyright 2002
+ * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
+ * Marius Groeger <mgroeger@sysgo.de>
+ *
```
* `---` 原来的文件
* `+++` 修改后的文件
* `-` ：去掉该内容
* `+` ：增加该内容

2. `diff 和 patch`
```
diff: 为逐行比较两个文本文件，列出其不同之处，可是
    做成diff记录就是补丁 
* -u: 以统一格式创建补丁文件 
* -r: 递归选项,diff会将两个不通版本源代码目录中的所有对应的文件进行一次比较，包括子目录文件
* -N：确保补丁文件将正确地处理已经创建或删除文件的情况
patch : 利用diff制作的补丁来打到文件(夹),使其文件文件夹一致
* -p1 : 省略从开始数来的第一个`/`之前的目录地址
* $ cd u-boot-1.1.6
* $ patch -p1 < ../u-boot-1.1.6_jz2440.patch       // -p1省略了u-boot-1.1.6/
```
3. `备份`
* cd ..
* tar cjf u-boot-1.1.6_jz2440.tar.bz2 u-boot-1.1.6
4. `编译`
* $ make 100ask24x0_config
* $ make
5. 把生成的 u-boot.bin 通过 nor启动,在linux下使用 sudo dnw u-boot.bin 烧写到 nand flash中
6. 打补丁完成

## 内核打补丁即修复
1. tar xjf linux-2.6.22.6.tar.bz2 
2. patch  -p1 < ../linux-2.6.22.6_jz2440.patch
3. cp config_ok  .config
4. cd ..
5. tar cjf linux-2.6.22.6_jz2440.tar.bz2 linux-2.6.22.6
6. make uImage

## sourceinsignt
1. 可以直接拖动文件进 sourceinsignt里面，其自动打开让你查看其源代码
2. 在要查看的源代码文件夹中新建一个文件夹，通过 sourceinsignt 的new project新建文件，synchronize file来产生目录文件
3. ctrl + 选中的字符, 软件会跳到文件的定义调用处