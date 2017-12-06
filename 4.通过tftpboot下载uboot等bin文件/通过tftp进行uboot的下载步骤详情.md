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