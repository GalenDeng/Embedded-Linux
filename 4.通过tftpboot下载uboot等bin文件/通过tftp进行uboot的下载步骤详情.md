## 通过tftp进行uboot的下载步骤详情 (2017.12.6)
1. `下载tftpboot软件，并安装`
2. 打开`tftpd32软件`，`选择要下载的文件的地址`,选择`本电脑的IP地址`
* ![tftp设置]()
3. NOR flash启动单板,按空格进行u-boot的菜单栏，按 q 退出 menu
   进入到 openjtag 中, 设置 ipaddr serverip ,其中 serverip要和电脑的IP相同，而 ipaddr可以随便设置，但要保证其和serverip在同一个网段下,最后 `save`
* ![]()
* openjtag > print  //print可查看网络的设置情况
4. 在u-boot菜单栏中 ping 一下 电脑的 IP 看是否 ping 通
5. `下载` 
* tftp 30000000 lcd.bin     // 30000000：为烧写地址 lcd.bin 为 二进制程序 这里不用指定目录,因为tftpd32已经指定了目录的地址